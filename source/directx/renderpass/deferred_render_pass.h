#pragma once

#include"render_pass.h"
#include"../resource/texture/texture.h"
#include"../resource/constant_buffer/constant_buffer.h"
#include"../resource/rendertexture/rendertexture.h"
#include"../resource/rendertexture/depthstenciltexture.h"

class DeferredRenderPass : public RenderPass
{
public:
	DeferredRenderPass(const Camera* camera);
	~DeferredRenderPass() override;

	bool Init() override;
	void DrawBegin() override;
	void Execute() override;
	void DrawEnd() override;

private:
	ConstantBuffer* m_WorldMatrixConstantBuffer = nullptr;
	ConstantBuffer* m_CameraConstantBuffer = nullptr;
	Texture* m_Texture = nullptr;

	// MRT用のRenderTexture
	RenderTexture* m_ColorTarget;
	RenderTexture* m_NormalTarget;

	// MRT用のDepthStencilTexture
	DepthStencilTexture* m_DepthStencilTexture;
};