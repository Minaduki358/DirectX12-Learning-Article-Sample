#include "render_pipeline.h"
#include"../shader/shader.h"

using namespace Microsoft::WRL;

RenderPipeline::RenderPipeline()
{
}

RenderPipeline::~RenderPipeline()
{
}

bool RenderPipeline::Create(ID3D12Device* device, const RenderPipelineDescriptor& desc)
{
    // ルートシグネチャ作成
    if (!CreateRootSignature(device, desc))
    {
        return false;
    }

    // パイプラインステート作成
    if (!CreatePipelineState(device, desc))
    {
        return false;
    }

    // トポロジーを保存
    m_PrimitiveTopology = desc.primitiveTopology;

    return true;
}

void RenderPipeline::Set(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetGraphicsRootSignature(m_RootSignature.Get());
    commandList->SetPipelineState(m_PipelineState.Get());
    commandList->IASetPrimitiveTopology(m_PrimitiveTopology);
}

bool RenderPipeline::CreateRootSignature(ID3D12Device* device, const RenderPipelineDescriptor& desc)
{
    // ルートパラメータが指定されていない場合はデフォルト
    if (desc.rootParameters.empty())
    {
        return CreateDefaultRootSignature(device);
    }

    // ルートパラメータを構築
    std::vector<D3D12_ROOT_PARAMETER> rootParams;
    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> descriptorRanges;

    for (size_t i = 0; i < desc.rootParameters.size(); i++)
    {
        const RootParameterDescriptor& paramDesc = desc.rootParameters[i];
        D3D12_ROOT_PARAMETER rootParam = {};

        switch (paramDesc.type)
        {
        case RootParameterType::CBV:
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            rootParam.Descriptor.ShaderRegister = paramDesc.shaderRegister;
            rootParam.Descriptor.RegisterSpace = paramDesc.registerSpace;
            rootParam.ShaderVisibility = paramDesc.visibility;
            break;

        case RootParameterType::SRV:
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            rootParam.Descriptor.ShaderRegister = paramDesc.shaderRegister;
            rootParam.Descriptor.RegisterSpace = paramDesc.registerSpace;
            rootParam.ShaderVisibility = paramDesc.visibility;
            break;

        case RootParameterType::UAV:
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            rootParam.Descriptor.ShaderRegister = paramDesc.shaderRegister;
            rootParam.Descriptor.RegisterSpace = paramDesc.registerSpace;
            rootParam.ShaderVisibility = paramDesc.visibility;
            break;

        case RootParameterType::DescriptorTable:
            // 記述子テーブル
            D3D12_DESCRIPTOR_RANGE range = {};
            range.RangeType = paramDesc.rangeType;
            range.NumDescriptors = paramDesc.descriptorRangeCount;
            range.BaseShaderRegister = paramDesc.shaderRegister;
            range.RegisterSpace = paramDesc.registerSpace;
            range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

            descriptorRanges.push_back({range});

            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParam.DescriptorTable.NumDescriptorRanges = 1;
            rootParam.DescriptorTable.pDescriptorRanges = descriptorRanges.back().data();
            rootParam.ShaderVisibility = paramDesc.visibility;
            break;

        }

        rootParams.push_back(rootParam);
    }

    // 静的サンプラー（D3D12_STATIC_SAMPLER_DESCを直接使用）
    const std::vector<D3D12_STATIC_SAMPLER_DESC>& samplers = desc.staticSamplers;

    // ルートシグネチャ作成
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = static_cast<UINT>(rootParams.size());
    rootSignatureDesc.pParameters = rootParams.data();
    rootSignatureDesc.NumStaticSamplers = static_cast<UINT>(samplers.size());
    rootSignatureDesc.pStaticSamplers = samplers.empty() ? nullptr : samplers.data();
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    HRESULT result = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        signature.GetAddressOf(),
        error.GetAddressOf()
    );

    if (FAILED(result))
    {
        if (error)
        {
            OutputDebugStringA((char*)error->GetBufferPointer());
        }
        return false;
    }

    result = device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(m_RootSignature.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    return true;
}

