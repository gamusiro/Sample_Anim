/*===================================================================================
*    Date : 2023/06/22(��)
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
	/// ����������
	/// </summary>
	void Init(WindowWin* window);

	/// <summary>
	/// �I������
	/// </summary>
	void Uninit();

	/// <summary>
	/// ��ʃN���A
	/// </summary>
	void Clear();

	/// <summary>
	/// �o�b�t�@�X���b�v
	/// </summary>
	void Present();

	friend class Application;

public:
	/// <summary>
	/// ���_�o�b�t�@�̐���
	/// </summary>
	ID3D11Buffer* CreateVertexBuffer(unsigned int perVertexSize, unsigned int vertexNum, void* data);

	/// <summary>
	/// ���_�o�b�t�@�̐���
	/// </summary>
	ID3D11Buffer* CreateIndexBuffer(unsigned int perIndexSize, unsigned int indexNum, void* data);

	/// <summary>
	/// �u�����h�ݒ���쐬
	/// </summary>
	/// <param name="bDesc"></param>
	/// <returns></returns>
	ID3D11BlendState* CreateBlendState(const D3D11_BLEND_DESC& bDesc);

	/// <summary>
	/// ���X�^���C�U�ݒ���쐬
	/// </summary>
	/// <param name="rDesc"></param>
	/// <returns></returns>
	ID3D11RasterizerState* CreateRasterizerState(const D3D11_RASTERIZER_DESC& rDesc);

	/// <summary>
	/// ���_�V�F�[�_�̐���
	/// </summary>
	ID3D11VertexShader* CreateVertexShader(const std::string& source);
	
	/// <summary>
	/// ���̓��C�A�E�g�̐���
	/// </summary>
	/// <param name="buffer"></param>
	/// <param name="inputLayout"></param>
	/// <returns></returns>
	ID3D11InputLayout* CreateInputLayout(const std::string& source, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayout);
	
	/// <summary>
	/// �s�N�Z���V�F�[�_�̐���
	/// </summary>
	ID3D11PixelShader* CreatePixelShader(const std::string& source);

	/// <summary>
	/// �V�F�[�_���\�[�X�̐���(16����)
	/// </summary>
	ID3D11Buffer* CreateShaderResource(unsigned int size, unsigned int stride = sizeof(float));

	/// <summary>
	/// �T���v���[�̐���
	/// </summary>
	ID3D11SamplerState* CreateSamplerState(const D3D11_SAMPLER_DESC& sDesc);

	/// <summary>
	/// �e�N�X�`���̐���
	/// </summary>
	ID3D11ShaderResourceView* CreateTextureFromBuffer(const unsigned char* buffer, unsigned int width, unsigned int height);

	/// <summary>
	/// �e�N�X�`���̐���
	/// </summary>
	ID3D11ShaderResourceView* CreateTextureFromFile(const char* fileName);

	/// <summary>
	///	���_�o�b�t�@�̐ݒ�
	/// </summary>
	void SetVertexBuffer(ID3D11Buffer* buffer, unsigned int stride, unsigned int offset = 0);

	/// <summary>
	/// 
	/// </summary>
	void SetIndexBuffer(ID3D11Buffer* buffer, DXGI_FORMAT format, unsigned int offset = 0);

	/// <summary>
	/// �C���f�b�N�X�o�b�t�@�̐ݒ�
	/// </summary>
	void SetBlendState(ID3D11BlendState* blendState);

	/// <summary>
	/// ���X�^���C�U�X�e�[�g�̐ݒ�
	/// </summary>
	void SetRasterizerState(ID3D11RasterizerState* rasterizerState);

	/// <summary>
	/// ���̓��C�A�E�g�̐ݒ�
	/// </summary>
	void SetInputLayout(ID3D11InputLayout* inputLayout);

	/// <summary>
	/// ���_�V�F�[�_�̐ݒ�
	/// </summary>
	void SetVertexShader(ID3D11VertexShader* shader);

	/// <summary>
	/// �s�N�Z���V�F�[�_�̐ݒ�
	/// </summary>
	void SetPixelShader(ID3D11PixelShader* shader);

	/// <summary>
	/// �v���~�e�B�u�g�|���W�[�̐ݒ�
	/// </summary>
	void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology);

	/// <summary>
	/// �萔�o�b�t�@�̐ݒ�
	/// </summary>
	void SetShaderResource(ID3D11Buffer* resource, unsigned int slot, unsigned short visibility);

	/// <summary>
	/// �e�N�X�`���̐ݒ�
	/// </summary>
	void SetTexture(ID3D11ShaderResourceView* resourceView, unsigned int slot, unsigned short visibility);

	/// <summary>
	/// �T���v���[�̐ݒ�
	/// </summary>
	void SetSamplerState(ID3D11SamplerState* samplerState, unsigned int slot, unsigned short visibility);

	/// <summary>
	/// �}�b�v
	/// </summary>
	void Map(ID3D11Buffer* buffer, void* data, size_t size);

	/// <summary>
	/// �`��R�[��
	/// </summary>
	void Draw(unsigned int vertexNum, unsigned int startVertexLocation);

	/// <summary>
	/// �`��R�[��
	/// </summary>
	void DrawIndexed(
		unsigned int indexNum,
		unsigned int instanceNum,
		unsigned int firstIndex = 0,
		unsigned int vertexoffset = 0,
		unsigned int firstInstance = 0
	);

	/// <summary>
	/// ���f���s��̌v�Z
	/// </summary>
	glm::mat4 CalculateModelMatrix(
		const glm::vec3 position,
		const glm::vec3 rotation,
		const glm::vec3 scale
	);

	/// <summary>
	/// ���f���s��̌v�Z
	/// </summary>
	glm::mat4 CalculateModelMatrix(
		const glm::vec3 position,
		const glm::quat rotation,
		const glm::vec3 scale
	);

	/// <summary>
	/// �J�����̐ݒ�
	/// </summary>
	void SetCamera(CAMERA cam) { m_mainCamera = cam; }

	/// <summary>
	/// �J�����̐ݒ�
	/// </summary>
	CAMERA GetCamera() { return m_mainCamera; }

	/// <summary>
	/// �J�����s��̎擾
	/// </summary>
	glm::mat4 GetCameraMatrix();

	/// <summary>
	/// �s�񂩂琳�ʕ��������߂�
	/// </summary>
	glm::vec3 GetForward(const glm::mat4& mat);

	/// <summary>
	/// �s�񂩂�E���������߂�
	/// </summary>
	glm::vec3 GetRight(const glm::mat4& mat);

	/// <summary>
	/// 2D�ˉe�s��̎擾
	/// </summary>
	glm::mat4 GetProjection2DMatrix();

	/// <summary>
	/// 3D�ˉe�s��̎擾
	/// </summary>
	glm::mat4 GetProjection3DMatrix();

private:
	/// <summary>
	/// �f�o�C�X�ƃX���b�v�`�F�[���̍쐬����
	/// </summary>
	/// <param name="hWnd">hwnd</param>
	/// <returns>true : ����I��</returns>
	bool CreateDeviceAndSwapChain(WindowWin* window);

	/// <summary>
	/// �����_�[�^�[�Q�b�g�̐�������
	/// </summary>
	/// <returns>true : ����I��</returns>
	bool CreateRenderTarget();

	/// <summary>
	/// �[�x�o�b�t�@
	/// </summary>
	/// <returns>true : ����I��</returns>
	bool CreateDepthBuffer();

	/// <summary>
	/// �r���[�|�[�g�ݒ�
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

