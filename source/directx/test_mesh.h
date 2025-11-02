#pragma once

#include"vertex_helper.h"

class TestMesh
{
public:
	bool Init();
	void Draw();
private:
	void CreateMesh();
	bool CreateVertexBuffer();
	bool CreateIndexBuffer();

private:
	VertexPosUv m_Vertices[4];
	unsigned short m_Indices[6];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView = {};
};