#pragma once

#include"../function/singleton.h"

/// <summary>
/// 描画基盤クラス
/// </summary>
class Renderer : public Singleton<Renderer>
{
private:
	friend class Singleton<Renderer>;

	Renderer() = default;
	~Renderer() = default;

public:

	/// <summary>
	/// 初期化
	/// </summary>
	bool Init();
private:

	/// <summary>
	/// DXGIの作成
	/// </summary>
	bool CreateDXGI();

	/// <summary>
	/// DebugLayerの有効化
	/// </summary>
#ifdef _DEBUG
	bool EnableDebugLayer();
#endif

	/// <summary>
	/// Deviceの作成
	/// </summary>
	bool CreateDevice();

	/// <summary>
	/// CommandAllocatorの作成
	/// </summary>
	bool CreateCommandAllocator();

	/// <summary>
	/// CommandListの作成
	/// </summary>
	bool CreateCommandList();

	/// <summary>
	/// CommandQueueの作成
	/// </summary>
	bool CreateCommandQueue();

	/// <summary>
	/// SwapChainの作成
	/// </summary>
	bool CreateSwapChain();

	/// <summary>
	/// BackBufferRenderTargetDecriptorHeapの作成
	/// </summary>
	bool CreateBackBufferRenderTargetDecriptorHeap();

	/// <summary>
	/// BackBufferRenderTargetの作成
	/// </summary>
	bool CreateBackBufferRenderTarget();

	/// <summary>
	/// Fenceの作成
	/// </summary>
	bool CreateFence();
private:
	Microsoft::WRL::ComPtr<IDXGIFactory6> m_DxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device = nullptr;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue = nullptr;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_Swapchain = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_BackBufferRenderTargetDecriptorHeap = nullptr;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_BackBufferRenderTargets;

	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence = nullptr;
	UINT64 m_FenceVal = 0;
};