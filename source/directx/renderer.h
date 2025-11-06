#pragma once

#include"../function/singleton.h"
#include"render_pipeline/render_pipeline_manager.h"
#include"resource/resource_manager.h"
#include"resource/texture/texture_manager.h"
#include"resource/texture/texture.h"
#include"camera/camera.h"
#include"camera/camera_data.h"
#include"resource/constant_buffer/constant_buffer_manager.h"
#include"resource/constant_buffer/constant_buffer.h"

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

	/// <summary>
	/// 終了処理
	/// </summary>
	void Uninit();

	/// <summary>
	/// 描画開始処理
	/// </summary>
	void DrawBegin();

	/// <summary>
	/// 描画終了処理
	/// </summary>
	void DrawEnd();

	ID3D12Device* GetDevice() const { return m_Device.Get(); }

	ID3D12GraphicsCommandList* GetCommandList() const { return m_CommandList.Get(); }
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
	/// DepthStencilの作成
	/// </summary>
	bool CreateDepthStencil();

	/// <summary>
	/// Fenceの作成
	/// </summary>
	bool CreateFence();

	/// <summary>
	/// 前フレームの描画処理の完了を待つ
	/// </summary>
	void WaitForPreviousFrameGPU();
private:
	Microsoft::WRL::ComPtr<IDXGIFactory6> m_DxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device = nullptr;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue = nullptr;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_Swapchain = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_BackBufferRenderTargetDecriptorHeap = nullptr;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_BackBufferRenderTargets;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DepthStencilDecriptorHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencil = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_DepthStencilViewHandle = {};

	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence = nullptr;
	UINT64 m_FenceVal = 0;
	HANDLE m_FenceEvent = nullptr;

	std::unique_ptr<ResourceManager> m_ResourceManager;
	std::unique_ptr<RenderPipelineManager> m_RenderPipelineManager;
	std::unique_ptr<TextureManager> m_TextureManager;
	std::unique_ptr<ConstantBufferManager> m_ConstantBufferManager;
	std::unique_ptr<Camera> m_Camera;
	ConstantBuffer* m_CameraConstantBuffer = nullptr;
	Texture* texture;
};