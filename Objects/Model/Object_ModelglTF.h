/*===================================================================================
*    Date : 2023/06/23(��)
*        Author		: Gakuto.S
*        File		: Object_Model_glTF.h
*        Detail		:
===================================================================================*/
#ifndef OBJECT_MODEL_GLTF_H_
#define OBJECT_MODEL_GLTF_H_
#include <tiny_gltf.h>

#include "../../Application.h"

// DirectX�p
#ifdef _DIRECTX
#define USING_LEFT_HANDED
#endif // _WIN32

#define JOINT_MATRIX_MAX 128

namespace glTF
{
	struct Fruit;

	/// <summary>
	/// �u�����h���[�h�ݒ�
	/// </summary>
	enum class ALPHA_MODE
	{
		ALPHA_MODE_OPAQUE,
		ALPHA_MODE_MASK,
		ALPHA_MODE_BLEND,
	};

	/// <summary>
	/// �A�j���[�V�����p�X
	/// </summary>
	enum class PATH_TYPE 
	{ 
		PATH_TYPE_TRANSLATION,
		PATH_TYPE_ROTATION,
		PATH_TYPE_SCALE,
		PATH_TYPE_WEIGHT
	};

	/// <summary>
	/// �A�j���[�V�����̕�ԃ��[�h
	/// </summary>
	enum class INTERPOLATION_TYPE 
	{ 
		INTERPOLATION_TYPE_LINEAR,
		INTERPOLATION_TYPE_STEP,
		INTERPOLATION_TYPE_CUBICSPLINE
	};

	/// <summary>
	/// �}�e���A���̊g�����
	/// </summary>
	struct FruitExtensionMaterial
	{
		// �g�����
		int specularGlossinessTextureIndex	= -1;
		int diffuseTextureIndex				= -1;
		glm::vec4 diffuseFactor				= glm::vec4(1.0f);
		glm::vec3 specularFactor			= glm::vec3(0.0f);
		int specularGlossinessTexCoord		= -1;
		int metalicRoughness				= true;
		int specularGlosiness				= false;
	};

	/// <summary>
	/// ���f���}�e���A�����
	/// </summary>
	struct FruitMaterial
	{
		ALPHA_MODE alphaMode						= ALPHA_MODE::ALPHA_MODE_OPAQUE;
		float alphaCutoff							= 1.0f;
		float metallicFactor						= 1.0f;
		float roughnessFactor						= 1.0f;
		glm::vec4 baseColorFactor					= glm::vec4(1.0f);
		glm::vec4 emissiveFactor					= glm::vec4(1.0f);
		int baseColorTextureIndex					= -1;
		int metallicRoughnessTextureIndex			= -1;
		int normalTextureIndex						= -1;
		int occlusionTextureIndex					= -1;
		int emissiveTextureIndex					= -1;
		int doubleSided								= false;
		int baseColorTexCoord						= -1;
		int metallicRoughnessTexCoord				= -1;
		int normalTexCoord							= -1;
		int occlusionTexCoord						= -1;
		int emissiveTexCoord						= -1;
		float dummy = 1.0f;
	};

	/// <summary>
	/// �v���~�e�B�u�f�[�^
	/// </summary>
	struct FruitPrimitive
	{
		FruitPrimitive(
			unsigned int firstIndex, unsigned int indexCount, unsigned int vertexCount, FruitMaterial& material)
			:firstIndex(firstIndex), indexCount(indexCount), vertexCount(vertexCount), material(material)
		{
			hasIndices = indexCount > 0;
		}

		unsigned int	firstIndex;
		unsigned int	indexCount;
		unsigned int	vertexCount;
		FruitMaterial&	material;
		bool			hasIndices;
	};

	/// <summary>
	/// ���b�V�����
	/// </summary>
	struct FruitMesh
	{
		std::vector<FruitPrimitive*> primitives;

