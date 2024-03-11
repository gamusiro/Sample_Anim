/*===================================================================================
*    Date : 2023/06/22(木)
*        Author		: Gakuto.S
*        File		: main.cpp
*        Detail		:
===================================================================================*/
#include "Application.h"

#ifdef _WIN32

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Keyboard::Update();

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:		// キーが押されたら
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		default:
			break;
		}
		break;

	case WM_MOUSEWHEEL:
		Mouse::SetWheelDelta(float(GET_WHEEL_DELTA_WPARAM(wParam)));
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


#ifdef _DEBUG
int main()
{
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#else
int _stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
#endif // _DEBUG
	// ウィンドウの生成
	WindowWin window(1920, 1080, L"App <DX11>", WndProc);

	Application& app = Application::Instance();

	if (app.Init(&window))
	{
		while (!app.Close())
		{
			app.Update();
			app.Draw();
		}
		app.Uninit();
	}

	return 0;
}

#endif // _WIN32