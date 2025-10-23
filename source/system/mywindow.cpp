#include "mywindow.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

/// <summary>
/// ウィンドウプロシージャ関数
/// </summary>
LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE) {
			SendMessage(hwnd, WM_CLOSE, 0, 0);
		}
	case WM_ACTIVATEAPP:
	case WM_INPUT:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		break;

	case WM_CLOSE:
		if (MessageBoxW(hwnd, L"本当に終了してもよろしいですか？", L"確認", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK)
		{
			DestroyWindow(hwnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	};

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void MyWindow::Uninit()
{
	// 登録済みクラス解除
	UnregisterClass(WindowData::k_ClassName, GetModuleHandle(NULL));
}

bool MyWindow::EntryWindowClass()
{
	// ウィンドウクラス構造体初期化
	WNDCLASSEX windowClass = {
	sizeof(WNDCLASSEX),
	CS_HREDRAW | CS_VREDRAW,
	WndProc,
	0,
	0,
	GetModuleHandle(NULL),
	NULL,
	LoadCursor(NULL, IDC_ARROW),
	(HBRUSH)(COLOR_WINDOW + 1),
	NULL,
	WindowData::k_ClassName,				
	NULL	
	};

	// 構造体の登録
	if (RegisterClassEx(&windowClass) == 0)
	{
		return false;
	}

	return true;
}

bool MyWindow::Create()
{
	// ウィンドウクラスの初期化と登録
	if (EntryWindowClass() == false)
	{
		return false;
	}

	// ウィンドウスタイル
	const DWORD WINDOW_STYLE = WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);

	// ウィンドウサイズの算出	
	RECT windowRect = { 0, 0, SystemData::k_ScreenWidth, SystemData::k_ScreenHeight };
	AdjustWindowRect(&windowRect, WINDOW_STYLE, FALSE);

	// ウィンドウのクライアント領域と実際の大きさを求める
	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	RECT desktopRect;
	GetWindowRect(GetDesktopWindow(), &desktopRect);

	int desktop_width = desktopRect.right - desktopRect.left;
	int desktop_height = desktopRect.bottom - desktopRect.top;

	int windowX = (std::max)((desktop_width - windowWidth) / 2, 0);
	int windowY = (std::max)((desktop_height - windowHeight) / 2, 0);

	// ウィンドウの作成
	hwnd = CreateWindow(
		WindowData::k_ClassName,
		WindowData::k_TitleName,	
		WINDOW_STYLE,				
		windowX,							
		windowY,							
		windowWidth,					
		windowHeight,
		NULL,
		NULL,
		GetModuleHandle(NULL),	
		NULL
	);

	if (hwnd == nullptr)
	{
		return false;
	}

	ShowWindow(hwnd, SW_SHOW);

	if (UpdateWindow(hwnd) == FALSE)
	{
		return false;
	}

	return true;
}