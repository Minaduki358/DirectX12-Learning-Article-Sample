#pragma once

#include<mutex>
#include <cassert>

class SingletonFinalizer {
public:
	using FinalizerFunc = void(*)();
	static void addFinalizer(FinalizerFunc func);
	static void Finalize();
};

/**
* @brief シングルトンクラス
* @details これの継承先にシングルトンの振る舞いをさせる
* @details これの継承先のコンストラクタはprivateにし、これをfriendクラスにすること
*/
template<typename T>
class Singleton
{
private:
	static T* m_pInstance;
	static std::mutex m_Mutex;
	static std::once_flag initFlag;

protected:
	Singleton() = default;
	~Singleton() = default;

public:

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

	/**
	* @brief インスタンス取得関数
	* @return Singleton* クラスインスタンス
	* @details シングルトンを表す関数
	*/
	static T& GetInstance()
	{
		// 一度だけ処理をする
		std::call_once(initFlag, Create);
		//nullチェック
		assert(m_pInstance);
		return *m_pInstance;
	}

private:
	/**
	* @brief インスタンス作成関数
	*/
	static void Create()
	{
		if (m_pInstance == nullptr)
		{
			m_pInstance = new T;
			SingletonFinalizer::addFinalizer(&Singleton<T>::Destroy);
		}
	}

	/**
	* @brief インスタンス破棄関数
	*/
	static void Destroy()
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
};

template<typename T>
std::mutex Singleton<T>::m_Mutex;
template<typename T>
std::once_flag Singleton<T>::initFlag;
template<typename T>
T* Singleton<T>::m_pInstance = nullptr;
