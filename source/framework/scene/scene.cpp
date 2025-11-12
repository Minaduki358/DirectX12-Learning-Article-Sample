#include"scene.h"

Scene::Scene()
{
	m_Camera = std::make_unique<Camera>();
}

Scene::~Scene()
{
}

bool Scene::InitRenderPass()
{
	for (auto& renderPass : m_RenderPasses)
	{
		bool result = renderPass->Init();

		if (result == false)
		{
			return false;
		}
	}

	return true;
}

void Scene::DrawBegin()
{
	for (auto& renderPass : m_RenderPasses)
	{
		renderPass->DrawBegin();
	}
}

void Scene::DrawEnd()
{
	for (auto& renderPass : m_RenderPasses)
	{
		renderPass->DrawEnd();
	}
}