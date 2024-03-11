/*===================================================================================
*    Date : 2022/10/28(�y)
*        Author		: Gakuto.S
*        File		: Window_Win.h
*        Detail		: �E�B���h�E�YAPI���g�p���ăE�B���h�E�̐������s��
===================================================================================*/
#ifndef WINDOW_WIN_H_
#define WINDOW_WIN_H_

#ifdef _WIN32
#include <Windows.h>

class WindowWin
{
public:
	/// <summary>
	/// �R���X�g���N�^
	/// </summary>
	/// <param name="width">��</param>
	/// <param name="height">����</param>
	/// <param name="caption">�L���v�V����</param>
	/// <param name="proc">�v���V�[�W��</param>
	WindowWin(int width, int height, LPCWSTR caption, WNDPROC proc);

	/// <summary>
	/// �f�X�g���N�^
	/// </summary>
	~WindowWin();

public:
	/// <summary>
	/// �E�B���h�E�̕\��
	/// </summary>
	void ShowUp();

	/// <summary>
	/// �E�B���h�E�̏��
	/// </summary>
	/// <returns>true : �E�B���h�E�����</returns>
	bool Close();

	/// <summary>
	/// �E�B���h�E�n���h���̎擾
	/// </summary>
	/// <returns>HWND</returns>
	HWND GetHandle();

	/// <summary>
	/// �E�B���h�E�̕����擾����
	/// </summary>
	/// <returns>��</returns>
	unsigned int GetWidth();

	/// <summary>
	/// �E�B���h�E�̍������擾����
	/// </summary>
	/// <returns>����</returns>
	unsigned int GetHeight();

private:
	HWND					m_hWnd;
	HINSTANCE				m_hInstance;
	const LPCWSTR			k_className;
};
#endif	// WINDOWS_PLATFORM

#endif // !WINDOW_WIN_H_