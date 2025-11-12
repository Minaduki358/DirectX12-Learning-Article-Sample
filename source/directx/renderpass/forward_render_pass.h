#pragma once

#include"render_pass.h"
#include"../resource/texture/texture.h"
#include"../resource/constant_buffer/constant_buffer.h"

class ForwardRenderPass : public RenderPass
{
public:
	ForwardRenderPass(const Camera* camera);
	~ForwardRenderPass() override;

	bool Init() override;
	void DrawBegin() override;
	void DrawEnd() override;

private:
	ConstantBuffer* m_CameraConstantBuffer = nullptr;
	Texture* m_Texture = nullptr;
};