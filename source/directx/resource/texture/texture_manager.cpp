#include"texture_manager.h"
#include"../resource_manager.h"

TextureManager::TextureManager(
    ID3D12Device* device,
    ID3D12CommandQueue* commandQueue,
    ResourceManager* resourceManager
)
    : m_Device(device)
    , m_CommandQueue(commandQueue)
    , m_ResourceManager(resourceManager)
{
}

bool TextureManager::Init()
{
    if (!m_Device || !m_ResourceManager)
    {
        return false;
    }

    // TextureManager専用のCommandAllocatorを作成
    HRESULT result = m_Device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(m_CommandAllocator.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    // TextureManager専用のCommandListを作成
    result = m_Device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_CommandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(m_CommandList.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    // CommandListは作成時に記録状態なので、一旦閉じる
    m_CommandList->Close();

    return true;
}

ID3D12DescriptorHeap* TextureManager::GetSRVDescriptorHeap() const
{
    if (!m_ResourceManager)
    {
        return nullptr;
    }
    return m_ResourceManager->GetCBVSRVUAVHeap();
}

Texture* TextureManager::LoadTexture(const std::wstring& fileName)
{
    if (!m_ResourceManager)
    {
        return nullptr;
    }

    auto it = m_Textures.find(fileName);
    if (it != m_Textures.end())
    {
        return it->second.get();
    }

    ResourceManager::AllocationResult allocation = m_ResourceManager->AllocateSRV(1);
    if (!allocation.success)
    {
        return nullptr;
    }

    auto texture = std::make_unique<Texture>(
        m_Device,
        m_CommandList.Get(),
        m_CommandQueue,
        m_CommandAllocator.Get()
    );

    if (!texture->Init(fileName))
    {
        return nullptr;
    }

    UINT descriptorSize = m_ResourceManager->GetDescriptorSize();
    ID3D12DescriptorHeap* descriptorHeap = m_ResourceManager->GetCBVSRVUAVHeap();
    if (!texture->CreateShaderResourceView(descriptorHeap, allocation.index, descriptorSize))
    {
        return nullptr;
    }

    Texture* texturePtr = texture.get();
    m_Textures[fileName] = std::move(texture);

    return texturePtr;
}


