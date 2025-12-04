#include"depthstenciltexture.h"

DepthStencilTexture::DepthStencilTexture(ID3D12Device* device)
	:m_Device(device)
{
}

DepthStencilTexture::~DepthStencilTexture()
{
}

bool DepthStencilTexture::Init(UINT width, UINT height, DXGI_FORMAT format, ID3D12DescriptorHeap* dsvHeap, UINT dsvDescriptorIndex, UINT dsvDescriptorSize)
{
	m_Format = format;

    // 深度フォーマットに対応するTypelessフォーマットを取得
    DXGI_FORMAT resourceFormat = DXGI_FORMAT_R32_TYPELESS;  // D32_FLOATの場合
    if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)
    {
        resourceFormat = DXGI_FORMAT_R24G8_TYPELESS;
    }

    // DepthStencilリソースの作成
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = resourceFormat;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 0;
    heapProps.VisibleNodeMask = 0;

    HRESULT result = m_Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(m_Resource.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    // RTV作成
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = format;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

    m_DSVHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
    m_DSVHandle.ptr += static_cast<SIZE_T>(dsvDescriptorIndex) * dsvDescriptorSize;

    m_Device->CreateDepthStencilView(m_Resource.Get(), &dsvDesc, m_DSVHandle);

    return true;
}

bool DepthStencilTexture::CreateShaderResourceView(ID3D12DescriptorHeap* srvHeap, UINT srvDescriptorIndex, UINT srvDescriptorSize)
{
    // 深度フォーマットに対応するSRV用フォーマットを取得
    DXGI_FORMAT srvFormat = DXGI_FORMAT_R32_FLOAT;  // D32_FLOATの場合
    if (m_Format == DXGI_FORMAT_D24_UNORM_S8_UINT)
    {
        srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = srvFormat;
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