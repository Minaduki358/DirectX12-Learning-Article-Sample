#include"rendertexture.h"

RenderTexture::RenderTexture(ID3D12Device* device)
{
}

RenderTexture::~RenderTexture()
{
}

bool RenderTexture::Init(UINT width, UINT height, DXGI_FORMAT format, ID3D12DescriptorHeap* rtvHeap, UINT rtvDescriptorIndex, UINT rtvDescriptorSize)
{
    m_Format = format;

    // リソース設定
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = format;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    // ヒーププロパティ
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    // クリア値
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;
    clearValue.Color[0] = 0.0f;
    clearValue.Color[1] = 0.0f;
    clearValue.Color[2] = 0.0f;
    clearValue.Color[3] = 1.0f;

    // リソース作成
    HRESULT result = m_Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clearValue,
        IID_PPV_ARGS(m_Resource.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    // RTV作成
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = format;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    m_RTVHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    m_RTVHandle.ptr += static_cast<SIZE_T>(rtvDescriptorIndex) * rtvDescriptorSize;

    m_Device->CreateRenderTargetView(m_Resource.Get(), &rtvDesc, m_RTVHandle);

    return true;
}

bool RenderTexture::CreateShaderResourceView(ID3D12DescriptorHeap* srvHeap, UINT srvDescriptorIndex, UINT srvDescriptorSize)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = m_Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    m_SRVHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
    m_SRVHandle.ptr += static_cast<SIZE_T>(srvDescriptorIndex) * srvDescriptorSize;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += static_cast<SIZE_T>(srvDescriptorIndex) * srvDescriptorSize;

    m_Device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, cpuHandle);

    return true;
}