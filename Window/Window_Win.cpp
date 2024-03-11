/*===================================================================================
*    Date : 2022/10/28(Sat)
*        Author		: Gakuto.S
*        File		: Window_Win.h
*        Detail		: Create a window for WINDOWS using WinAPI.
===================================================================================*/
#include "Window_Win.h"

#ifdef _WIN32

#include <thread>
#include <assert.h>

/// <summary>
/// Constructor
/// </summary>
/// <param name="width">width</param>
/// <param name="height">height</param>
/// <param name="caption">caption</param>
/// <param name="proc">procedure</param>
WindowWin::WindowWin(int width, int height, LPCWSTR caption, WNDPROC proc)
	:m_hInstance(GetModuleHandle(nullptr)),
	k_className(caption)
{
	HRESULT ret = CoInitializeEx(nullptr, COINIT::COINIT_MULTITHREADED);
	if (FAILED(ret))
		throw("ComÇÃèâä˙âªÇ…é∏îs");	// Failed to initialize COM library.

	WNDCLASSEX wndEx{};
	wndEx.cbSize		= sizeof(WNDCLASSEX);
	wndEx.lpfnWndProc	= proc;
	wndEx.style			= CS_CLASSDC;
	wndEx.hInstance		= m_hInstance;
	wndEx.lpszClassName = k_className;
	RegisterClassEx(&wndEx);

	m_hWnd = CreateWindow(
		k_className,
		caption,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width, height,
		nullptr,
		nullptr,
		wndEx.hInstance,
		nullptr
	);
}


WindowWin::~WindowWin()
{
	UnregisterClass(k_className, m_hInstance);
	CoUninitialize();
}


void WindowWin::ShowUp()
{
	// Show up the window
	ShowWindow(m_hWnd, true);
	UpdateWindow(m_hWnd);
}


bool WindowWin::Close()
{
	MSG msg{};
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return true;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return false;
}

HWND WindowWin::GetHandle()
{
	return m_hWnd;
}

unsigned int WindowWin::GetWidth()
{
	RECT rect{};
	GetWindowRect(m_hWnd, &rect);
	return rect.right - rect.left;
}

unsigned int WindowWin::GetHeight()
{
	RECT rect{};
	GetWindowRect(m_hWnd, &rect);
	return rect.bottom - rect.top;
}

#endif	// _WIN32