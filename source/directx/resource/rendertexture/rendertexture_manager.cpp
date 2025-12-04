#include"rendertexture_manager.h"
#include "../resource_manager.h"

RenderTextureManager::RenderTextureManager(ID3D12Device* device, ResourceManager* resourceManager)
	: m_Device(device)
	, m_ResourceManager(resourceManager)
{
}

RenderTextureManager::~RenderTextureManager()
{
}

bool RenderTextureManager::Init()
{
    if (!m_Device || !m_ResourceManager)
    {
        return false;
    }

    return true;
}

RenderTexture* RenderTextureManager::CreateRenderTexture(const std::string& name, UINT width, UINT height, DXGI_FORMAT format, bool createSRV)
{
	if (m_RenderTextures.find(name) != m_RenderTextures.end())
	{
		return m_RenderTextures[name].get();
	}

	auto renderTexture = std::make_unique<RenderTexture>(m_Device);

    auto rtvAlloc = m_ResourceManager->AllocateRTV(1);
    if (rtvAlloc.success == false)
    {
        return nullptr;
    }

    // RenderTargetとして初期化
    if (!renderTexture->Init(
        width,
        height,
        format,
        m_ResourceManager->GetRTVHeap(),
        rtvAlloc.index,
        m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)))
    {
        return nullptr;
    }

    // SRVも作成する場合
    if (createSRV)
    {
        auto srvAlloc = m_ResourceManager->AllocateSRV(1);
        if (srvAlloc.success == false)
        {
            return nullptr;
        }

        if (!renderTexture->CreateShaderResourceView(
            m_ResourceManager->GetCBVSRVUAVHeap(),
            srvAlloc.index,
            m_ResourceManager->GetDescriptorSize()))
        {
            return nullptr;
        }
    }

    RenderTexture* result = renderTexture.get();
    m_RenderTextures[name] = std::move(renderTexture);

    return result;
}

DepthStencilTexture* RenderTextureManager::CreateDepthStencilTexture(const std::string name, UINT width, UINT height, DXGI_FORMAT format, bool createSRV)
{
    if (m_DepthStencilTextures.find(name) != m_DepthStencilTextures.end())
    {
        return m_DepthStencilTextures[name].get();
    }

    auto depthStencilTexture = std::make_unique<DepthStencilTexture>(m_Device);

    auto dsvAlloc = m_ResourceManager->AllocateDSV(1);
    if (dsvAlloc.success == false)
    {
        return nullptr;
    }

    // RenderTargetとして初期化
    if (!depthStencilTexture->Init(
        width,
        height,
        format,
        m_ResourceManager->GetDSVHeap(),
        dsvAlloc.index,
        m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)))
    {
        return nullptr;
    }

    // SRVも作成する場合
    if (createSRV)
    {
        auto srvAlloc = m_ResourceManager->AllocateSRV(1);
        if (!srvAlloc.success)
        {
            return nullptr;
        }

        if (!depthStencilTexture->CreateShaderResourceView(
            m_ResourceManager->GetCBVSRVUAVHeap(),
            srvAlloc.index,
            m_ResourceManager->GetDescriptorSize()))
        {
            return nullptr;
        }
    }

    DepthStencilTexture* result = depthStencilTexture.get();
    m_DepthStencilTextures[name] = std::move(depthStencilTexture);

    return result;
}

RenderTexture* RenderTextureManager::GetRenderTexture(const std::string& name)
{
    auto it = m_RenderTextures.find(name);
    if (it != m_RenderTextures.end())
    {
        return it->second.get();
    }

    return nullptr;
}

DepthStencilTexture* RenderTextureManager::GetDepthStencilTexture(const std::string& name)
{
    auto it = m_DepthStencilTextures.find(name);
    if (it != m_DepthStencilTextures.end())
    {
        return it->second.get();
    }

    return nullptr;
}
