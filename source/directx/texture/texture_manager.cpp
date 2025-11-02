#include"texture_manager.h"

namespace TextureManagerData
{
    /// <summary>
    /// 最大のデスクリプター数
    /// </summary>
    UINT k_MaxDescriptorNum = 1000;
}

TextureManager::TextureManager(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    ID3D12CommandQueue* commandQueue,
    ID3D12CommandAllocator* commandAllocator
)
    : m_Device(device)
    , m_CommandList(commandList)
    , m_CommandQueue(commandQueue)
    , m_CommandAllocator(commandAllocator)
{
}

bool TextureManager::Init()
{
    if (!m_Device)
    {
        return false;
    }

    // Descriptorサイズを取得
    m_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );

    // SRV用DescriptorHeapを作成
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = TextureManagerData::k_MaxDescriptorNum;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // シェーダーからアクセス可能
    heapDesc.NodeMask = 0;

    HRESULT result = m_Device->CreateDescriptorHeap(
        &heapDesc,
        IID_PPV_ARGS(m_SRVDescriptorHeap.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    return true;
}

Texture* TextureManager::LoadTexture(const std::wstring& fileName)
{
    // 1. 既に読み込まれているかチェック（キャッシュ確認）
    auto it = m_Textures.find(fileName);
    if (it != m_Textures.end())
    {
        return it->second.get(); // 既に読み込まれているテクスチャを返す
    }

    // 2. 上限チェック
    if (m_CurrentDescriptorIndex >= TextureManagerData::k_MaxDescriptorNum)
    {
        // エラー: DescriptorHeapが満杯
        return nullptr;
    }

    // 3. Textureインスタンスを作成
    auto texture = std::make_unique<Texture>(
        m_Device,
        m_CommandList,
        m_CommandQueue,
        m_CommandAllocator
    );

    // 4. テクスチャファイルを読み込んで初期化
    if (!texture->Init(fileName))
    {
        return nullptr; // 読み込み失敗
    }

    // 5. SRV（ShaderResourceView）を作成
    // 現在のDescriptorIndexを使用
    if (!texture->CreateShaderResourceView(m_SRVDescriptorHeap.Get(), m_CurrentDescriptorIndex, m_DescriptorSize))
    {
        return nullptr; // SRV作成失敗
    }

    // 6. テクスチャをマップに追加（所有権を移動）
    Texture* texturePtr = texture.get();
    m_Textures[fileName] = std::move(texture);

    // 7. 次のDescriptorIndexに進む
    m_CurrentDescriptorIndex++;

    return texturePtr; // 非所有のポインタを返す
}


