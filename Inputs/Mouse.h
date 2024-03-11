/*===================================================================================
*    Date : 2023/06/28(水)
*        Author		: Gakuto.S
*        File		: Mouse.h
*        Detail		:
===================================================================================*/
#ifndef MOUSE_H_
#define MOUSE_H_

#include <Windows.h>

class Mouse
{
public:
	/// <summary>
	/// カーソルの更新
	/// </summary>
	static void Update();

	/// <summary>
	/// カーソルポジションの設定(画面座標)
	/// </summary>
	static void SetCursourPos(int x, int y);

	/// <summary>
	/// ホイール移動
	/// </summary>
	static void SetWheelDelta(float delta);

	static POINT GetCurrnet() { return m_currently; }
	static POINT GetLast() { return m_last; }
	static float GetDelta() { return m_delta; }

private:
	static POINT m_currently;
	static POINT m_last;
	static float m_delta;
};

#endif // !MOUSE_H_