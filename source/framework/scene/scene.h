#pragma once

#include"../../directx/renderpass/render_pass.h"
#include"../../directx/camera/camera.h"

class Scene
{
private:
public:
	Scene();
	virtual ~Scene();

	bool InitRenderPass();

	virtual bool Init() = 0;
	virtual void Uninit() = 0;
	virtual void Update() = 0;

	virtual void DrawBegin();
	virtual void Draw();
	virtual void DrawEnd();

protected:
	/// <summary>
	/// シーンを登録
	/// </summary>
	template<typename T>
	void RegisterRenderPass(const Camera* camera)
	{
		static_assert(std::is_base_of<RenderPass, T>::value, "T must inherit from RenderPass");
		auto renderPass = std::make_unique<T>(camera);
		m_RenderPasses.push_back(std::move(renderPass));
	}

protected:
	std::vector<std::unique_ptr<RenderPass>> m_RenderPasses;
	std::unique_ptr<Camera> m_Camera;
};