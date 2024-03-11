/*===================================================================================
*    Date : 2023/06/23(金)
*        Author		: Gakuto.S
*        File		: Object_Sprite.cpp
*        Detail		:
===================================================================================*/
#include "Object_Sprite.h"

struct VERTEX3D
{
	glm::vec3 position;
};

static VERTEX3D g_vertices[]
{
	{{-0.5f, -0.5f,  0.0f}},
	{{ 0.5f, -0.5f,  0.0f}},
	{{-0.5f,  0.5f,  0.0f}},
	{{ 0.5f,  0.5f,  0.0f}},
};

void ObjectSprite::Init()
{
	// 頂点バッファの生成
	m_vertexBuffer = m_graphics->CreateVertexBuffer(sizeof(VERTEX3D), 4, g_vertices);

	// ブレンド設定の生成
	D3D11_BLEND_DESC bDesc{};
	bDesc.RenderTarget[0].BlendEnable			= true;
	bDesc.RenderTarget[0].SrcBlend				= D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
	bDesc.RenderTarget[0].DestBlend				= D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
	bDesc.RenderTarget[0].BlendOp				= D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	bDesc.RenderTarget[0].SrcBlendAlpha			= D3D11_BLEND::D3D11_BLEND_ONE;
	bDesc.RenderTarget[0].DestBlendAlpha		= D3D11_BLEND::D3D11_BLEND_ZERO;
	bDesc.RenderTarget[0].BlendOpAlpha			= D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	bDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;
	m_blendState = m_graphics->CreateBlendState(bDesc);

	// ラスタライザ設定の生成
	D3D11_RASTERIZER_DESC rDesc{};
	rDesc.CullMode			= D3D11_CULL_MODE::D3D11_CULL_NONE;
	rDesc.FillMode			= D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rDesc.DepthClipEnable	= true;
	m_rasterizerState = m_graphics->CreateRasterizerState(rDesc);

	// コンパイル済みシェーダの読み込み
	common::FileIO vsResource("SpriteVS.cso");
	common::FileIO psResource("SpritePS.cso");

	m_vertexShader	= m_graphics->CreateVertexShader(vsResource.GetInfo());
	m_pixelShader	= m_graphics->CreatePixelShader(psResource.GetInfo());

	// 入力レイアウトの生成
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	m_inputLayout = m_graphics->CreateInputLayout(vsResource.GetInfo(), inputLayout);

}

void ObjectSprite::Uninit()
{
	SAFE_RELEASE(m_inputLayout);
	SAFE_RELEASE(m_pixelShader);
	SAFE_RELEASE(m_vertexShader);
	SAFE_RELEASE(m_rasterizerState);
	SAFE_RELEASE(m_blendState);
	SAFE_RELEASE(m_vertexBuffer);
}

void ObjectSprite::Update()
{
}

void ObjectSprite::Draw()
{
	m_graphics->SetBlendState(m_blendState);
	m_graphics->SetRasterizerState(m_rasterizerState);
	m_graphics->SetInputLayout(m_inputLayout);
	m_graphics->SetVertexShader(m_vertexShader);
	m_graphics->SetPixelShader(m_pixelShader);
	m_graphics->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_graphics->SetVertexBuffer(m_vertexBuffer, sizeof(VERTEX3D));
	m_graphics->Draw(4, 0);
}
