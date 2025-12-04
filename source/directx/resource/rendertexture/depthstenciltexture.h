#pragma once

class DepthStencilTexture
{
public:
	DepthStencilTexture(ID3D12Device* device);
	~DepthStencilTexture();

	bool Init(UINT width, UINT height, DXGI_FORMAT format, ID3D12DescriptorHeap* dsvHeap, UINT dsvDescriptorIndex, UINT dsvDescriptorSize);

	bool CreateShaderResourceView(ID3D12DescriptorHeap* srvHeap, UINT srvDescriptorIndex, UINT srvDescriptorSize);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() const { return m_DSVHandle; }

private:
	ID3D12Device* m_Device = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_DSVHandle = {};
	D3D12_GPU_DESCRIPTOR_HANDLE m_SRVHandle = {};
	DXGI_FORMAT m_Format = DXGI_FORMAT_UNKNOWN;
};