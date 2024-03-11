/*===================================================================================
*    Date : 2023/05/17(��)
*        Author		: Gakuto.S
*        File		: Application.h
*        Detail		: �V���O���g���Ŏ���
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
	/// �R���X�g���N�^
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
	/// �N���X�̃C���X�^���X���擾����
	/// </summary>
	/// <returns></returns>
	static Application& Instance();

	/// <summary>
	/// ����������
	/// </summary>
	/// <param name="master">���C���E�B���h�E</param>
	/// <returns>true : ����I��</returns>
#ifdef _WIN32
	bool Init(WindowWin* master);
#endif // _WIN32

	/// <summary>
	/// �I������
	/// </summary>
	void Uninit();

	/// <summary>
	/// �E�B���h�E��Ԃ̔c��
	/// </summary>
	/// <returns>true : �E�B���h�E�����</returns>
	bool Close();

	/// <summary>
	/// �X�V����
	/// </summary>
	void Update();

	/// <summary>
	/// �`�揈��
	/// </summary>
	void Draw();

public:
	/// <summary>
	/// 1�t���[���ɂ�����������(�~��)
	/// </summary>
	float GetDeltaTime() { return m_deltaTime; };

public:
	/// <summary>
	/// �O���t�B�b�N�����N���X�̃|�C���^���擾
	/// </summary>
	/// <returns></returns>
	GraphicsDX11* Graphics() { return m_graphics; }

private:
#ifdef _WIN32
	WindowWin*		m_mainWindow;
	GraphicsDX11*	m_graphics;
#endif // _WIN32

	float m_moveVelocity	= 1.0f;		// �ړ����x
	float m_eyeVelocity		= 0.005f;	// �}�E�X���x
	float m_deltaTime;					// �f���^�^�C��
	std::chrono::system_clock::time_point m_lastTime;
};

#endif // !APPLICATION_H_