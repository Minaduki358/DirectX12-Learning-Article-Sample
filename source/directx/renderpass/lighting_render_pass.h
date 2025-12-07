#pragma once

#include"render_pass.h"
#include"../resource/texture/texture.h"
#include"../resource/constant_buffer/constant_buffer.h"
#include"../resource/rendertexture/rendertexture.h"
#include"../resource/rendertexture/depthstenciltexture.h"

class LightingRenderPass : public RenderPass
{
public:
	LightingRenderPass(const Camera* camera);
	~LightingRenderPass() override;

	bool Init() override;
	void Uninit() override;
	void DrawBegin() override;
	void Draw() override;
	void DrawEnd() override;

private:
	// MRT用のRenderTexture
	RenderTexture* m_AlbedoTarget = nullptr;
	RenderTexture* m_NormalTarget = nullptr;

	RenderTexture* m_RenderTarget = nullptr;
};