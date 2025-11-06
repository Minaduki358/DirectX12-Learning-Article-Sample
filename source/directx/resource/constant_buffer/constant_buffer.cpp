#include "constant_buffer.h"
#include "../resource_manager.h"

using namespace Microsoft::WRL;

ConstantBuffer::ConstantBuffer()
{
}

ConstantBuffer::~ConstantBuffer()
{
    if (m_ConstantBuffer && m_MappedData)
    {
        m_ConstantBuffer->Unmap(0, nullptr);
        m_MappedData = nullptr;
    }
}

bool ConstantBuffer::Init(ID3D12Device* device, ResourceManager* resourceManager, UINT size)
{
    if (!device || !resourceManager)
    {
        return false;
    }

    m_Device = device;
    m_ResourceManager = resourceManager;

    // DirectX12ではコンスタントバッファは256バイトアライメントが必要
    // サイズを256バイトにアライメント
    m_Size = (size + 255) & ~255;

    // ResourceManagerからCBVデスクリプタを割り当て
    ResourceManager::AllocationResult allocation = m_ResourceManager->AllocateCBV(1);
    if (!allocation.success)
    {
        return false;
    }

    // コンスタントバッファリソースを作成（UPLOADヒープ）
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 0;
    heapProps.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = m_Size;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT result = m_Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_ConstantBuffer.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    // リソースをマップ（CPUから直接書き込み可能にする）
    D3D12_RANGE readRange = { 0, 0 }; // 読み取り範囲なし
    result = m_ConstantBuffer->Map(0, &readRange, &m_MappedData);
    if (FAILED(result))
    {
        return false;
    }

    // CBV（Constant Buffer View）を作成
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = m_ConstantBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = m_Size;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_ResourceManager->GetCPUDescriptorHandle(allocation.index);
    m_Device->CreateConstantBufferView(&cbvDesc, cpuHandle);

    // GPUデスクリプタハンドルを保存
    m_GPUDescriptorHandle = m_ResourceManager->GetGPUDescriptorHandle(allocation.index);

    return true;
}

void ConstantBuffer::UpdateData(const void* data, UINT size)
{
    if (!m_MappedData || !data)
    {
        return;
    }

    // マップされたメモリに直接コピー（GPUに自動的に転送される）
    UINT copySize = (size < m_Size) ? size : m_Size;
    memcpy(m_MappedData, data, copySize);
}

