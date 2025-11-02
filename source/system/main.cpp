#include"../system/mywindow.h"
#include"../directx/renderer.h"
#include"../directx/test_mesh.h"

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

	TestMesh* test = new TestMesh();
	test->Init();

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
			renderer.DrawBegin();
			test->Draw();
			renderer.DrawEnd();
		}
	}

	delete test;

	// 終了処理
	renderer.Uninit();
	myWindow.Uninit();

	// シングルトン終了
	SingletonFinalizer::Finalize();

	// COMライブラリの終了処理
	CoUninitialize();

	return (int)msg.wParam;
}