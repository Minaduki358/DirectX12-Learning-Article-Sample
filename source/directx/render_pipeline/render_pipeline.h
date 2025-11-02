#pragma once

#include"render_pipeline_descriptor.h"

class RenderPipeline
{
public:
	RenderPipeline();
	~RenderPipeline();

    bool Create(ID3D12Device* device, const RenderPipelineDescriptor& desc);
    void Set(ID3D12GraphicsCommandList* commandList);

    ID3D12RootSignature* GetRootSignature() const { return m_RootSignature.Get(); }
    ID3D12PipelineState* GetPipelineState() const { return m_PipelineState.Get(); }

private:
    bool CreateRootSignature(ID3D12Device* device, const RenderPipelineDescriptor& desc);

    bool CreatePipelineState(ID3D12Device* device, const RenderPipelineDescriptor& desc);

    bool CreateDefaultRootSignature(ID3D12Device* device);
private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
    D3D_PRIMITIVE_TOPOLOGY m_PrimitiveTopology;
};