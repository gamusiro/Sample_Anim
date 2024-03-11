/*===================================================================================
*    Date : 2023/05/17(水)
*        Author		: Gakuto.S
*        File		: Application.h
*        Detail		: シングルトンで実装
===================================================================================*/
#ifndef APPLICATION_H_
#define APPLICATION_H_

#define _DIRECTX

#include "Config.h"
#include "Common/FileIO.h"
#include "Common/Load_Texture.h"

#ifdef _WIN32
#include "Window/Window_Win.h"
#include "Graphics/Graphics_DX11.h"
#include "Inputs/Keyboard.h"
#include "Inputs/Mouse.h"
#endif // _WIN32

class Application
{
private:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Application()
		:m_lastTime(std::chrono::system_clock::now()),
		 m_deltaTime(0.0f)
	{}

public:
	Application(Application&)	= delete;
	Application(Application*)	= delete;

public:
	/// <summary>
	/// クラスのインスタンスを取得する
	/// </summary>
	/// <returns></returns>
	static Application& Instance();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="master">メインウィンドウ</param>
	/// <returns>true : 正常終了</returns>
#ifdef _WIN32
	bool Init(WindowWin* master);
#endif // _WIN32

	/// <summary>
	/// 終了処理
	/// </summary>
	void Uninit();

	/// <summary>
	/// ウィンドウ状態の把握
	/// </summary>
	/// <returns>true : ウィンドウを閉じる</returns>
	bool Close();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

public:
	/// <summary>
	/// 1フレームにかかった時間(ミリ)
	/// </summary>
	float GetDeltaTime() { return m_deltaTime; };

public:
	/// <summary>
	/// グラフィック処理クラスのポインタを取得
	/// </summary>
	/// <returns></returns>
	GraphicsDX11* Graphics() { return m_graphics; }

private:
#ifdef _WIN32
	WindowWin*		m_mainWindow;
	GraphicsDX11*	m_graphics;
#endif // _WIN32

	float m_moveVelocity	= 1.0f;		// 移動速度
	float m_eyeVelocity		= 0.005f;	// マウス感度
	float m_deltaTime;					// デルタタイム
	std::chrono::system_clock::time_point m_lastTime;
};

#endif // !APPLICATION_H_