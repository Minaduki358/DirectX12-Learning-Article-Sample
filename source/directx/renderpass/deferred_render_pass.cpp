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

    if (renderer.CreatePipeline("gbuffer", gbuffer) == false)
    {
        return false;
    }

    //// ★ MRT用のRTVDescriptorHeapを作成 ★
    //D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    //rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    //rtvHeapDesc.NumDescriptors = 2;  // 2つのレンダーターゲット
    //rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    //rtvHeapDesc.NodeMask = 0;

    //HRESULT result = renderer.GetDevice()->CreateDescriptorHeap(
    //    &rtvHeapDesc,
    //    IID_PPV_ARGS(m_RTVHeap.ReleaseAndGetAddressOf())
    //);

    //if (FAILED(result))
    //{
    //    return false;
    //}

    //UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    //// ★ RenderTextureを2つ作成 ★
    //m_ColorTarget = std::make_unique<RenderTexture>(device);
    //if (!m_ColorTarget->InitAsRenderTarget(
    //    SystemData::k_ScreenWidth,
    //    SystemData::k_ScreenHeight,
    //    DXGI_FORMAT_R8G8B8A8_UNORM,
    //    m_RTVHeap.Get(),
    //    0,  // 最初のディスクリプタ
    //    rtvDescriptorSize))
    //{
    //    return false;
    //}

    //m_NormalTarget = std::make_unique<RenderTexture>(device);
    //if (!m_NormalTarget->InitAsRenderTarget(
    //    SystemData::k_ScreenWidth,
    //    SystemData::k_ScreenHeight,
    //    DXGI_FORMAT_R16G16B16A16_FLOAT,  // 法線は高精度フォーマット
    //    m_RTVHeap.Get(),
    //    1,  // 2番目のディスクリプタ
    //    rtvDescriptorSize))
    //{
    //    return false;
    //}


	return true;
}

void DeferredRenderPass::DrawBegin()
{

}

void DeferredRenderPass::DrawEnd()
{
}
