#include "resource_manager.h"

using namespace Microsoft::WRL;

ResourceManager::ResourceManager()
    : m_Device(nullptr)
    , m_DescriptorSize(0)
    , m_CurrentIndex(0)
    , m_MaxDescriptors(1000)
{
}

ResourceManager::~ResourceManager()
{
}

bool ResourceManager::Init(ID3D12Device* device, UINT maxDescriptors)
{
    if (!device)
    {
        return false;
    }

    m_Device = device;
    m_MaxDescriptors = maxDescriptors;

    // デスクリプタサイズを取得
    m_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );

    // CBV_SRV_UAV用デスクリプタヒープを作成
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = m_MaxDescriptors;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;

    HRESULT result = m_Device->CreateDescriptorHeap(
        &heapDesc,
        IID_PPV_ARGS(m_CBVSRVUAVHeap.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    // カウンターを初期化
    m_SRVCounter = {};
    m_CBVCounter = {};
    m_UAVCounter = {};

    return true;
}

ResourceManager::AllocationResult ResourceManager::AllocateDescriptor(DescriptorType type, UINT count)
{
    AllocationResult result = {};
    result.success = false;
    result.index = UINT_MAX;

    // ヒープが満杯かチェック
    if (m_CurrentIndex + count > m_MaxDescriptors)
    {
        // エラー: デスクリプタヒープが満杯
        return result;
    }

    // スタックアロケーター方式で割り当て
    result.index = m_CurrentIndex;
    result.success = true;

    // タイプ別カウンターを更新
    switch (type)
    {
    case DescriptorType::SRV:
        m_SRVCounter.current += count;
        break;
    case DescriptorType::CBV:
        m_CBVCounter.current += count;
        break;
    case DescriptorType::UAV:
        m_UAVCounter.current += count;
        break;
    }

    // スタックポインタを進める
    m_CurrentIndex += count;

    return result;
}

D3D12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetCPUDescriptorHandle(UINT index) const
{
    if (!m_CBVSRVUAVHeap || index >= m_MaxDescriptors)
    {
        return {};
    }

    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_CBVSRVUAVHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(index) * static_cast<SIZE_T>(m_DescriptorSize);
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE ResourceManager::GetGPUDescriptorHandle(UINT index) const
{
    if (!m_CBVSRVUAVHeap || index >= m_MaxDescriptors)
    {
        return {};
    }

    D3D12_GPU_DESCRIPTOR_HANDLE handle = m_CBVSRVUAVHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(index) * static_cast<SIZE_T>(m_DescriptorSize);
    return handle;
}

UINT ResourceManager::GetUsedCount(DescriptorType type) const
{
    switch (type)
    {
    case DescriptorType::SRV:
        return m_SRVCounter.current;
    case DescriptorType::CBV:
        return m_CBVCounter.current;
    case DescriptorType::UAV:
        return m_UAVCounter.current;
    default:
        return 0;
    }
}

