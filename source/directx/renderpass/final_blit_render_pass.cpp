#include "final_blit_render_pass.h"
#include"../renderer.h"

namespace FinalBlitPipelineLayout
{
    // ルートパラメーターのインデックス
    constexpr UINT TEXTURE_CBV = 0;

    // シェーダーレジスタ
    constexpr UINT TEXTURE_REGISTER = 0;    // t0
}

FinalBlitRenderPass::FinalBlitRenderPass(const Camera* camera)
	:RenderPass(camera)
{
}

FinalBlitRenderPass::~FinalBlitRenderPass()
{
}

bool FinalBlitRenderPass::Init()
{
    Renderer& renderer = Renderer::GetInstance();

    m_SourceTexture = renderer.GetRenderTextureManager()->GetRenderTexture("LightingResult");
    if (m_SourceTexture == nullptr)
    {
        return false;
    }

    RenderPipelineDescriptor finalBlit;
    finalBlit.vsFilePath = L"source/directx/shader/final_blit_vs.hlsl";
    finalBlit.psFilePath = L"source/directx/shader/final_blit_ps.hlsl";
    finalBlit.inputLayout = {};

    // テクスチャ用のSRVパラメーターを追加（ルートパラメータ1）
    RootParameterDescriptor textureParam;
    textureParam.type = RootParameterType::DescriptorTable;
    textureParam.shaderRegister = FinalBlitPipelineLayout::TEXTURE_REGISTER;
    textureParam.registerSpace = 0;
    textureParam.visibility = D3D12_SHADER_VISIBILITY_PIXEL;
    textureParam.rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    textureParam.descriptorRangeCount = 1;
    finalBlit.rootParameters.push_back(textureParam);

    // 静的サンプラーを追加
    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    finalBlit.staticSamplers.push_back(samplerDesc);

    finalBlit.rtvFormats = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };

    finalBlit.depthEnable = false;

    if (renderer.CreatePipeline("finalblit", finalBlit) == false)
    {
        return false;
    }

	return true;
}

void FinalBlitRenderPass::DrawBegin()
{
    Renderer& renderer = Renderer::GetInstance();
    ID3D12GraphicsCommandList* commandList = renderer.GetCommandList();

    // パイプライン設定
    renderer.SetPipeline("finalblit");

    // デスクリプタヒープ設定
    ID3D12DescriptorHeap* descriptorHeaps[] = { renderer.GetCBVSRVUAVHeap() };
    commandList->SetDescriptorHeaps(1, descriptorHeaps);

    // バックバッファをRenderTargetとして設定
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = renderer.GetCurrentBackBufferRTVHandle();
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // バックバッファをクリア
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

    // ソーステクスチャをSRVとしてバインド
    if (m_SourceTexture)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = m_SourceTexture->GetGPUDescriptorHandle();
        commandList->SetGraphicsRootDescriptorTable(FinalBlitPipelineLayout::TEXTURE_REGISTER, textureHandle);
    }

    // フルスクリーン三角形を描画
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);
}

void FinalBlitRenderPass::Execute()
{
}

void FinalBlitRenderPass::DrawEnd()
{
}
