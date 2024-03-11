/*===================================================================================
*    Date : 2023/06/23(金)
*        Author		: Gakuto.S
*        File		: Object_Sprite.h
*        Detail		:
===================================================================================*/
#ifndef OBJECT_SPRITE_H_
#define OBJECT_SPRITE_H_
#include "../../Application.h"

class ObjectSprite
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	ObjectSprite()
		:m_graphics(Application::Instance().Graphics())
	{
		assert(m_graphics);
	}

public:
	void Init();
	void Uninit();
	void Update();
	void Draw();

private:
	ID3D11Buffer*			m_vertexBuffer;
	ID3D11BlendState*		m_blendState;
	ID3D11RasterizerState*	m_rasterizerState;
	ID3D11VertexShader*		m_vertexShader;
	ID3D11InputLayout*		m_inputLayout;
	ID3D11PixelShader*		m_pixelShader;

	GraphicsDX11*		m_graphics;
};

#endif // !OBJECT_SPRITE_H_
