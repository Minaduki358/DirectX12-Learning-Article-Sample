#include"../system/mywindow.h"
#include"../directx/renderer.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// 参照しないパラメーター
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	// メモリリーク通知
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COMライブラリの初期化
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	// インスタンス作成
	MyWindow& myWindow = MyWindow::GetInstance();
	Renderer& renderer = Renderer::GetInstance();

	// ウィンドウ生成
	if (myWindow.Create() == false)
	{
		return 0;
	}

	if (renderer.Init() == false)
	{
		return 0;
	}

	MSG msg;
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{

		}
	}

	// ウィンドウ終了処理
	myWindow.Uninit();

	// シングルトン終了
	SingletonFinalizer::Finalize();

	// COMライブラリの終了処理
	CoUninitialize();

	return (int)msg.wParam;
}