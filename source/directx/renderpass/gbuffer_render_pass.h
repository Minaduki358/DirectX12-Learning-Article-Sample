#pragma once

#include"render_pass.h"
#include"../resource/texture/texture.h"
#include"../resource/constant_buffer/constant_buffer.h"
#include"../resource/rendertexture/rendertexture.h"
#include"../resource/rendertexture/depthstenciltexture.h"
#include"../test_mesh.h"

class GBufferRenderPass : public RenderPass
{
public:
	GBufferRenderPass(const Camera* camera);
	~GBufferRenderPass() override;

	bool Init() override;
	void Uninit() override;
	void DrawBegin() override;
	void Draw() override;
	void DrawEnd() override;

	void AddMesh(std::unique_ptr<TestMesh> testMesh);

private:
	ConstantBuffer* m_WorldMatrixConstantBuffer = nullptr;
	ConstantBuffer* m_CameraConstantBuffer = nullptr;
	Texture* m_Texture = nullptr;

	// MRT用のRenderTexture
	RenderTexture* m_ColorTarget;
	RenderTexture* m_NormalTarget;

	// MRT用のDepthStencilTexture
	DepthStencilTexture* m_DepthStencilTexture;

	std::vector<std::unique_ptr<TestMesh>> m_TestMeshes;
};