#include"test_mesh.h"
#include"renderer.h"

using namespace DirectX;

TestMesh::TestMesh(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
	: m_Device(device)
	, m_CommandList(commandList)
{
}

bool TestMesh::Init()
{
	CreateMesh();

	if (CreateVertexBuffer() == false)
	{
		return false;
	}

	if (CreateIndexBuffer() == false)
	{
		return false;
	}

	return true;
}

void TestMesh::Draw()
{
	m_CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	m_CommandList->IASetIndexBuffer(&m_IndexBufferView);
	m_CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void TestMesh::CreateMesh()
{
	m_Vertices[0] = { {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} };
	m_Vertices[1] = { {-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f} };
	m_Vertices[2] = { {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f} };
	m_Vertices[3] = { {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f} };

	m_Indices[0] = 0;
	m_Indices[1] = 1;
	m_Indices[2] = 2;
	m_Indices[3] = 2;
	m_Indices[4] = 1;
	m_Indices[5] = 3;
}

bool TestMesh::CreateVertexBuffer()
{
	D3D12_HEAP_PROPERTIES heapprop = {};

	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};

	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(m_Vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT result = m_Device->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_VertexBuffer.ReleaseAndGetAddressOf()));

	if (FAILED(result))
	{
		return false;
	}

	VertexPosUv* vertMap = nullptr;

	result = m_VertexBuffer->Map(0, nullptr, (void**)&vertMap);

	if (FAILED(result))
	{
		return false;
	}

	std::copy(std::begin(m_Vertices), std::end(m_Vertices), vertMap);

	m_VertexBuffer->Unmap(0, nullptr);

	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress(); // バッファーの仮想アドレス
	m_VertexBufferView.SizeInBytes = sizeof(m_Vertices);      // 全バイト数
	m_VertexBufferView.StrideInBytes = sizeof(m_Vertices[0]); // 1頂点あたりのバイト数

	return true;
}

bool TestMesh::CreateIndexBuffer()
{
	D3D12_HEAP_PROPERTIES heapprop = {};

	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	// 設定は、バッファーのサイズ以外頂点バッファーの設定を使いまわす
	D3D12_RESOURCE_DESC resdesc = {};

	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(m_Indices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT result = m_Device->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_IndexBuffer.ReleaseAndGetAddressOf()));

	if (FAILED(result))
	{
		return false;
	}

	// 作ったバッファーにインデックスデータをコピー
	unsigned short* mappedIndex = nullptr;
	result = m_IndexBuffer->Map(0, nullptr, (void**)&mappedIndex);

	if (FAILED(result))
	{
		return false;
	}

	std::copy(std::begin(m_Indices), std::end(m_Indices), mappedIndex);
	m_IndexBuffer->Unmap(0, nullptr);

	// インデックスバッファービューを作成
	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	m_IndexBufferView.SizeInBytes = sizeof(m_Indices);

	return true;
}
