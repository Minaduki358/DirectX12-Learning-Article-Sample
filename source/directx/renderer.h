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
#include"resource/rendertexture/rendertexture_manager.h"

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

	/// <summary>
	/// パイプラインの作成
	/// </summary>
	bool CreatePipeline(const std::string& name, const RenderPipelineDescriptor& desc);

	/// <summary>
	/// パイプラインのセット
	/// </summary>
	bool SetPipeline(const std::string& name);

	ID3D12Device* GetDevice() const { return m_Device.Get(); }

	ID3D12GraphicsCommandList* GetCommandList() const { return m_CommandList.Get(); }

	TextureManager* GetTextureManager() const { return m_TextureManager.get(); }

	ConstantBufferManager* GetConstantBufferManager() const { return m_ConstantBufferManager.get(); }

	RenderTextureManager* GetRenderTextureManager() const { return m_RenderTextureManager.get(); }

	ID3D12DescriptorHeap* GetCBVSRVUAVHeap() const { return m_ResourceManager->GetCBVSRVUAVHeap(); }

	/// <summary>
	/// 現在のバックバッファのRTVハンドルを取得
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRTVHandle() const;

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
	/// BackBufferRenderTarget,DecriptorHeapの作成
	/// </summary>
	bool CreateBackBufferRenderTargetAndDecriptorHeap();

	/// <summary>
	/// Fenceの作成
	/// </summary>
	bool CreateFence();

private:
	Microsoft::WRL::ComPtr<IDXGIFactory6> m_DxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device = nullptr;

	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_CommandAllocators;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue = nullptr;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_Swapchain = nullptr;
	std::vector<UINT> m_BackBufferRTVIndices;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_BackBufferRenderTargets;

	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence = nullptr;
	UINT64 m_FenceVal = 0;
	HANDLE m_FenceEvent = nullptr;

	static const UINT FRAME_COUNT = 2;
	std::vector<UINT64> m_FenceValues;

	std::unique_ptr<ResourceManager> m_ResourceManager;
	std::unique_ptr<RenderPipelineManager> m_RenderPipelineManager;
	std::unique_ptr<TextureManager> m_TextureManager;
	std::unique_ptr<ConstantBufferManager> m_ConstantBufferManager;
	std::unique_ptr<RenderTextureManager> m_RenderTextureManager;
};