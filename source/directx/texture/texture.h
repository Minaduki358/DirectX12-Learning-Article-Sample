#pragma once

class Texture
{
public:
	Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* commandAllocator);

	bool Init(const std::wstring& fileName);

	bool CreateShaderResourceView(ID3D12DescriptorHeap* descriptorHeap, UINT descriptorIndex, UINT descriptorSize);

	/// <summary>
	/// GPU Descriptor Handleを取得（シェーダーで使用）
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle() const { return m_GPUDescriptorHandle; }

private:
	bool LoadFile(const std::wstring& fileName);

	bool UploadTextureToGPU();

private:
	ID3D12Device* m_Device = nullptr;
	ID3D12GraphicsCommandList* m_CommandList = nullptr;
	ID3D12CommandQueue* m_CommandQueue = nullptr;
	ID3D12CommandAllocator* m_CommandAllocator = nullptr;

	DirectX::TexMetadata m_TextureMetadata = {};
	DirectX::ScratchImage m_ScrachImage = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> m_TextureBuffer = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescriptorHandle = {}; // シェーダーからアクセスするためのGPUハンドル
};