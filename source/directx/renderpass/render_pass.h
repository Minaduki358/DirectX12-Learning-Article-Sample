#pragma once

#include"../camera/camera.h"

class RenderPass
{
public:
	RenderPass(const Camera* camera);
	virtual ~RenderPass();

	virtual bool Init() = 0;
	virtual void Uninit() = 0;
	virtual void DrawBegin() = 0;
	virtual void Draw() = 0;
	virtual void DrawEnd() = 0;

protected:
	/// <summary>
	/// シザー矩形と画面解像度でビューポートを設定
	/// </summary>
	void SetupDefaultViewportAndScissor();

protected:
	const Camera* m_Camera= nullptr;
	D3D12_VIEWPORT m_ViewPort = {};
	D3D12_RECT m_ScissorRec = {};
};