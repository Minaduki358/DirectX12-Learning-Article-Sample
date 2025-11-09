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

    // 1. 既に読み込まれているかチェック（キャッシュ確認）
    auto it = m_Textures.find(fileName);
    if (it != m_Textures.end())
    {
        return it->second.get(); // 既に読み込まれているテクスチャを返す
    }

    // 2. ResourceManagerからSRVデスクリプタを割り当て
    ResourceManager::AllocationResult allocation = m_ResourceManager->AllocateSRV(1);
    if (!allocation.success)
    {
        // エラー: デスクリプタヒープが満杯
        return nullptr;
    }

    // 3. Textureインスタンスを作成
    auto texture = std::make_unique<Texture>(
        m_Device,
        m_CommandList.Get(),
        m_CommandQueue,
        m_CommandAllocator.Get()
    );

    // 4. テクスチャファイルを読み込んで初期化
    if (!texture->Init(fileName))
    {
        return nullptr; // 読み込み失敗
    }

    // 5. SRV（ShaderResourceView）を作成
    // ResourceManagerから割り当てられたインデックスを使用
    UINT descriptorSize = m_ResourceManager->GetDescriptorSize();
    ID3D12DescriptorHeap* descriptorHeap = m_ResourceManager->GetCBVSRVUAVHeap();
    if (!texture->CreateShaderResourceView(descriptorHeap, allocation.index, descriptorSize))
    {
        return nullptr; // SRV作成失敗
    }

    // 6. テクスチャをマップに追加（所有権を移動）
    Texture* texturePtr = texture.get();
    m_Textures[fileName] = std::move(texture);

    return texturePtr; // 非所有のポインタを返す
}