		struct UniformBlock
		{
			glm::mat4 matrix;
			glm::mat4 jointMatrix[JOINT_MATRIX_MAX]{};
			float jointcount = 0;
			float dummy[3]{};
		}uniformBlock;

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~FruitMesh()
		{
			for (FruitPrimitive* p : primitives)
				delete p;
		}
	};

	/// <summary>
	/// �X�L�����
	/// </summary>
	struct FruitSkin
	{
		std::string				name;
		Fruit*					skeletonRoot;
		std::vector<glm::mat4>	inverseBindMatrices;
		std::vector<Fruit*>		joints;
	};

	/// <summary>
	/// �t���[�c
	/// </summary>
	struct Fruit
	{
		Fruit*				parent;
		unsigned int		index;
		std::vector<Fruit*> children;
		glm::mat4			matrix;
		std::string			name;
		FruitMesh*			mesh;
		FruitSkin*			skin;
		int					skinIndex = -1;
		glm::vec3			translation{};
		glm::quat			rotation{};
		glm::vec3			scale{ 1.0f };

		/// <summary>
		/// ���[�J���}�g���b�N�X�̌v�Z
		/// </summary>
		glm::mat4 LocalMatrix();

		/// <summary>
		/// �m�[�h�̍s��擾
		/// </summary>
		/// <returns></returns>
		glm::mat4 GetMatrix();

		/// <summary>
		/// �f�X�g���N�^
		/// </summary>
		~Fruit()
		{
			// ���b�V���f�[�^�̔j��
			if (mesh)
				delete mesh;

			// �q�m�[�h�f�[�^�̔j��
			for (auto& child : children)
				delete child;
		}

		/// <summary>
		/// �m�[�h�̍X�V
		/// </summary>
		void Update();
	};

	/// <summary>
	/// ���_���
	/// </summary>
	struct Vertex3D
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texcoord0;
		glm::vec2 texcoord1;
		glm::vec4 joint0;
		glm::vec4 weight0;
		glm::vec4 color;
	};

	/// <summary>
	/// �A�j���[�V�����̃`�����l��
	/// </summary>
	struct FruitAnimationChannel
	{
		PATH_TYPE	path;
		Fruit*		node;
		uint32_t	samplerIndex;
	};

	/// <summary>
	/// �A�j���[�V�����̃T���v���[
	/// </summary>
	struct FruitAnimationSampler
	{
		INTERPOLATION_TYPE		interpolation;
		std::vector<float>		inputs;
		std::vector<glm::vec4>	outputsVec4;
	};

	/// <summary>
	/// �A�j���[�V����
	/// </summary>
	struct FruitAnimation
	{
		std::string							name;
		std::vector<FruitAnimationChannel>	channels;
		std::vector<FruitAnimationSampler>	samplers;
		float start = std::numeric_limits<float>::max();
		float end = std::numeric_limits<float>::min();
	};

};


class ModelglTF
{
private:
	/// <summary>
	/// �f�[�^�ǂݍ��ݎ��ɕK�v�ȃf�[�^�i�[�p
	/// </summary>
	struct LoaderInfomation
	{
		unsigned int*	indices;
		glTF::Vertex3D* vertices;
		size_t	indexPos = 0;
		size_t	vertexPos = 0;
	};

	/// <summary>
	/// ��Ɨp�o�b�t�@
	/// </summary>
	template <typename T>
	struct BufferVariable
	{
		const T* buffer = nullptr;
		int	byteStride = 0;
	};


public:
	/// <summary>
	/// �R���X�g���N�^
	/// </summary>
	ModelglTF(const std::string& fileName)
		:m_graphics(Application::Instance().Graphics()),
		k_fileName(fileName)
	{}

public:
	void Init();
	void Uninit();
	void Update();
	void Draw();

private:
	/// <summary>
	/// �t�@�C���̓ǂݍ���
	/// </summary>
	void LoadFile(tinygltf::Model& model);

