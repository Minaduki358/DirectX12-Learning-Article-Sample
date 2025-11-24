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


    // ★ RTVヒープ作成 ★
    m_RTVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = m_MaxRTVDescriptors;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  // RTVはShaderVisibleでない
    rtvHeapDesc.NodeMask = 0;

    result = device->CreateDescriptorHeap(
        &rtvHeapDesc,
        IID_PPV_ARGS(m_RTVHeap.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    // ★ DSVヒープ作成 ★
    m_DSVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = m_MaxDSVDescriptors;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;

    result = device->CreateDescriptorHeap(
        &dsvHeapDesc,
        IID_PPV_ARGS(m_DSVHeap.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

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

ResourceManager::AllocationResult ResourceManager::AllocateRTV(UINT count)
{
    AllocationResult result = {};
    result.success = false;
    result.index = UINT_MAX;

    if (m_RTVCurrentIndex + count > m_MaxRTVDescriptors)
    {
        return result;
    }

    result.index = m_RTVCurrentIndex;
    result.success = true;
    m_RTVCurrentIndex += count;

    return result;
}

ResourceManager::AllocationResult ResourceManager::AllocateDSV(UINT count)
{
    AllocationResult result = {};
    result.success = false;
    result.index = UINT_MAX;

    if (m_DSVCurrentIndex + count > m_MaxDSVDescriptors)
    {
        return result;
    }

    result.index = m_DSVCurrentIndex;
    result.success = true;
    m_DSVCurrentIndex += count;

    return result;
}

D3D12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetRTVHandle(UINT index) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(index) * static_cast<SIZE_T>(m_RTVDescriptorSize);
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetDSVHandle(UINT index) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(index) * static_cast<SIZE_T>(m_DSVDescriptorSize);
    return handle;
}

