#include "deferred_render_pass.h"
#include"../renderer.h"

namespace DeferredPipelineLayout
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

DeferredRenderPass::DeferredRenderPass(const Camera* camera)
	: RenderPass(camera)
{

}

DeferredRenderPass::~DeferredRenderPass()
{
}

bool DeferredRenderPass::Init()
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
    worldMatrixParam.shaderRegister = DeferredPipelineLayout::WORLDMATRIX_REGISTER;
    worldMatrixParam.registerSpace = 0;
    worldMatrixParam.visibility = D3D12_SHADER_VISIBILITY_ALL;
    worldMatrixParam.rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    worldMatrixParam.descriptorRangeCount = 1;
    gbuffer.rootParameters.push_back(worldMatrixParam);

    // カメラデータ用のCBVパラメーターを追加
    RootParameterDescriptor cameraDataParam;
    cameraDataParam.type = RootParameterType::DescriptorTable;
    cameraDataParam.shaderRegister = DeferredPipelineLayout::CAMERA_REGISTER;
    cameraDataParam.registerSpace = 0;
    cameraDataParam.visibility = D3D12_SHADER_VISIBILITY_ALL;
    cameraDataParam.rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cameraDataParam.descriptorRangeCount = 1;
    gbuffer.rootParameters.push_back(cameraDataParam);

    // テクスチャ用のSRVパラメーターを追加（ルートパラメータ1）
    RootParameterDescriptor textureParam;
    textureParam.type = RootParameterType::DescriptorTable;
    textureParam.shaderRegister = DeferredPipelineLayout::TEXTURE_REGISTER;
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

	return true;
}

void DeferredRenderPass::DrawBegin()
{
    Renderer& renderer = Renderer::GetInstance();
    ID3D12GraphicsCommandList* commandList = renderer.GetCommandList();

    renderer.SetPipeline("gbuffer");

    const CameraData& cameraData = m_Camera->GetCameraData();
    // コンスタントバッファにデータを書き込み
    m_CameraConstantBuffer->UpdateData(&cameraData, sizeof(CameraData));

    // ResourceManagerからデスクリプタヒープを設定
    ID3D12DescriptorHeap* descriptorHeaps[] = { renderer.GetCBVSRVUAVHeap() };
    commandList->SetDescriptorHeaps(1, descriptorHeaps);

    // カメラデータのCBVをルートパラメータに設定
    if (m_CameraConstantBuffer)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE cameraHandle = m_CameraConstantBuffer->GetGPUDescriptorHandle();
        commandList->SetGraphicsRootDescriptorTable(DeferredPipelineLayout::CAMERA_CBV, cameraHandle);
    }

    // テクスチャのSRVをルートパラメータに設定
    if (m_Texture)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = m_Texture->GetGPUDescriptorHandle();
        commandList->SetGraphicsRootDescriptorTable(DeferredPipelineLayout::TEXTURE_CBV, textureHandle);
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

void DeferredRenderPass::Execute()
{
}

void DeferredRenderPass::DrawEnd()
{
}