	/// <summary>
	/// �T���v���[�̎擾
	/// </summary>
	void LoadSamplers(const tinygltf::Model& model);

	/// <summary>
	/// �e�N�X�`���̎擾
	/// </summary>
	void LoadTextures(const tinygltf::Model& model);

	/// <summary>
	/// �}�e���A���̎擾
	/// </summary>
	/// <param name="model"></param>
	void LoadMaterials(tinygltf::Model& model);

	/// <summary>
	/// �m�[�h�̒��_���ƃC���f�b�N�X�����擾����
	/// </summary>
	void GetVerticesAndIndicesOfFruit(
		const tinygltf::Node& node,
		const tinygltf::Model& model,
		size_t& vertexCount,
		size_t& indexCount
	);

	/// <summary>
	/// �t���[�c��ǂݍ���
	/// </summary>
	void LoadFruit(
		glTF::Fruit* parent,
		const tinygltf::Model& model,
		const tinygltf::Node& node,
		unsigned int nodeIndex,
		LoaderInfomation& infomation
	);

	/// <summary>
	/// �A�j���[�V�����̎擾
	/// </summary>
	void LoadAnimations(const tinygltf::Model& model);

	/// <summary>
	/// �X�L�����̎擾
	/// </summary>
	/// <param name="model"></param>
	void LoadSkins(const tinygltf::Model& model);

	/// <summary>
	/// �m�[�h��������
	/// </summary>
	/// <returns></returns>
	glTF::Fruit* FindFruit(glTF::Fruit* parent, unsigned int index);

	/// <summary>
	/// �C���f�b�N�X����m�[�h��������
	/// </summary>
	/// <returns></returns>
	glTF::Fruit* FruitFromIndex(unsigned int index);

	/// <summary>
	/// �`�揈��
	/// </summary>
	void DrawFruit(glTF::Fruit* fruit, glTF::ALPHA_MODE mode);

	/// <summary>
	/// �A�j���[�V�����̍X�V����
	/// </summary>
	void UpdateAnimation();

	/// <summary>
	/// �}�e���A���f�[�^���V�F�[�_�ɐݒ肷��
	/// </summary>
	void SetMaterialDatas(glTF::FruitMaterial& material);


private:
	/// <summary>
	/// �`��ݒ�̕ϐ�(�p�C�v���C��)
	/// </summary>
	ID3D11BlendState*		m_blendState;
	ID3D11RasterizerState*	m_cullBack;
	ID3D11RasterizerState*	m_cullNone;
	ID3D11VertexShader*		m_vertexShader;
	ID3D11InputLayout*		m_inputLayout;
	ID3D11PixelShader*		m_pixelShader;

private:
	/// <summary>
	/// ���f���`��̕ϐ�
	/// </summary>
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	ID3D11Buffer* m_wolrdMatrix;
	ID3D11Buffer* m_jointsMatrix;
	ID3D11Buffer* m_materialBuffer;

public:
	glm::vec3 m_position	= glm::vec3(0.0f);
	glm::vec3 m_rotate		= glm::vec3(0.0f);
	glm::vec3 m_scale		= glm::vec3(1.0f);

private:
	const std::string	k_fileName;

	GraphicsDX11*	m_graphics;
	unsigned int	m_animationIndex;	// �����A�j���[�V�����̎��ʗp
	float			m_animationTimer;	// �A�j���[�V�����̃^�C�}�[

	std::vector<glTF::Fruit*>								m_fruits;
	std::vector<glTF::Fruit*>								m_linearFruits;
	std::vector<glTF::FruitSkin*>							m_skins;	
	std::vector<glTF::FruitAnimation>						m_animations;
	std::vector<glTF::FruitMaterial>						m_materials;
	std::vector<std::pair<int, ID3D11ShaderResourceView*>>	m_textures;
	std::vector<ID3D11SamplerState*>						m_samplers;
};

#endif // !OBJECT_MODEL_GLTF_H_