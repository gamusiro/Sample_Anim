/*===================================================================================
*    Date : 2023/06/22(木)
*        Author		: Gakuto.S
*        File		: Graphics_DX11.h
*        Detail		:
===================================================================================*/
#ifndef GRAPHICS_DX11_H_
#define GRAPHICS_DX11_H_
#include "../../Config.h"

#ifdef _WIN32

#define SET_VS 0x01
#define SET_HS 0x02
#define SET_DS 0x04
#define SET_GS 0x08
#define SET_PS 0x10
#define SET_AL 0x7F

class WindowWin;
class GraphicsDX11
{
private:
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init(WindowWin* window);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Uninit();

	/// <summary>
	/// 画面クリア
	/// </summary>
	void Clear();

	/// <summary>
	/// バッファスワップ
	/// </summary>
	void Present();

	friend class Application;

public:
	/// <summary>
	/// 頂点バッファの生成
	/// </summary>
	ID3D11Buffer* CreateVertexBuffer(unsigned int perVertexSize, unsigned int vertexNum, void* data);

	/// <summary>
	/// 頂点バッファの生成
	/// </summary>
	ID3D11Buffer* CreateIndexBuffer(unsigned int perIndexSize, unsigned int indexNum, void* data);

	/// <summary>
	/// ブレンド設定を作成
	/// </summary>
	/// <param name="bDesc"></param>
	/// <returns></returns>
	ID3D11BlendState* CreateBlendState(const D3D11_BLEND_DESC& bDesc);

	/// <summary>
	/// ラスタライザ設定を作成
	/// </summary>
	/// <param name="rDesc"></param>
	/// <returns></returns>
	ID3D11RasterizerState* CreateRasterizerState(const D3D11_RASTERIZER_DESC& rDesc);

	/// <summary>
	/// 頂点シェーダの生成
	/// </summary>
	ID3D11VertexShader* CreateVertexShader(const std::string& source);
	
	/// <summary>
	/// 入力レイアウトの生成
	/// </summary>
	/// <param name="buffer"></param>
	/// <param name="inputLayout"></param>
	/// <returns></returns>
	ID3D11InputLayout* CreateInputLayout(const std::string& source, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayout);
	
	/// <summary>
	/// ピクセルシェーダの生成
	/// </summary>
	ID3D11PixelShader* CreatePixelShader(const std::string& source);

	/// <summary>
	/// シェーダリソースの生成(16制限)
	/// </summary>
	ID3D11Buffer* CreateShaderResource(unsigned int size, unsigned int stride = sizeof(float));

	/// <summary>
	/// サンプラーの生成
	/// </summary>
	ID3D11SamplerState* CreateSamplerState(const D3D11_SAMPLER_DESC& sDesc);

	/// <summary>
	/// テクスチャの生成
	/// </summary>
	ID3D11ShaderResourceView* CreateTextureFromBuffer(const unsigned char* buffer, unsigned int width, unsigned int height);

	/// <summary>
	/// テクスチャの生成
	/// </summary>
	ID3D11ShaderResourceView* CreateTextureFromFile(const char* fileName);

	/// <summary>
	///	頂点バッファの設定
	/// </summary>
	void SetVertexBuffer(ID3D11Buffer* buffer, unsigned int stride, unsigned int offset = 0);

	/// <summary>
	/// 
	/// </summary>
	void SetIndexBuffer(ID3D11Buffer* buffer, DXGI_FORMAT format, unsigned int offset = 0);

	/// <summary>
	/// インデックスバッファの設定
	/// </summary>
	void SetBlendState(ID3D11BlendState* blendState);

	/// <summary>
	/// ラスタライザステートの設定
	/// </summary>
	void SetRasterizerState(ID3D11RasterizerState* rasterizerState);

	/// <summary>
	/// 入力レイアウトの設定
	/// </summary>
	void SetInputLayout(ID3D11InputLayout* inputLayout);

