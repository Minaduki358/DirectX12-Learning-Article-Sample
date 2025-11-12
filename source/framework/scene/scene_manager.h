#pragma once

#include"../../function/singleton.h"
#include"scene.h"
#include<string>
#include<memory>
#include<unordered_map>

class SceneManager : public Singleton<SceneManager>
{
private:
	friend class Singleton<SceneManager>;

	SceneManager() = default;
	~SceneManager() = default;

public:

	bool Init();

	void Uninit();

	void Update();

	void DrawBegin();

	void DrawEnd();

	/// <summary>
	/// シーンを登録
	/// </summary>
	template<typename T>
	void RegisterScene(const std::string& name)
	{
		static_assert(std::is_base_of<Scene, T>::value, "T must inherit from Scene");
		auto scene = std::make_unique<T>();
		m_Scenes[name] = std::move(scene);
	}

	/// <summary>
	/// シーン切り替えを予約
	/// </summary>
	void RequestChangeScene(const std::string& sceneName);

private:
	/// <summary>
	/// 実際のシーン切り替え処理
	/// </summary>
	bool ChangeSceneInternal();

private:
	Scene* m_CurrentScene = nullptr;
	std::string m_CurrentSceneName;
	std::unordered_map<std::string, std::unique_ptr<Scene>> m_Scenes;

	bool m_NextSceneRequested = false;
	std::string m_NextSceneName;
};