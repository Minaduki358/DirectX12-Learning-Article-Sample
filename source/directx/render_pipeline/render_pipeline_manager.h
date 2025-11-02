#pragma once

#include"render_pipeline_descriptor.h"
#include"render_pipeline.h"

class RenderPipelineManager 
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    bool CreatePipeline(const std::string& name, const RenderPipelineDescriptor& desc);
    bool SetPipeline(const std::string& name);
    RenderPipeline* GetPipeline(const std::string& name);

private:
    ID3D12Device* m_Device;
    ID3D12GraphicsCommandList* m_CommandList;
    std::map<std::string, std::unique_ptr<RenderPipeline>> m_Pipelines;
    RenderPipeline* m_CurrentPipeline;
};