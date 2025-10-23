#pragma once

#include"../function/singleton.h"

/// <summary>
/// ウィンドウの情報
/// </summary>
namespace WindowData
{
	// ウィンドウタイトルバーの名前
	static const wchar_t* k_ClassName = L"AppClass";

	// ウィンドウタイトルバーの名前
	static const wchar_t* k_TitleName = L"DirectX12Sample";
}

/// <summary>
/// ウィンドウ管理クラス
/// </summary>
class MyWindow : public Singleton<MyWindow>
{
private:
	// ウィンドウハンドル
	HWND hwnd = nullptr;

private:
	friend class Singleton<MyWindow>;

	MyWindow() = default;
	~MyWindow() = default;

public:

	/// <summary>
	/// ウィンドウの生成
	/// </summary>
	bool Create();

	/// <summary>
	/// 終了関数
	/// </summary>
	void Uninit();

	/// <summary>
	/// HWND取得
	/// </summary>
	inline HWND GetHWND() const
	{
		return hwnd;
	}

private:

	/// <summary>
	/// ウィンドウクラスの初期化、登録
	/// </summary>
	bool EntryWindowClass();
};
