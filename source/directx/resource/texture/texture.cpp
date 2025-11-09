#include"texture.h"

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

	// テクスチャのためのヒープ設定
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;// テクスチャ用
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texHeapProp.CreationNodeMask = 0;
	texHeapProp.VisibleNodeMask = 0;

	// テクスチャリソースの設定
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Format = m_TextureMetadata.format;
	texDesc.Width = m_TextureMetadata.width;
	texDesc.Height = m_TextureMetadata.height;
	texDesc.DepthOrArraySize = m_TextureMetadata.arraySize;
	texDesc.MipLevels = m_TextureMetadata.mipLevels;
	texDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(m_TextureMetadata.dimension);
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Alignment = 0;

	// テクスチャバッファーの作成
	HRESULT result = m_Device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(m_TextureBuffer.ReleaseAndGetAddressOf()));

	if (FAILED(result))
	{
		return false;
	}

	// GetCopyableFootprintsを使用してレイアウト情報を取得
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout = {};
	UINT numRows = 0;
	UINT64 rowSizeInBytes = 0;
	UINT64 totalBytes = 0;

	m_Device->GetCopyableFootprints(
		&texDesc,           // テクスチャの情報
		0,                  // 最初のサブリソース（ミップレベル0）
		1,                  // サブリソース数（1つ）
		0,                  // ベースオフセット
		&layout,            // 出力: レイアウト情報
		&numRows,           // 出力: 行数
		&rowSizeInBytes,    // 出力: 1行のバイト数（実データ）
		&totalBytes         // 出力: 必要な合計バイト数
	);

	// 中間バッファーとしてのアップロードヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProp = {};
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	uploadHeapProp.CreationNodeMask = 0;
	uploadHeapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC uploadBuffDesc = {};
	uploadBuffDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	uploadBuffDesc.Alignment = 0;
	uploadBuffDesc.Width = totalBytes;  // GetCopyableFootprintsで取得した値を使用
	uploadBuffDesc.Height = 1;
	uploadBuffDesc.DepthOrArraySize = 1;
	uploadBuffDesc.MipLevels = 1;
	uploadBuffDesc.Format = DXGI_FORMAT_UNKNOWN;
	uploadBuffDesc.SampleDesc.Count = 1;
	uploadBuffDesc.SampleDesc.Quality = 0;
	uploadBuffDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	uploadBuffDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ComPtr<ID3D12Resource> uploadBuff = nullptr;
	// 中間バッファー作成
	result = m_Device->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&uploadBuffDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result))
	{
		return false;
	}

	// データのコピー（Mapして書き込み）
	uint8_t* mapforImg = nullptr;
	result = uploadBuff->Map(0, nullptr, (void**)&mapforImg);
	if (FAILED(result))
	{
		return false;
	}

	auto srcAddress = img->pixels;
	auto dstRowPitch = layout.Footprint.RowPitch;  // GetCopyableFootprintsで取得した行ピッチ
	auto srcRowPitch = img->rowPitch;               // 元画像の行ピッチ

	for (UINT y = 0; y < numRows; ++y)  // GetCopyableFootprintsで取得した行数
	{
		// 実データだけをコピー
		std::memcpy(mapforImg, srcAddress, srcRowPitch);

		srcAddress += srcRowPitch;
		mapforImg += dstRowPitch;
	}

	uploadBuff->Unmap(0, nullptr);

	// コマンドリストを使う前にResetする
	m_CommandAllocator->Reset();
	m_CommandList->Reset(m_CommandAllocator, nullptr);

	// コピー元の設定
	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = uploadBuff.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint = layout;  // GetCopyableFootprintsで取得した情報をそのまま使用

	// コピー先の設定
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

	if (FAILED(result))
	{
		return false;
	}

	m_CommandQueue->Signal(fence.Get(), fenceValue);

	if (fence->GetCompletedValue() < fenceValue)
	{
		HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (event == nullptr)
		{
			return false;
		}

		fence->SetEventOnCompletion(fenceValue, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	return true;
}
