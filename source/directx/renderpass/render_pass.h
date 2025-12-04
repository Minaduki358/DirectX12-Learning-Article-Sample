#pragma once

#include"../camera/camera.h"

class RenderPass
{
public:
	RenderPass(const Camera* camera);
	virtual ~RenderPass();

	virtual bool Init() = 0;
	virtual void DrawBegin() = 0;
	virtual void Execute() = 0;
	virtual void DrawEnd() = 0;

protected:
	const Camera* m_Camera= nullptr;
};