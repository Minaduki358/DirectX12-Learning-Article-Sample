#include"renderer.h"
#include"../system/mywindow.h"

using namespace DirectX;
using namespace Microsoft::WRL;

bool Renderer::Init()
{
    if (CreateDXGI() == false)
    {
        return false;
    }

#ifdef _DEBUG
    if (EnableDebugLayer() == false)
    {
        return false;
    }
#endif

    if (CreateDevice() == false)
    {
        return false;
    }

    if (CreateCommandAllocator() == false)
    {
        return false;
    }

    if (CreateCommandList() == false)
    {
        return false;
    }

    if (CreateCommandQueue() == false)
    {
        return false;
    }

    if (CreateSwapChain() == false)
    {
        return false;
    }

    if (CreateBackBufferRenderTargetDecriptorHeap() == false)
    {
        return false;
    }

    if (CreateBackBufferRenderTarget() == false)
    {
        return false;
    }

    if (CreateFence() == false)
    {
        return false;
    }

	return true;
}

bool Renderer::CreateDXGI()
{
#ifdef _DEBUG
    HRESULT result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(m_DxgiFactory.ReleaseAndGetAddressOf()));
    if (FAILED(result))
    {
        return false;
    }
#else
    HRESULT result = CreateDXGIFactory2(0, IID_PPV_ARGS(m_DxgiFactory.ReleaseAndGetAddressOf()));
    if (FAILED(result))
    {
        return false;
    }
#endif // _DEBUG

    return true;
}

#ifdef _DEBUG
bool Renderer::EnableDebugLayer()
{
    ID3D12Debug* debugLayer = nullptr;
    HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));

    if (FAILED(result))
    {
        return false;
    }

    debugLayer->EnableDebugLayer();
    debugLayer->Release();

    return true;
}
#endif

bool Renderer::CreateDevice()
{
    D3D_FEATURE_LEVEL feature_Levels[] =
    {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };

    ComPtr<IDXGIAdapter1> adapter = nullptr;
    for (UINT i = 0;
            SUCCEEDED(m_DxgiFactory->EnumAdapterByGpuPreference(
            i,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
        i++)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        // ソフトウェアアダプターをスキップ
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        // 各機能レベルでデバイス作成を試行
        for (D3D_FEATURE_LEVEL level : feature_Levels)
        {
            HRESULT result = D3D12CreateDevice(
                adapter.Get(),
                level,
                IID_PPV_ARGS(m_Device.ReleaseAndGetAddressOf())
            );

            if (SUCCEEDED(result))
            {
                return true;
            }
        }
    }

    return true;
}

bool Renderer::CreateCommandAllocator()
{
    HRESULT result = m_Device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(m_CommandAllocator.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    return true;
}

bool Renderer::CreateCommandList()
{
    HRESULT result = m_Device->CreateCommandList(
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

    return true;
}

bool Renderer::CreateCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cmdQueueDesc.NodeMask = 0;

    HRESULT result = m_Device->CreateCommandQueue(
        &cmdQueueDesc,
        IID_PPV_ARGS(m_CommandQueue.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    return true;
}

bool Renderer::CreateSwapChain()
{
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

    swapchainDesc.Width = SystemData::k_ScreenWidth;
    swapchainDesc.Height = SystemData::k_ScreenHeight;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.Stereo = false;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapchainDesc.BufferCount = 2;
    swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Flags = 0;

    MyWindow& myWindow = MyWindow::GetInstance();

    ComPtr<IDXGISwapChain1> swapChain1;
    HRESULT result = m_DxgiFactory->CreateSwapChainForHwnd(
        m_CommandQueue.Get(),
        myWindow.GetHWND(),
        &swapchainDesc,
        nullptr,
        nullptr,
        swapChain1.ReleaseAndGetAddressOf()
    );

    if (FAILED(result))
    {
        return false;
    }

    // SwapChain4に変換
    result = swapChain1.As(&m_Swapchain);
    if (FAILED(result))
    {
        return false;
    }

    // ALT+ENTERを無効化
    result = m_DxgiFactory->MakeWindowAssociation(
        myWindow.GetHWND(),
        DXGI_MWA_NO_ALT_ENTER
    );

    if (FAILED(result))
    {
        return false;
    }

    return true;
}

bool Renderer::CreateBackBufferRenderTargetDecriptorHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NodeMask = 0;
    // 表裏の2つ
    heapDesc.NumDescriptors = 2;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT result = m_Device->CreateDescriptorHeap(
        &heapDesc,
        IID_PPV_ARGS(m_BackBufferRenderTargetDecriptorHeap.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    return true;
}

bool Renderer::CreateBackBufferRenderTarget()
{
    DXGI_SWAP_CHAIN_DESC1 swcDesc = {};
    HRESULT result = m_Swapchain->GetDesc1(&swcDesc);

    if (FAILED(result))
    {
        return false;
    }

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    // ガンマ補正
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_BackBufferRenderTargetDecriptorHeap->GetCPUDescriptorHandleForHeapStart();
    UINT rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (UINT i = 0; i < swcDesc.BufferCount; i++)
    {
        // SwapChainとRenderTargetを紐づける
        ComPtr<ID3D12Resource> resource;
        HRESULT result = m_Swapchain->GetBuffer(i, IID_PPV_ARGS(resource.ReleaseAndGetAddressOf()));

        if (FAILED(result))
        {
            return false;
        }

        m_BackBufferRenderTargets.push_back(resource);

        // RenderTargetの作成
        m_Device->CreateRenderTargetView(
            m_BackBufferRenderTargets[i].Get(),
            &rtvDesc,
            rtvHandle
        );

        // ポインタをずらす
        rtvHandle.ptr += rtvDescriptorSize;
    }

    return true;
}

bool Renderer::CreateFence()
{
    HRESULT result = m_Device->CreateFence(
        m_FenceVal,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    return true;
}
