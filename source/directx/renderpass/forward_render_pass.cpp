#include"forward_render_pass.h"
#include"../renderer.h"

namespace ForwardPipelineLayout
{
    // ルートパラメーターのインデックス
    constexpr UINT CAMERA_CBV = 0;
    constexpr UINT TEXTURE_CBV = 1;

    // シェーダーレジスタ
    constexpr UINT CAMERA_REGISTER = 0;      // b0
    constexpr UINT TEXTURE_REGISTER = 0;    // t0
}

ForwardRenderPass::ForwardRenderPass(const Camera* camera)
    : RenderPass(camera)
{

}

ForwardRenderPass::~ForwardRenderPass()
{
}

bool ForwardRenderPass::Init()
{
    // ビューポート設定
    m_ViewPort.Width = static_cast<float>(SystemData::k_ScreenWidth);
    m_ViewPort.Height = static_cast<float>(SystemData::k_ScreenHeight);
    m_ViewPort.TopLeftX = 0.0f;
    m_ViewPort.TopLeftY = 0.0f;
    m_ViewPort.MinDepth = 0.0f;

    // シザー矩形設定
    m_ScissorRec.left = 0;
    m_ScissorRec.top = 0;
    m_ScissorRec.right = SystemData::k_ScreenWidth;
    m_ScissorRec.bottom = SystemData::k_ScreenHeight;

	Renderer& renderer = Renderer::GetInstance();

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

    RenderPipelineDescriptor basicDesc;
    basicDesc.vsFilePath = L"source/directx/shader/basic_vs.hlsl";
    basicDesc.psFilePath = L"source/directx/shader/basic_ps.hlsl";
    basicDesc.inputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
    // カメラデータ用のCBVパラメーターを追加（ルートパラメータ0）
    // DescriptorTableとして定義（SetGraphicsRootDescriptorTableを使用するため）
    RootParameterDescriptor cameraDataParam;
    cameraDataParam.type = RootParameterType::DescriptorTable;
    cameraDataParam.shaderRegister = ForwardPipelineLayout::CAMERA_REGISTER;
    cameraDataParam.registerSpace = 0;
    cameraDataParam.visibility = D3D12_SHADER_VISIBILITY_ALL;
    cameraDataParam.rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cameraDataParam.descriptorRangeCount = 1;
    basicDesc.rootParameters.push_back(cameraDataParam);

    // テクスチャ用のSRVパラメーターを追加（ルートパラメータ1）
    RootParameterDescriptor textureParam;
    textureParam.type = RootParameterType::DescriptorTable;
    textureParam.shaderRegister = ForwardPipelineLayout::TEXTURE_REGISTER;
    textureParam.registerSpace = 0;
    textureParam.visibility = D3D12_SHADER_VISIBILITY_PIXEL;
    textureParam.rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    textureParam.descriptorRangeCount = 1;
    basicDesc.rootParameters.push_back(textureParam);

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
    basicDesc.staticSamplers.push_back(samplerDesc);

    if (renderer.CreatePipeline("forward", basicDesc) == false)
    {
        return false;
    }

	return true;
}

void ForwardRenderPass::Uninit()
{
}

void ForwardRenderPass::DrawBegin()
{
    Renderer& renderer = Renderer::GetInstance();

    renderer.SetPipeline("forward");

    const CameraData& cameraData = m_Camera->GetCameraData();
    // コンスタントバッファにデータを書き込み
    m_CameraConstantBuffer->UpdateData(&cameraData, sizeof(CameraData));

    // ResourceManagerからデスクリプタヒープを設定
    ID3D12DescriptorHeap* descriptorHeaps[] = { renderer.GetCBVSRVUAVHeap()};
    renderer.GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);

    // カメラデータのCBVをルートパラメータ0に設定
    if (m_CameraConstantBuffer)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE cameraHandle = m_CameraConstantBuffer->GetGPUDescriptorHandle();
        renderer.GetCommandList()->SetGraphicsRootDescriptorTable(0, cameraHandle);
    }

    // テクスチャのSRVをルートパラメータ1に設定
    if (m_Texture)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = m_Texture->GetGPUDescriptorHandle();
        renderer.GetCommandList()->SetGraphicsRootDescriptorTable(1, textureHandle);
    }
}

void ForwardRenderPass::Draw()
{
}

void ForwardRenderPass::DrawEnd()
{
}
