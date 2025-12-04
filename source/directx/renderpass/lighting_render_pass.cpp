#include "lighting_render_pass.h"
#include"../renderer.h"

namespace LightingPipelineLayout
{
    // シェーダーレジスタ
    constexpr UINT WORLDMATRIX_REGISTER = 0;      // b0
    constexpr UINT CAMERA_REGISTER = 1;      // b1
    constexpr UINT TEXTURE_REGISTER = 0;    // t0

    // ルートパラメーターのインデックス
    constexpr UINT ALBEDO_CBV = 0;
    constexpr UINT NORMAL_CBV = 1;

    // シェーダーレジスタ
    constexpr UINT ALBEDO_TEXTURE_REGISTER = 0;      // b0
    constexpr UINT NORMAL_TEXTURE_REGISTER = 1;      // b1
}


LightingRenderPass::LightingRenderPass(const Camera* camera)
	:RenderPass(camera)
{
}

LightingRenderPass::~LightingRenderPass()
{
}

bool LightingRenderPass::Init()
{
	Renderer& renderer = Renderer::GetInstance();

    RenderPipelineDescriptor lighting;
    lighting.vsFilePath = L"source/directx/shader/lgihting_vs.hlsl";
    lighting.psFilePath = L"source/directx/shader/lighting_ps.hlsl";
    // 頂点バッファなしでフルスクリーン三角形を描画するため、inputLayoutは空
    lighting.inputLayout = {};

    // テクスチャ用のSRVパラメーターを追加
    RootParameterDescriptor albedoTextureParam;
    albedoTextureParam.type = RootParameterType::DescriptorTable;
    albedoTextureParam.shaderRegister = LightingPipelineLayout::ALBEDO_CBV;
    albedoTextureParam.registerSpace = 0;
    albedoTextureParam.visibility = D3D12_SHADER_VISIBILITY_PIXEL;
    albedoTextureParam.rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    albedoTextureParam.descriptorRangeCount = 1;
    lighting.rootParameters.push_back(albedoTextureParam);

    // テクスチャ用のSRVパラメーターを追加
    RootParameterDescriptor normalTextureParam;
   normalTextureParam.type = RootParameterType::DescriptorTable;
   normalTextureParam.shaderRegister = LightingPipelineLayout::NORMAL_CBV;
   normalTextureParam.registerSpace = 0;
   normalTextureParam.visibility = D3D12_SHADER_VISIBILITY_PIXEL;
   normalTextureParam.rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
   normalTextureParam.descriptorRangeCount = 1;
    lighting.rootParameters.push_back(normalTextureParam);

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
    lighting.staticSamplers.push_back(samplerDesc);

    lighting.rtvFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };

    // フルスクリーン描画なので深度テストは不要
    lighting.depthEnable = false;

    if (renderer.CreatePipeline("lighting", lighting) == false)
    {
        return false;
    }

    // GBufferRenderPassで作成されたテクスチャを取得
    RenderTextureManager* renderTextureManager = renderer.GetRenderTextureManager();
    m_AlbedoTarget = renderTextureManager->GetRenderTexture("ColorRenderTexture");
    m_NormalTarget = renderTextureManager->GetRenderTexture("NormalRenderTexture");

    if (m_AlbedoTarget == nullptr || m_NormalTarget == nullptr)
    {
        return false;
    }

    // ライティング結果を出力するRenderTextureを作成
    m_RenderTarget = renderTextureManager->CreateRenderTexture("LightingResult", SystemData::k_ScreenWidth, SystemData::k_ScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

    if (m_RenderTarget == nullptr)
    {
        return false;
    }

	return true;
}

void LightingRenderPass::DrawBegin()
{
    Renderer& renderer = Renderer::GetInstance();
    ID3D12GraphicsCommandList* commandList = renderer.GetCommandList();

    // PIXEL_SHADER_RESOURCE → RENDER_TARGETに遷移
    D3D12_RESOURCE_BARRIER barrierToRT = {};
    barrierToRT.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrierToRT.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrierToRT.Transition.pResource = m_RenderTarget->GetResource();
    barrierToRT.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrierToRT.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrierToRT.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier(1, &barrierToRT);

    renderer.SetPipeline("lighting");

    // ResourceManagerからデスクリプタヒープを設定
    ID3D12DescriptorHeap* descriptorHeaps[] = { renderer.GetCBVSRVUAVHeap() };
    commandList->SetDescriptorHeaps(1, descriptorHeaps);

    // RTVハンドルを取得
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RenderTarget->GetCPUDescriptorHandle();

    // RenderTargetを設定
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // RenderTargetをクリア
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // ビューポート設定
    D3D12_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(SystemData::k_ScreenWidth);
    viewport.Height = static_cast<float>(SystemData::k_ScreenHeight);
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    commandList->RSSetViewports(1, &viewport);

    // シザー矩形設定
    D3D12_RECT scissorRect = {};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = SystemData::k_ScreenWidth;
    scissorRect.bottom = SystemData::k_ScreenHeight;
    commandList->RSSetScissorRects(1, &scissorRect);

    // GBufferテクスチャをバインド
    if (m_AlbedoTarget)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = m_AlbedoTarget->GetGPUDescriptorHandle();
        commandList->SetGraphicsRootDescriptorTable(LightingPipelineLayout::ALBEDO_CBV, textureHandle);
    }

    if (m_NormalTarget)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = m_NormalTarget->GetGPUDescriptorHandle();
        commandList->SetGraphicsRootDescriptorTable(LightingPipelineLayout::NORMAL_CBV, textureHandle);
    }

    // フルスクリーン三角形を描画
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);
}

void LightingRenderPass::Execute()
{
}

void LightingRenderPass::DrawEnd()
{
    Renderer& renderer = Renderer::GetInstance();
    ID3D12GraphicsCommandList* commandList = renderer.GetCommandList();

    // RENDER_TARGET → PIXEL_SHADER_RESOURCEに遷移
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_RenderTarget->GetResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    commandList->ResourceBarrier(1, &barrier);
}
