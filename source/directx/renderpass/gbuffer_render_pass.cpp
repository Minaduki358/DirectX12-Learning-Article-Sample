#include"gbuffer_render_pass.h"
#include"../renderer.h"

namespace GBufferPipelineLayout
{
    // ルートパラメーターのインデックス
    constexpr UINT WORLDMATRIX_CBV = 0;
    constexpr UINT CAMERA_CBV = 1;
    constexpr UINT TEXTURE_CBV = 2;

    // シェーダーレジスタ
    constexpr UINT WORLDMATRIX_REGISTER = 0;      // b0
    constexpr UINT CAMERA_REGISTER = 1;      // b1
    constexpr UINT TEXTURE_REGISTER = 0;    // t0
}

GBufferRenderPass::GBufferRenderPass(const Camera* camera)
    : RenderPass(camera)
{

}

GBufferRenderPass::~GBufferRenderPass()
{
}

bool GBufferRenderPass::Init()
{
    Renderer& renderer = Renderer::GetInstance();

    // ワールド行列用のコンスタントバッファを作成
    m_WorldMatrixConstantBuffer = renderer.GetConstantBufferManager()->CreateConstantBuffer<DirectX::XMMATRIX>();
    if (m_WorldMatrixConstantBuffer == nullptr)
    {
        return false;
    }

    // カメラデータ用のコンスタントバッファを作成
    m_CameraConstantBuffer = renderer.GetConstantBufferManager()->CreateConstantBuffer<CameraData>();
    if (m_CameraConstantBuffer == nullptr)
    {
        return false;
    }

    m_Texture = renderer.GetTextureManager()->LoadTexture(L"resources/texture/test.png");
    if (m_Texture == nullptr)
    {
        return false;
    }

    RenderPipelineDescriptor gbuffer;
    gbuffer.vsFilePath = L"source/directx/shader/gbuffer_vs.hlsl";
    gbuffer.psFilePath = L"source/directx/shader/gbuffer_ps.hlsl";
    gbuffer.inputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };

    RootParameterDescriptor worldMatrixParam;
    worldMatrixParam.type = RootParameterType::DescriptorTable;
    worldMatrixParam.shaderRegister = GBufferPipelineLayout::WORLDMATRIX_REGISTER;
    worldMatrixParam.registerSpace = 0;
    worldMatrixParam.visibility = D3D12_SHADER_VISIBILITY_ALL;
    worldMatrixParam.rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    worldMatrixParam.descriptorRangeCount = 1;
    gbuffer.rootParameters.push_back(worldMatrixParam);

    // カメラデータ用のCBVパラメーターを追加
    RootParameterDescriptor cameraDataParam;
    cameraDataParam.type = RootParameterType::DescriptorTable;
    cameraDataParam.shaderRegister = GBufferPipelineLayout::CAMERA_REGISTER;
    cameraDataParam.registerSpace = 0;
    cameraDataParam.visibility = D3D12_SHADER_VISIBILITY_ALL;
    cameraDataParam.rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cameraDataParam.descriptorRangeCount = 1;
    gbuffer.rootParameters.push_back(cameraDataParam);

    // テクスチャ用のSRVパラメーターを追加（ルートパラメータ1）
    RootParameterDescriptor textureParam;
    textureParam.type = RootParameterType::DescriptorTable;
    textureParam.shaderRegister = GBufferPipelineLayout::TEXTURE_REGISTER;
    textureParam.registerSpace = 0;
    textureParam.visibility = D3D12_SHADER_VISIBILITY_PIXEL;
    textureParam.rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    textureParam.descriptorRangeCount = 1;
    gbuffer.rootParameters.push_back(textureParam);

    // 静的サンプラーを追加
    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    gbuffer.staticSamplers.push_back(samplerDesc);

    gbuffer.rtvFormats = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM };

    if (renderer.CreatePipeline("gbuffer", gbuffer) == false)
    {
        return false;
    }

    // MRT用のRTVDescriptorHeapを作成
    RenderTextureManager* renderTextureManager = renderer.GetRenderTextureManager();
    m_ColorTarget = renderTextureManager->CreateRenderTexture("ColorRenderTexture", SystemData::k_ScreenWidth, SystemData::k_ScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_NormalTarget = renderTextureManager->CreateRenderTexture("NormalRenderTexture", SystemData::k_ScreenWidth, SystemData::k_ScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

    if (m_ColorTarget == nullptr || m_NormalTarget == nullptr)
    {
        return false;
    }

    // MRT用のDSVDescriptorHeapを作成
    m_DepthStencilTexture = renderTextureManager->CreateDepthStencilTexture("GBufferDepthStencil", SystemData::k_ScreenWidth, SystemData::k_ScreenHeight, DXGI_FORMAT_D32_FLOAT);

    if (m_DepthStencilTexture == nullptr)
    {
        return false;
    }

    auto test = std::make_unique<TestMesh>(renderer.GetDevice(), renderer.GetCommandList());
    test->Init();
    AddMesh(std::move(test));

    return true;
}

void GBufferRenderPass::DrawBegin()
{
    Renderer& renderer = Renderer::GetInstance();
    ID3D12GraphicsCommandList* commandList = renderer.GetCommandList();

    // GBufferテクスチャをPIXEL_SHADER_RESOURCE → RENDER_TARGETに遷移
    // RenderTextureは初期状態がPIXEL_SHADER_RESOURCEなので、初回フレームでも正常に動作する
    D3D12_RESOURCE_BARRIER barriers[2] = {};
    
    barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barriers[0].Transition.pResource = m_ColorTarget->GetResource();
    barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barriers[1].Transition.pResource = m_NormalTarget->GetResource();
    barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    commandList->ResourceBarrier(2, barriers);

    renderer.SetPipeline("gbuffer");

    const CameraData& cameraData = m_Camera->GetCameraData();
    // コンスタントバッファにデータを書き込み
    m_CameraConstantBuffer->UpdateData(&cameraData, sizeof(CameraData));

    DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
    m_WorldMatrixConstantBuffer->UpdateData(&matrix, sizeof(DirectX::XMMATRIX));
    

    // ResourceManagerからデスクリプタヒープを設定
    ID3D12DescriptorHeap* descriptorHeaps[] = { renderer.GetCBVSRVUAVHeap() };
    commandList->SetDescriptorHeaps(1, descriptorHeaps);

    if (m_WorldMatrixConstantBuffer)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE cameraHandle = m_WorldMatrixConstantBuffer->GetGPUDescriptorHandle();
        commandList->SetGraphicsRootDescriptorTable(GBufferPipelineLayout::WORLDMATRIX_CBV, cameraHandle);
    }

    // カメラデータのCBVをルートパラメータに設定
    if (m_CameraConstantBuffer)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE cameraHandle = m_CameraConstantBuffer->GetGPUDescriptorHandle();
        commandList->SetGraphicsRootDescriptorTable(GBufferPipelineLayout::CAMERA_CBV, cameraHandle);
    }

    // テクスチャのSRVをルートパラメータに設定
    if (m_Texture)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = m_Texture->GetGPUDescriptorHandle();
        commandList->SetGraphicsRootDescriptorTable(GBufferPipelineLayout::TEXTURE_CBV, textureHandle);
    }

    // MRT用のRTVハンドル配列を作成
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2] = {
        m_ColorTarget->GetCPUDescriptorHandle(),
        m_NormalTarget->GetCPUDescriptorHandle()
    };
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_DepthStencilTexture->GetCPUDescriptorHandle();

    // 複数のRenderTargetを設定
    commandList->OMSetRenderTargets(2, rtvHandles, FALSE, &dsvHandle);

    // 各RenderTargetをクリア
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandles[0], clearColor, 0, nullptr);
    commandList->ClearRenderTargetView(rtvHandles[1], clearColor, 0, nullptr);

    // 深度バッファをクリア
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // ビューポートの設定（フレーム開始時に1回）
    D3D12_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(SystemData::k_ScreenWidth);
    viewport.Height = static_cast<float>(SystemData::k_ScreenHeight);
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    commandList->RSSetViewports(1, &viewport);

    // シザー矩形の設定（フレーム開始時に1回）
    D3D12_RECT scissorRect = {};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = SystemData::k_ScreenWidth;
    scissorRect.bottom = SystemData::k_ScreenHeight;
    commandList->RSSetScissorRects(1, &scissorRect);
}

void GBufferRenderPass::Execute()
{
    for (auto& mesh : m_TestMeshes)
    {
        mesh->Draw();
    }
}

void GBufferRenderPass::DrawEnd()
{
    Renderer& renderer = Renderer::GetInstance();
    ID3D12GraphicsCommandList* commandList = renderer.GetCommandList();

    // GBufferテクスチャをRENDER_TARGET → PIXEL_SHADER_RESOURCEに遷移
    D3D12_RESOURCE_BARRIER barriers[2] = {};
    
    barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barriers[0].Transition.pResource = m_ColorTarget->GetResource();
    barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barriers[1].Transition.pResource = m_NormalTarget->GetResource();
    barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    commandList->ResourceBarrier(2, barriers);
}

void GBufferRenderPass::AddMesh(std::unique_ptr<TestMesh> testMesh)
{
    m_TestMeshes.push_back(std::move(testMesh));
}