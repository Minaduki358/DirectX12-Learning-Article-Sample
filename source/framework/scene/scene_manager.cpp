#include"scene_manager.h"

bool SceneManager::Init()
{
	return true;
}

void SceneManager::Uninit()
{
	if (m_CurrentScene)
	{
		m_CurrentScene->Uninit();
	}

	m_Scenes.clear();
	m_CurrentScene = nullptr;
	m_CurrentSceneName.clear();
}

void SceneManager::Update()
{
	if (m_NextSceneRequested)
	{
		ChangeSceneInternal();
		m_NextSceneRequested = false;
	}

	if (m_CurrentScene)
	{
		m_CurrentScene->Update();
	}
}

void SceneManager::DrawBegin()
{
	if (m_CurrentScene)
	{
		m_CurrentScene->DrawBegin();
	}
}

void SceneManager::DrawEnd()
{
	if (m_CurrentScene)
	{
		m_CurrentScene->DrawEnd();
	}
}

void SceneManager::RequestChangeScene(const std::string& sceneName)
{
	auto it = m_Scenes.find(sceneName);
	if (it == m_Scenes.end())
	{
		return;
	}

	if (m_NextSceneRequested && m_NextSceneName == sceneName)
	{
		return;
	}

	m_NextSceneRequested = true;
	m_NextSceneName = sceneName;
}

bool SceneManager::ChangeSceneInternal()
{
	auto it = m_Scenes.find(m_NextSceneName);
	if (it == m_Scenes.end())
	{
		return false;
	}

	if (m_CurrentScene)
	{
		m_CurrentScene->Uninit();
	}

	m_CurrentScene = it->second.get();
	m_CurrentSceneName = m_NextSceneName;

	if (m_CurrentScene->Init() == false)
	{
		return false;
	}

	if (m_CurrentScene->InitRenderPass() == false)
	{
		return false;
	}

	return true;
}