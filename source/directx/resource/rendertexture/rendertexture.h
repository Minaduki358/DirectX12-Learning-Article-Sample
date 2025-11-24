#pragma once

class RenderTexture
{
public:
	RenderTexture(ID3D12Device* device);
	~RenderTexture();

	bool Init(UINT width, UINT height, DXGI_FORMAT format, ID3D12DescriptorHeap* rtvHeap, UINT rtvDescriptorIndex, UINT rtvDescriptorSize);

	bool CreateShaderResourceView(ID3D12DescriptorHeap* srvHeap, UINT srvDescriptorIndex, UINT srvDescriptorSize);

private:
	ID3D12Device* m_Device = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_RTVHandle = {};
	D3D12_GPU_DESCRIPTOR_HANDLE m_SRVHandle = {};
	DXGI_FORMAT m_Format = DXGI_FORMAT_UNKNOWN;
};