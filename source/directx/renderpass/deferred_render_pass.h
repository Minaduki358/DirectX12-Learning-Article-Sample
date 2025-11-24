#pragma once

#include"render_pass.h"
#include"../resource/texture/texture.h"
#include"../resource/constant_buffer/constant_buffer.h"
#include"../resource/rendertexture/rendertexture.h"

class DeferredRenderPass : public RenderPass
{
public:
	DeferredRenderPass(const Camera* camera);
	~DeferredRenderPass() override;

	bool Init() override;
	void DrawBegin() override;
	void DrawEnd() override;

private:
	ConstantBuffer* m_WorldMatrixConstantBuffer = nullptr;
	ConstantBuffer* m_CameraConstantBuffer = nullptr;
	Texture* m_Texture = nullptr;

	// ★ MRT用のRenderTexture ★
	std::unique_ptr<RenderTexture> m_ColorTarget;    // カラー
	std::unique_ptr<RenderTexture> m_NormalTarget;   // 法線など
};