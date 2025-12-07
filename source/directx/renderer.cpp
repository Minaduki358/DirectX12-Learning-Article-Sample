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

    if (CreateFence() == false)
    {
        return false;
    }

    // ResourceManagerを先に初期化（デスクリプタヒープを作成）
    m_ResourceManager = std::make_unique<ResourceManager>();
    if (!m_ResourceManager->Init(m_Device.Get(), 1000))
    {
        return false;
    }

    // TextureManagerはResourceManagerを参照
    m_TextureManager = std::make_unique<TextureManager>(
        m_Device.Get(), 
        m_CommandQueue.Get(), 
        m_ResourceManager.get()
    );
    if (m_TextureManager->Init() == false)
    {
        return false;
    }

    // ConstantBufferManagerを作成
    m_ConstantBufferManager = std::make_unique<ConstantBufferManager>(
        m_Device.Get(),
        m_ResourceManager.get()
    );
    if (m_ConstantBufferManager->Init() == false)
    {
        return false;
    }

    // RenderTextureManagerを作成
    m_RenderTextureManager = std::make_unique<RenderTextureManager>(m_Device.Get(), m_ResourceManager.get());
    if (m_RenderTextureManager->Init() == false)
    {
        return false;
    }

    // BackBufferの作成
    if (CreateBackBufferRenderTargetAndDecriptorHeap() == false)
    {
        return false;
    }

    // RenderPipelineManager作成
    m_RenderPipelineManager = std::make_unique<RenderPipelineManager>(m_Device.Get(), m_CommandList.Get());
    if (m_RenderPipelineManager == nullptr)
    {
        return false;
    }

	return true;
}

void Renderer::Uninit()
{
    // GPU処理が完全に終わるまで待機
    if (m_Fence && m_CommandQueue) 
    {
        m_CommandQueue->Signal(m_Fence.Get(), ++m_FenceVal);
        if (m_Fence->GetCompletedValue() < m_FenceVal) 
        {
            if (m_FenceEvent != nullptr) 
            {
                m_Fence->SetEventOnCompletion(m_FenceVal, m_FenceEvent);
                WaitForSingleObject(m_FenceEvent, INFINITE);
            }
        }
    }

    // HANDLEをクローズ
    if (m_FenceEvent != nullptr) 
    {
        CloseHandle(m_FenceEvent);
        m_FenceEvent = nullptr;
    }
}

void Renderer::DrawBegin()
{
    UINT backBufferIndex = m_Swapchain->GetCurrentBackBufferIndex();
    
    // このバックバッファの前回の使用が完了するまで待つ（2フレーム前）
    if (m_Fence->GetCompletedValue() < m_FenceValues[backBufferIndex])
    {
        m_Fence->SetEventOnCompletion(m_FenceValues[backBufferIndex], m_FenceEvent);
        WaitForSingleObject(m_FenceEvent, INFINITE);
    }
    
    // バックバッファに対応するCommandAllocatorをリセット
    m_CommandAllocators[backBufferIndex]->Reset();
    m_CommandList->Reset(m_CommandAllocators[backBufferIndex].Get(), nullptr);

    // Present → RenderTarget
    D3D12_RESOURCE_BARRIER barrierDesc = {};
    barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrierDesc.Transition.pResource = m_BackBufferRenderTargets[backBufferIndex].Get();
    barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_CommandList->ResourceBarrier(1, &barrierDesc);
}

void Renderer::DrawEnd()
{
    UINT backBufferIndex = m_Swapchain->GetCurrentBackBufferIndex();

    // RenderTarget → Present
    D3D12_RESOURCE_BARRIER barrierDesc = {};
    barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrierDesc.Transition.pResource = m_BackBufferRenderTargets[backBufferIndex].Get();
    barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    m_CommandList->ResourceBarrier(1, &barrierDesc);

    m_CommandList->Close();
    ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(1, cmdLists);

    // 現在のバックバッファにFence値を記録
    m_FenceVal++;
    m_FenceValues[backBufferIndex] = m_FenceVal;
    m_CommandQueue->Signal(m_Fence.Get(), m_FenceVal);

    m_Swapchain->Present(1, 0);
}

bool Renderer::CreatePipeline(const std::string& name, const RenderPipelineDescriptor& desc)
{
    return m_RenderPipelineManager->CreatePipeline(name, desc);
}

bool Renderer::SetPipeline(const std::string& name)
{
    return m_RenderPipelineManager->SetPipeline(name);
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::GetCurrentBackBufferRTVHandle() const
{
    UINT backBufferIndex = m_Swapchain->GetCurrentBackBufferIndex();
    return m_ResourceManager->GetRTVHandle(m_BackBufferRTVIndices[backBufferIndex]);
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
    // バックバッファ数分のCommandAllocatorを作成（2つ）
    m_CommandAllocators.resize(2);
    
    for (UINT i = 0; i < 2; i++)
    {
        HRESULT result = m_Device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(m_CommandAllocators[i].ReleaseAndGetAddressOf())
        );

        if (FAILED(result))
        {
            return false;
        }
    }

    return true;
}

bool Renderer::CreateCommandList()
{
    HRESULT result = m_Device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_CommandAllocators[0].Get(),
        nullptr,
        IID_PPV_ARGS(m_CommandList.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    m_CommandList->Close();

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

bool Renderer::CreateBackBufferRenderTargetAndDecriptorHeap()
{
    DXGI_SWAP_CHAIN_DESC1 swcDesc = {};
    HRESULT result = m_Swapchain->GetDesc1(&swcDesc);

    if (FAILED(result))
    {
        return false;
    }

    auto allocation = m_ResourceManager->AllocateRTV(swcDesc.BufferCount);
    if (allocation.success == false)
    {
        return false;
    }

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    for (UINT i = 0; i < swcDesc.BufferCount; i++)
    {
        // SwapChainとRenderTargetを紐づける
        ComPtr<ID3D12Resource> resource;
        result = m_Swapchain->GetBuffer(i, IID_PPV_ARGS(resource.ReleaseAndGetAddressOf()));

        if (FAILED(result))
        {
            return false;
        }

        m_BackBufferRenderTargets.push_back(resource);

        // ResourceManagerからCPUハンドルを取得
        UINT rtvIndex = allocation.index + i;
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_ResourceManager->GetRTVHandle(rtvIndex);

        // インデックスを保存
        m_BackBufferRTVIndices.push_back(rtvIndex);

        // RenderTargetViewを作成
        m_Device->CreateRenderTargetView(
            m_BackBufferRenderTargets[i].Get(),
            &rtvDesc,
            rtvHandle
        );
    }

    return true;
}

bool Renderer::CreateFence()
{
    HRESULT result = m_Device->CreateFence(
        0,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(m_Fence.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    if (m_FenceEvent == nullptr) 
    {
        return false;
    }

    // 各フレームのFence値を初期化
    m_FenceValues.resize(FRAME_COUNT, 0);
    m_FenceVal = 0;

    return true;
}