bool RenderPipeline::CreatePipelineState(ID3D12Device* device, const RenderPipelineDescriptor& desc)
{
    // シェーダー
    Shader vertexShader;
    if (!vertexShader.Load(desc.vsFilePath, desc.vsEntryPoint, desc.vsShaderModel))
    {
        return false;
    }

    Shader pixelShader;
    if (!pixelShader.Load(desc.psFilePath, desc.psEntryPoint, desc.psShaderModel))
    {
        return false;
    }

    // パイプラインステート設定
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    // ルートシグネチャ
    psoDesc.pRootSignature = m_RootSignature.Get();

    // シェーダー
    psoDesc.VS.pShaderBytecode = vertexShader.GetBlob()->GetBufferPointer();
    psoDesc.VS.BytecodeLength = vertexShader.GetBlob()->GetBufferSize();
    psoDesc.PS.pShaderBytecode = pixelShader.GetBlob()->GetBufferPointer();
    psoDesc.PS.BytecodeLength = pixelShader.GetBlob()->GetBufferSize();

    // 入力レイアウト
    psoDesc.InputLayout.pInputElementDescs = desc.inputLayout.data();
    psoDesc.InputLayout.NumElements = static_cast<UINT>(desc.inputLayout.size());

    // ラスタライザー
    psoDesc.RasterizerState.FillMode = desc.fillMode;
    psoDesc.RasterizerState.CullMode = desc.cullMode;
    psoDesc.RasterizerState.FrontCounterClockwise = desc.frontCounterClockwise;
    psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.RasterizerState.MultisampleEnable = FALSE;
    psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
    psoDesc.RasterizerState.ForcedSampleCount = 0;
    psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // ブレンド
    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;
    psoDesc.BlendState.RenderTarget[0].BlendEnable = desc.blendEnable;
    psoDesc.BlendState.RenderTarget[0].SrcBlend = desc.srcBlend;
    psoDesc.BlendState.RenderTarget[0].DestBlend = desc.destBlend;
    psoDesc.BlendState.RenderTarget[0].BlendOp = desc.blendOp;
    psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = desc.srcBlendAlpha;
    psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = desc.destBlendAlpha;
    psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = desc.blendOpAlpha;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // 深度ステンシル
    psoDesc.DepthStencilState.DepthEnable = desc.depthEnable;
    psoDesc.DepthStencilState.DepthWriteMask = desc.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    psoDesc.DepthStencilState.DepthFunc = desc.depthFunc;
    psoDesc.DepthStencilState.StencilEnable = FALSE;

    // レンダーターゲット
    psoDesc.NumRenderTargets = static_cast<UINT>(desc.rtvFormats.size());
    for (UINT i = 0; i < psoDesc.NumRenderTargets && i < 8; i++)
    {
        psoDesc.RTVFormats[i] = desc.rtvFormats[i];
    }

    // depthEnableがfalseの場合、DSVがnullでも動作するようにDSVFormatはUNKNOWNにする
    psoDesc.DSVFormat = desc.depthEnable ? desc.dsvFormat : DXGI_FORMAT_UNKNOWN;

    // サンプル設定
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.SampleMask = UINT_MAX;

    // プリミティブトポロジー
    psoDesc.PrimitiveTopologyType = desc.primitiveTopologyType;

    // パイプラインステート作成
    HRESULT result = device->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS(m_PipelineState.ReleaseAndGetAddressOf())
    );

    return SUCCEEDED(result);
}

bool RenderPipeline::CreateDefaultRootSignature(ID3D12Device* device)
{
    D3D12_ROOT_PARAMETER rootParameter = {};
    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter.Descriptor.ShaderRegister = 0;
    rootParameter.Descriptor.RegisterSpace = 0;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = &rootParameter;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    HRESULT result = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        signature.GetAddressOf(),
        error.GetAddressOf()
    );

    if (FAILED(result))
    {
        if (error)
        {
            OutputDebugStringA((char*)error->GetBufferPointer());
        }
        return false;
    }

    result = device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(m_RootSignature.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    return true;
}
