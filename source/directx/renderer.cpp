#include"renderer.h"
#include"../system/mywindow.h"
#include "test_mesh.h"

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

    if (CreateDepthStencil() == false)
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
        m_CommandList.Get(), 
        m_CommandQueue.Get(), 
        m_CommandAllocator.Get(),
        m_ResourceManager.get()
    );
    if (m_TextureManager->Init() == false)
    {
        return false;
    }

    texture = m_TextureManager->LoadTexture(L"resources/texture/test.png");

    // ConstantBufferManagerを作成
    m_ConstantBufferManager = std::make_unique<ConstantBufferManager>(
        m_Device.Get(),
        m_ResourceManager.get()
    );
    if (!m_ConstantBufferManager->Init())
    {
        return false;
    }

    // カメラを作成
    m_Camera = std::make_unique<Camera>();
    m_Camera->SetPosition(DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f));
    m_Camera->SetLookAt(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
    m_Camera->SetUp(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    
    // アスペクト比を計算
    float aspectRatio = static_cast<float>(SystemData::k_ScreenWidth) / static_cast<float>(SystemData::k_ScreenHeight);
    m_Camera->SetProjection(DirectX::XM_PI / 4.0f, aspectRatio, 0.1f, 100.0f);
    m_Camera->SetModelMatrix(DirectX::XMMatrixIdentity());

    // カメラデータ用のコンスタントバッファを作成
    m_CameraConstantBuffer = m_ConstantBufferManager->CreateConstantBuffer(sizeof(CameraData));
    if (!m_CameraConstantBuffer)
    {
        return false;
    }

    m_RenderPipelineManager = std::make_unique<RenderPipelineManager>(m_Device.Get(), m_CommandList.Get());

    RenderPipelineDescriptor basicDesc;
    basicDesc.vsFilePath = L"source/directx/shader/basic_vs.hlsl";
    basicDesc.psFilePath = L"source/directx/shader/basic_ps.hlsl";
    basicDesc.inputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
    // カメラデータ用のCBVパラメーターを追加（ルートパラメータ0）
    // DescriptorTableとして定義（SetGraphicsRootDescriptorTableを使用するため）
    RootParameterDescriptor cameraDataParam;
    cameraDataParam.type = RootParameterType::DescriptorTable;
    cameraDataParam.shaderRegister = 0;  // b0
    cameraDataParam.registerSpace = 0;
    cameraDataParam.visibility = D3D12_SHADER_VISIBILITY_ALL;
    cameraDataParam.rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cameraDataParam.descriptorRangeCount = 1;
    basicDesc.rootParameters.push_back(cameraDataParam);

    // テクスチャ用のSRVパラメーターを追加（ルートパラメータ1）
    RootParameterDescriptor textureParam;
    textureParam.type = RootParameterType::DescriptorTable; 
    textureParam.shaderRegister = 0;  // t0
    textureParam.registerSpace = 0;
    textureParam.visibility = D3D12_SHADER_VISIBILITY_PIXEL;
    textureParam.rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    textureParam.descriptorRangeCount = 1;
    basicDesc.rootParameters.push_back(textureParam);

    // 静的サンプラーを追加
    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    basicDesc.staticSamplers.push_back(samplerDesc);

    m_RenderPipelineManager->CreatePipeline("basic", basicDesc);


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
    // 前フレームのGPU完了を待つ
    WaitForPreviousFrameGPU();

    m_CommandAllocator->Reset();
    m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

    UINT backBufferIndex = m_Swapchain->GetCurrentBackBufferIndex();

    // Present → RenderTarget
    D3D12_RESOURCE_BARRIER barrierDesc = {};
    barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrierDesc.Transition.pResource = m_BackBufferRenderTargets[backBufferIndex].Get();
    barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_CommandList->ResourceBarrier(1, &barrierDesc);

    // RenderTargetとDepthStencilを設定
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_BackBufferRenderTargetDecriptorHeap->GetCPUDescriptorHandleForHeapStart();
    UINT rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    rtvHandle.ptr += static_cast<SIZE_T>(backBufferIndex) * rtvDescriptorSize;
    m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &m_DepthStencilViewHandle);
    // クリア
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_CommandList->ClearDepthStencilView(m_DepthStencilViewHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // ビューポートの設定（フレーム開始時に1回）
    D3D12_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(SystemData::k_ScreenWidth);
    viewport.Height = static_cast<float>(SystemData::k_ScreenHeight);
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_CommandList->RSSetViewports(1, &viewport);

    // シザー矩形の設定（フレーム開始時に1回）
    D3D12_RECT scissorRect = {};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = SystemData::k_ScreenWidth;
    scissorRect.bottom = SystemData::k_ScreenHeight;
    m_CommandList->RSSetScissorRects(1, &scissorRect);

    m_RenderPipelineManager->SetPipeline("basic");

    // カメラの更新
    m_Camera->Update();

    // カメラデータを構造体にまとめる
    CameraData cameraData;
    cameraData.ModelMatrix = DirectX::XMMatrixTranspose(m_Camera->GetModelMatrix());
    cameraData.ViewMatrix = DirectX::XMMatrixTranspose(m_Camera->GetViewMatrix());
    cameraData.ProjectionMatrix = DirectX::XMMatrixTranspose(m_Camera->GetProjectionMatrix());

    // コンスタントバッファにデータを書き込み
    m_CameraConstantBuffer->UpdateData(&cameraData, sizeof(CameraData));

    // ResourceManagerからデスクリプタヒープを設定
    ID3D12DescriptorHeap* descriptorHeaps[] = { m_ResourceManager->GetCBVSRVUAVHeap() };
    m_CommandList->SetDescriptorHeaps(1, descriptorHeaps);

    // カメラデータのCBVをルートパラメータ0に設定
    if (m_CameraConstantBuffer)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE cameraHandle = m_CameraConstantBuffer->GetGPUDescriptorHandle();
        m_CommandList->SetGraphicsRootDescriptorTable(0, cameraHandle);
    }

    // テクスチャのSRVをルートパラメータ1に設定
    if (texture)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = texture->GetGPUDescriptorHandle();
        m_CommandList->SetGraphicsRootDescriptorTable(1, textureHandle);
    }
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

    m_FenceVal++;
    m_CommandQueue->Signal(m_Fence.Get(), m_FenceVal);

    m_Swapchain->Present(1, 0);
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

