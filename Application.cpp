/*===================================================================================
*    Date : 2023/05/17(水)
*        Author		: Gakuto.S
*        File		: Application.cpp
*        Detail		:
===================================================================================*/
#include "Application.h"


// TEST
#include "Objects/Sprite/Object_Sprite.h"
#include "Objects/Model/Object_ModelglTF.h"

ModelglTF* g_stem;
ModelglTF* g_polly;


Application& Application::Instance()
{
	static Application app;
	return app;
}

#ifdef _WIN32
bool Application::Init(WindowWin* master)
{
	if (master == nullptr)
		return false;

	// ウィンドウの登録処理
	m_mainWindow = master;
	master->ShowUp();

	// グラフィックのセットアップ
	m_graphics = new GraphicsDX11();
	m_graphics->Init(master);

	// カメラ設定
	CAMERA camera;
	camera.position = glm::vec3( 0.0f,  1.0f,  5.0f);
	camera.rotation	= glm::vec3( 0.0f,  135.0f,  0.0f);
	m_graphics->SetCamera(camera);

	g_stem = new ModelglTF("Assets/Model/ToyCar.glb");
	g_stem->Init();
	g_stem->m_position = glm::vec3(-1.0, 0.0, 0.0);
	g_stem->m_scale = glm::vec3(1.0f) * 50.0f;

	g_polly = new ModelglTF("Assets/Model/Polly.glb");
	g_polly->Init();
	g_polly->m_position = glm::vec3(1.0, 0.0, 0.0);
	g_polly->m_scale = glm::vec3(1.0f) * 1.02f;

	return true;
}
#endif // _WIN32

void Application::Uninit()
{
	g_polly->Uninit();
	delete g_polly;

	g_stem->Uninit();
	delete g_stem;

	m_graphics->Uninit();
	delete m_graphics;
}

bool Application::Close()
{
	if (m_mainWindow == nullptr)
		return true;

	auto currently = std::chrono::system_clock::now();
	long long micro = std::chrono::duration_cast<std::chrono::microseconds> (currently - m_lastTime).count(); //処理に要した時間をミリ秒に変換
	m_deltaTime = float(float(micro) / 1000000.0f);
	
	m_lastTime = currently;

	return m_mainWindow->Close();
}

void Application::Update()
{
	// カメラの更新
	CAMERA cam = m_graphics->GetCamera();
	Mouse::Update();

	glm::mat4 camMat = m_graphics->GetCameraMatrix();
	glm::vec3 forward = m_graphics->GetForward(camMat);
	glm::vec3 right = m_graphics->GetRight(camMat);

	// カメラ移動
	if		(Keyboard::GetKeyPress('W'))
		cam.position += forward * m_moveVelocity * m_deltaTime;
	else if (Keyboard::GetKeyPress('S'))
		cam.position -= forward * m_moveVelocity * m_deltaTime;

	// 拡大縮小
	cam.position += glm::vec3(Mouse::GetDelta() * m_eyeVelocity) * forward * m_deltaTime;
	Mouse::SetWheelDelta(0.0f);

	if (Keyboard::GetKeyPress('D'))
		cam.position += right * m_moveVelocity * m_deltaTime;
	else if (Keyboard::GetKeyPress('A'))
		cam.position -= right * m_moveVelocity * m_deltaTime;

	// 注視点移動
	if (Keyboard::GetKeyPress(VK_RBUTTON))
	{
		POINT cur = Mouse::GetCurrnet();
		POINT old = Mouse::GetLast();
		cam.rotation.x += float(cur.y - old.y) * m_eyeVelocity;
		cam.rotation.y += float(cur.x - old.x) * m_eyeVelocity;
	}

	m_graphics->SetCamera(cam);


	// モデルの更新
	g_stem->Update();
	g_polly->Update();
}

void Application::Draw()
{
	m_graphics->Clear();

	g_stem->Draw();
	g_polly->Draw();

	m_graphics->Present();
}