#pragma once
#include"render_pass.h"
#include"../resource/texture/texture.h"
#include"../resource/constant_buffer/constant_buffer.h"
#include"../resource/rendertexture/rendertexture.h"

class FinalBlitRenderPass : public RenderPass
{
public:
	FinalBlitRenderPass(const Camera* camera);
	~FinalBlitRenderPass() override;

	bool Init() override;
	void DrawBegin() override;
	void Execute() override;
	void DrawEnd() override;

private:
	RenderTexture* m_SourceTexture = nullptr;
};