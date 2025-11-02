#pragma once

#include"root_parameter_descriptor.h"

struct RenderPipelineDescriptor
{
    // シェーダー設定（既存）
    const wchar_t* vsFilePath = nullptr;
    const wchar_t* psFilePath = nullptr;
    const char* vsEntryPoint = "main";
    const char* psEntryPoint = "main";
    const char* vsShaderModel = "vs_5_1";
    const char* psShaderModel = "ps_5_1";

    // 入力レイアウト（既存）
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;

    // ★ ルートシグネチャ設定（新規）★
    std::vector<RootParameterDescriptor> rootParameters;
    std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;

    // ラスタライザー設定（既存）
    D3D12_FILL_MODE fillMode = D3D12_FILL_MODE_SOLID;
    D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK;
    bool frontCounterClockwise = false;

    // ブレンド設定（既存）
    bool blendEnable = false;
    D3D12_BLEND srcBlend = D3D12_BLEND_ONE;
    D3D12_BLEND destBlend = D3D12_BLEND_ZERO;
    D3D12_BLEND_OP blendOp = D3D12_BLEND_OP_ADD;
    D3D12_BLEND srcBlendAlpha = D3D12_BLEND_ONE;
    D3D12_BLEND destBlendAlpha = D3D12_BLEND_ZERO;
    D3D12_BLEND_OP blendOpAlpha = D3D12_BLEND_OP_ADD;

    // 深度ステンシル設定（既存）
    bool depthEnable = true;
    bool depthWriteEnable = true;
    D3D12_COMPARISON_FUNC depthFunc = D3D12_COMPARISON_FUNC_LESS;

    // レンダーターゲット設定（既存）
    DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    DXGI_FORMAT dsvFormat = DXGI_FORMAT_D32_FLOAT;

    // プリミティブトポロジー（既存）
    D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};