bool Renderer::CreateDepthStencil()
{
    // DepthStencilView用のDescriptorHeapを作成
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NodeMask = 0;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT result = m_Device->CreateDescriptorHeap(
        &dsvHeapDesc,
        IID_PPV_ARGS(m_DepthStencilDecriptorHeap.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    // DepthStencilリソースの作成
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = SystemData::k_ScreenWidth;
    depthDesc.Height = SystemData::k_ScreenHeight;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 0;
    heapProps.VisibleNodeMask = 0;

    result = m_Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(m_DepthStencil.ReleaseAndGetAddressOf())
    );

    if (FAILED(result))
    {
        return false;
    }

    // DepthStencilViewの作成
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    m_DepthStencilViewHandle = m_DepthStencilDecriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_Device->CreateDepthStencilView(
        m_DepthStencil.Get(),
        &dsvDesc,
        m_DepthStencilViewHandle
    );

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

    m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    if (m_FenceEvent == nullptr) 
    {
        return false;
    }

    return true;
}

void Renderer::WaitForPreviousFrameGPU()
{
    const UINT64 fenceValue = m_FenceVal;

    // GPU完了値を確認
    if (m_Fence->GetCompletedValue() < fenceValue)
    {
        // まだ完了していない場合は待機
        m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent);
        WaitForSingleObject(m_FenceEvent, INFINITE);
    }
}
