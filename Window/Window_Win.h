/*===================================================================================
*    Date : 2022/10/28(土)
*        Author		: Gakuto.S
*        File		: Window_Win.h
*        Detail		: ウィンドウズAPIを使用してウィンドウの生成を行う
===================================================================================*/
#ifndef WINDOW_WIN_H_
#define WINDOW_WIN_H_

#ifdef _WIN32
#include <Windows.h>

class WindowWin
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="width">幅</param>
	/// <param name="height">高さ</param>
	/// <param name="caption">キャプション</param>
	/// <param name="proc">プロシージャ</param>
	WindowWin(int width, int height, LPCWSTR caption, WNDPROC proc);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~WindowWin();

public:
	/// <summary>
	/// ウィンドウの表示
	/// </summary>
	void ShowUp();

	/// <summary>
	/// ウィンドウの状態
	/// </summary>
	/// <returns>true : ウィンドウを閉じる</returns>
	bool Close();

	/// <summary>
	/// ウィンドウハンドルの取得
	/// </summary>
	/// <returns>HWND</returns>
	HWND GetHandle();

	/// <summary>
	/// ウィンドウの幅を取得する
	/// </summary>
	/// <returns>幅</returns>
	unsigned int GetWidth();

	/// <summary>
	/// ウィンドウの高さを取得する
	/// </summary>
	/// <returns>高さ</returns>
	unsigned int GetHeight();

private:
	HWND					m_hWnd;
	HINSTANCE				m_hInstance;
	const LPCWSTR			k_className;
};
#endif	// WINDOWS_PLATFORM

#endif // !WINDOW_WIN_H_