	/// <summary>
	/// 頂点シェーダの設定
	/// </summary>
	void SetVertexShader(ID3D11VertexShader* shader);

	/// <summary>
	/// ピクセルシェーダの設定
	/// </summary>
	void SetPixelShader(ID3D11PixelShader* shader);

	/// <summary>
	/// プリミティブトポロジーの設定
	/// </summary>
	void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology);

	/// <summary>
	/// 定数バッファの設定
	/// </summary>
	void SetShaderResource(ID3D11Buffer* resource, unsigned int slot, unsigned short visibility);

	/// <summary>
	/// テクスチャの設定
	/// </summary>
	void SetTexture(ID3D11ShaderResourceView* resourceView, unsigned int slot, unsigned short visibility);

	/// <summary>
	/// サンプラーの設定
	/// </summary>
	void SetSamplerState(ID3D11SamplerState* samplerState, unsigned int slot, unsigned short visibility);

	/// <summary>
	/// マップ
	/// </summary>
	void Map(ID3D11Buffer* buffer, void* data, size_t size);

	/// <summary>
	/// 描画コール
	/// </summary>
	void Draw(unsigned int vertexNum, unsigned int startVertexLocation);

	/// <summary>
	/// 描画コール
	/// </summary>
	void DrawIndexed(
		unsigned int indexNum,
		unsigned int instanceNum,
		unsigned int firstIndex = 0,
		unsigned int vertexoffset = 0,
		unsigned int firstInstance = 0
	);

	/// <summary>
	/// モデル行列の計算
	/// </summary>
	glm::mat4 CalculateModelMatrix(
		const glm::vec3 position,
		const glm::vec3 rotation,
		const glm::vec3 scale
	);

	/// <summary>
	/// モデル行列の計算
	/// </summary>
	glm::mat4 CalculateModelMatrix(
		const glm::vec3 position,
		const glm::quat rotation,
		const glm::vec3 scale
	);

	/// <summary>
	/// カメラの設定
	/// </summary>
	void SetCamera(CAMERA cam) { m_mainCamera = cam; }

	/// <summary>
	/// カメラの設定
	/// </summary>
	CAMERA GetCamera() { return m_mainCamera; }

	/// <summary>
	/// カメラ行列の取得
	/// </summary>
	glm::mat4 GetCameraMatrix();

	/// <summary>
	/// 行列から正面方向を求める
	/// </summary>
	glm::vec3 GetForward(const glm::mat4& mat);

	/// <summary>
	/// 行列から右方向を求める
	/// </summary>
	glm::vec3 GetRight(const glm::mat4& mat);

	/// <summary>
	/// 2D射影行列の取得
	/// </summary>
	glm::mat4 GetProjection2DMatrix();

	/// <summary>
	/// 3D射影行列の取得
	/// </summary>
	glm::mat4 GetProjection3DMatrix();

private:
	/// <summary>
	/// デバイスとスワップチェーンの作成処理
	/// </summary>
	/// <param name="hWnd">hwnd</param>
	/// <returns>true : 正常終了</returns>
	bool CreateDeviceAndSwapChain(WindowWin* window);

	/// <summary>
	/// レンダーターゲットの生成処理
	/// </summary>
	/// <returns>true : 正常終了</returns>
	bool CreateRenderTarget();

	/// <summary>
	/// 深度バッファ
	/// </summary>
	/// <returns>true : 正常終了</returns>
	bool CreateDepthBuffer();

	/// <summary>
	/// ビューポート設定
	/// </summary>
	void SetViewport();

private:
	ID3D11Device*				m_device;
	ID3D11DeviceContext*		m_context;
	IDXGISwapChain*				m_swapchain;
	ID3D11RenderTargetView*		m_renderTargetView;
	ID3D11DepthStencilView*		m_depthStencilView;
	ID3D11DepthStencilState*	m_depthStencilState;

	UINT m_width;
	UINT m_height;

private:
	CAMERA m_mainCamera;
};
#endif // _WIN32

#endif // !GRAPHICS_DX11_H_

