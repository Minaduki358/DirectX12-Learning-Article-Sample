#include "render_pipeline_manager.h"

void RenderPipelineManager::Init(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    m_Device = device;
    m_CommandList = commandList;
}

bool RenderPipelineManager::CreatePipeline(const std::string& name, const RenderPipelineDescriptor& desc)
{
    if (m_Pipelines.find(name) != m_Pipelines.end())
    {
        return false;
    }

    // パイプライン作成
    auto pipeline = std::make_unique<RenderPipeline>();
    if (!pipeline->Create(m_Device, desc))
    {
        return false;
    }

    // 登録
    m_Pipelines[name] = std::move(pipeline);

    return true;
}

bool RenderPipelineManager::SetPipeline(const std::string& name)
{
    auto it = m_Pipelines.find(name);
    if (it == m_Pipelines.end())
    {
        return false;
    }

    // パイプラインを設定
    RenderPipeline* pipeline = it->second.get();
    pipeline->Set(m_CommandList);

    m_CurrentPipeline = pipeline;

    return true;
}

RenderPipeline* RenderPipelineManager::GetPipeline(const std::string& name)
{
    auto it = m_Pipelines.find(name);
    if (it == m_Pipelines.end())
    {
        return nullptr;
    }

    return it->second.get();
}
