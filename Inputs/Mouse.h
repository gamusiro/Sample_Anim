/*===================================================================================
*    Date : 2023/06/28(��)
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
	/// �J�[�\���̍X�V
	/// </summary>
	static void Update();

	/// <summary>
	/// �J�[�\���|�W�V�����̐ݒ�(��ʍ��W)
	/// </summary>
	static void SetCursourPos(int x, int y);

	/// <summary>
	/// �z�C�[���ړ�
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