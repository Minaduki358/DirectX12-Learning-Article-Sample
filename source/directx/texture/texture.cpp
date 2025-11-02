#include"texture.h"
#include"texture_helper.h"

using namespace DirectX;
using namespace Microsoft::WRL;

Texture::Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* commandAllocator)
{
	m_Device = device;
	m_CommandList = commandList;
	m_CommandQueue = commandQueue;
	m_CommandAllocator = commandAllocator;
}

bool Texture::Init(const std::wstring& fileName)
{
	if (!LoadFile(fileName))
	{
		return false;
	}

	if (!UploadTextureToGPU())
	{
		return false;
	}

	return true;
}

bool Texture::CreateShaderResourceView(ID3D12DescriptorHeap* descriptorHeap, UINT descriptorIndex, UINT descriptorSize)
{
	if (!descriptorHeap || !m_Device || !m_TextureBuffer)
	{
		return false;
	}

	// CPU Descriptor Handleを取得
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cpuHandle.ptr += static_cast<SIZE_T>(descriptorIndex) * static_cast<SIZE_T>(descriptorSize);

	// GPU Descriptor Handleを取得（シェーダーからアクセスするため）
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	gpuHandle.ptr += static_cast<SIZE_T>(descriptorIndex) * static_cast<SIZE_T>(descriptorSize);
	m_GPUDescriptorHandle = gpuHandle; // Textureクラスに保存

	// SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = m_TextureMetadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(m_TextureMetadata.mipLevels);
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	m_Device->CreateShaderResourceView(
		m_TextureBuffer.Get(),
		&srvDesc,
		cpuHandle
	);

	return true;
}

bool Texture::LoadFile(const std::wstring& fileName)
{
	// WICテクスチャのロード
	HRESULT result = LoadFromWICFile(
		fileName.c_str(),
		WIC_FLAGS_NONE,
		&m_TextureMetadata,
		m_ScrachImage
	);

	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool Texture::UploadTextureToGPU()
{
	// 生データ抽出
	const Image* img = m_ScrachImage.GetImage(0, 0, 0);

	// 中間バッファーとしてのアップロードヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProp = {};
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	uploadHeapProp.CreationNodeMask = 0;
	uploadHeapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Alignment = 0;
	resDesc.Width = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * img->height;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;

	ComPtr <ID3D12Resource> uploadBuff = nullptr;
	// 中間バッファー作成
	HRESULT result = m_Device->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,// 特になし
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuff.ReleaseAndGetAddressOf())
	);

	// テクスチャのためのヒープ設定
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;// テクスチャ用
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texHeapProp.CreationNodeMask = 0;
	texHeapProp.VisibleNodeMask = 0;

	// リソース設定(変数は使いまわし)
	resDesc.Format = m_TextureMetadata.format;
	resDesc.Width = m_TextureMetadata.width;
	resDesc.Height = m_TextureMetadata.height;
	resDesc.DepthOrArraySize = m_TextureMetadata.arraySize;
	resDesc.MipLevels = m_TextureMetadata.mipLevels;
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(m_TextureMetadata.dimension);
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	// テクスチャバッファーの作成
	result = m_Device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(m_TextureBuffer.ReleaseAndGetAddressOf()));

	// image->pixelsと同じ型にする
	uint8_t* mapforImg = nullptr;
	// マップ
	result = uploadBuff.Get()->Map(0, nullptr, (void**)&mapforImg);

	auto srcAddress = img->pixels;
	auto rowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	for (int y = 0; y < img->height; ++y)
	{
		std::copy_n(srcAddress, rowPitch, mapforImg);// コピー

		// 1行ごとのつじつまを合わせる
		srcAddress += img->rowPitch;
		mapforImg += rowPitch;
	}

	uploadBuff.Get()->Unmap(0, nullptr);

	// コマンドリストを使う前にResetする
	m_CommandAllocator->Reset();
	m_CommandList->Reset(m_CommandAllocator, nullptr);

	D3D12_TEXTURE_COPY_LOCATION src = {};
	//　コピー元
	src.pResource = uploadBuff.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = m_TextureMetadata.width;
	src.PlacedFootprint.Footprint.Height = m_TextureMetadata.height;
	src.PlacedFootprint.Footprint.Depth = m_TextureMetadata.depth;
	src.PlacedFootprint.Footprint.RowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	src.PlacedFootprint.Footprint.Format = img->format;

	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = m_TextureBuffer.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	m_CommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	// テクスチャをシェーダーリソースとして使用できるように状態遷移
	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = m_TextureBuffer.Get();
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	m_CommandList->ResourceBarrier(1, &BarrierDesc);

	// コマンドリストを閉じる
	m_CommandList->Close();

	// コマンドリストを実行
	ID3D12CommandList* cmdlists[] = { m_CommandList };
	m_CommandQueue->ExecuteCommandLists(1, cmdlists);

	// フェンスを使用してGPUの完了を待機
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceValue = 1;
	result = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));

	m_CommandQueue->Signal(fence.Get(), fenceValue);

	if (fence->GetCompletedValue() < fenceValue)
	{
		HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		fence->SetEventOnCompletion(fenceValue, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	return true;
}
