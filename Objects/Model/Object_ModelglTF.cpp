/*===================================================================================
*    Date : 2023/06/23(金)
*        Author		: Gakuto.S
*        File		: Object_Model_glTF.h
*        Detail		:
===================================================================================*/
#define _CRT_SECURE_NO_WARNINGS
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION 
#define STB_IMAGE_WRITE_IMPLEMENTATION 
#include "Object_ModelglTF.h"

/// <summary>
/// ローカル行列の計算
/// </summary>
glm::mat4 glTF::Fruit::LocalMatrix()
{
	glm::mat4 trl, rot, scl;
    trl = glm::translate(glm::mat4(1.0f), this->translation);
    rot = glm::mat4(this->rotation);
    scl = glm::scale(glm::mat4(1.0f), this->scale);

    return glm::transpose(trl * rot * scl);
}

/// <summary>
/// ノードのマトリックスを取得
/// </summary>
glm::mat4 glTF::Fruit::GetMatrix()
{
	glm::mat4 m = this->LocalMatrix();
	glTF::Fruit* p = parent;
	while (p)
	{
		m = m * p->LocalMatrix();
		p = p->parent;
	}
	return m;
}

/// <summary>
/// ノードの更新処理
/// </summary>
void glTF::Fruit::Update()
{
	if (mesh)
	{
		glm::mat4 m = this->GetMatrix();
		mesh->uniformBlock.matrix = m;
		if (skin)
		{
			// ジョイントマトリックスの更新
			glm::mat4 inverseTransform = glm::inverse(m);
			size_t numJoints = std::min((unsigned int)skin->joints.size(), (unsigned int)JOINT_MATRIX_MAX);
			for (size_t i = 0; i < numJoints; i++)
			{
				glTF::Fruit* fruit	= skin->joints[i];
				glm::mat4 jointMat	= glm::transpose(skin->inverseBindMatrices[i]) * fruit->GetMatrix();
				jointMat			= jointMat * inverseTransform;
				mesh->uniformBlock.jointMatrix[i] = jointMat;
			}
			mesh->uniformBlock.jointcount = (float)numJoints;
		}
	}

	for (auto& child : children) {
		child->Update();
	}
}

void ModelglTF::Init()
{
	// モデル読み込み
	tinygltf::Model gltfModel{};
	this->LoadFile(gltfModel);

	// デフォルトシーンが無かったら
	assert(gltfModel.defaultScene > -1);
	const tinygltf::Scene& targetScene = gltfModel.scenes[gltfModel.defaultScene];

	// テクスチャサンプラーを調べる
	this->LoadSamplers(gltfModel);

	// テクスチャを調べる
	this->LoadTextures(gltfModel);

	// マテリアルデータを調べる
	this->LoadMaterials(gltfModel);

	// 頂点数とインデックス数を調べる
	size_t vertexCount	= 0;
	size_t indexCount	= 0;
	for (auto& nodeIndex : targetScene.nodes)
		this->GetVerticesAndIndicesOfFruit(gltfModel.nodes[nodeIndex], gltfModel, vertexCount, indexCount);

	// 頂点とインデックスデータの領域を作成する
	LoaderInfomation infomation{};
	infomation.vertices = new glTF::Vertex3D[vertexCount];
	infomation.indices	= new unsigned int[indexCount];		// インデックスは 4byteで統一する

	// デフォルトのシーンからフルーツを生成する
	for (size_t i = 0; i < targetScene.nodes.size(); ++i)
	{
		const int nodeIndex = targetScene.nodes[i];
		const tinygltf::Node& node = gltfModel.nodes[nodeIndex];
		this->LoadFruit(nullptr, gltfModel, node, nodeIndex, infomation);
	}

	// アニメーションの取得
	if (!gltfModel.animations.empty())
		this->LoadAnimations(gltfModel);

	// スキンの取得
	this->LoadSkins(gltfModel);

	for (auto fruit : m_linearFruits)
	{
		// スキンのアサイン
		if (fruit->skinIndex > -1)
			fruit->skin = m_skins[fruit->skinIndex];

		// 初期ポーズ
		if (fruit->mesh)
			fruit->Update();
	}

	// 描画用のオブジェクト等の生成
	m_vertexBuffer	= m_graphics->CreateVertexBuffer(sizeof(glTF::Vertex3D), unsigned int(vertexCount), infomation.vertices);
	m_indexBuffer	= m_graphics->CreateIndexBuffer(sizeof(unsigned int), unsigned int(indexCount), infomation.indices);

	// 定数バッファの生成
	m_wolrdMatrix		= m_graphics->CreateShaderResource(sizeof(MATRICES));
	m_jointsMatrix		= m_graphics->CreateShaderResource(sizeof(glTF::FruitMesh::UniformBlock));
	m_materialBuffer	= m_graphics->CreateShaderResource(sizeof(glTF::FruitMaterial));

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
	rDesc.CullMode			= D3D11_CULL_MODE::D3D11_CULL_BACK;
	rDesc.FillMode			= D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rDesc.DepthClipEnable	= true;
	m_cullBack = m_graphics->CreateRasterizerState(rDesc);

	rDesc.CullMode			= D3D11_CULL_MODE::D3D11_CULL_NONE;
	m_cullNone				= m_graphics->CreateRasterizerState(rDesc);

	// コンパイル済みシェーダの読み込み
	common::FileIO vsResource("ModelVS.cso");
	common::FileIO psResource("ModelPS.cso");

	m_vertexShader	= m_graphics->CreateVertexShader(vsResource.GetInfo());
	m_pixelShader	= m_graphics->CreatePixelShader(psResource.GetInfo());

	// 入力レイアウトの生成
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout
	{
		{"POSITION",	0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,	0,  0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",		0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,		0, 24, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	1, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,		0, 32, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"JOINT",		0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 40, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"WEIGHT",		0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 56, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",		0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 72, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	m_inputLayout = m_graphics->CreateInputLayout(vsResource.GetInfo(), inputLayout);

	// 領域を破棄する
	delete[] infomation.vertices;
	delete[] infomation.indices;
}

void ModelglTF::Uninit()
{
	// スキンの破棄
	for (auto skin : m_skins)
		delete skin;

	// ノードの破棄
	for (auto fruit : m_fruits)
		delete fruit;

	// テクスチャの破棄
	for (auto& texture : m_textures)
		SAFE_RELEASE(texture.second);

	// サンプラーの破棄
	for (auto& sampler : m_samplers)
		SAFE_RELEASE(sampler);

	SAFE_RELEASE(m_inputLayout);
	SAFE_RELEASE(m_pixelShader);
	SAFE_RELEASE(m_vertexShader);
	SAFE_RELEASE(m_blendState);
	SAFE_RELEASE(m_cullBack);
	SAFE_RELEASE(m_cullNone);
	SAFE_RELEASE(m_materialBuffer);
	SAFE_RELEASE(m_jointsMatrix);
	SAFE_RELEASE(m_wolrdMatrix);
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_vertexBuffer);
}

void ModelglTF::Update()
{
	// アニメーションのタイマー更新
	if (m_animations.size() > 0)
	{
		m_animationTimer += Application::Instance().GetDeltaTime();
		if (m_animationTimer > m_animations[m_animationIndex].end)
			m_animationTimer -= m_animations[m_animationIndex].end;
	}

	this->UpdateAnimation();
}

void ModelglTF::Draw()
{
	/// <summary>
	/// パイプラインの設定
	/// </summary>
	m_graphics->SetInputLayout(m_inputLayout);
	m_graphics->SetVertexShader(m_vertexShader);
	m_graphics->SetPixelShader(m_pixelShader);
	m_graphics->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	

	// 行列計算(更新)
	MATRICES mat{};
	mat.model		= m_graphics->CalculateModelMatrix(m_position, m_rotate, m_scale);
	mat.view		= m_graphics->GetCameraMatrix();
	mat.projection	= m_graphics->GetProjection3DMatrix();
	mat.camPos		= glm::vec4(m_graphics->GetCamera().position, 0.0f);

	m_graphics->Map(m_wolrdMatrix, &mat, sizeof(MATRICES));
	m_graphics->SetShaderResource(m_wolrdMatrix, 0, SET_VS | SET_PS);

	// 頂点バッファの設定
	m_graphics->SetVertexBuffer(m_vertexBuffer, sizeof(glTF::Vertex3D));
	
	// インデックスバッファの設定
	if (m_indexBuffer)
		m_graphics->SetIndexBuffer(m_indexBuffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT);

	// 子ノードの描画(不透明)
	m_graphics->SetBlendState(m_blendState);
	m_graphics->SetRasterizerState(m_cullBack);
	for (auto& fruit : m_fruits)
		this->DrawFruit(fruit, glTF::ALPHA_MODE::ALPHA_MODE_OPAQUE);

	// 子ノードの描画(マスク)
	m_graphics->SetBlendState(m_blendState);
	m_graphics->SetRasterizerState(m_cullNone);
	for (auto& fruit : m_fruits)
		this->DrawFruit(fruit, glTF::ALPHA_MODE::ALPHA_MODE_MASK);

	// 子ノードの描画(半透明)
	m_graphics->SetBlendState(m_blendState);
	m_graphics->SetRasterizerState(m_cullNone);
	for (auto& fruit : m_fruits)
		this->DrawFruit(fruit, glTF::ALPHA_MODE::ALPHA_MODE_BLEND);
}

/// <summary>
/// モデルファイルの読み込み
/// </summary>
void ModelglTF::LoadFile(tinygltf::Model& model)
{
	tinygltf::TinyGLTF gltfContext;

	std::string error;
	std::string warning;

	// 拡張子からファイルフォーマットを調べる
	bool binary = false;
	size_t extpos = k_fileName.rfind('.', k_fileName.length());
	if (extpos != std::string::npos)
		binary = (k_fileName.substr(extpos + 1, k_fileName.length() - extpos) == "glb");

	// ファイルを読み込む
	bool loaded = false;
	if (binary)
		loaded = gltfContext.LoadBinaryFromFile(&model, &error, &warning, k_fileName.c_str());
	else
		loaded = gltfContext.LoadASCIIFromFile(&model, &error, &warning, k_fileName.c_str());
}

/// <summary>
/// サンプラーの読み込み
/// </summary>
void ModelglTF::LoadSamplers(const tinygltf::Model& model)
{
	for (const tinygltf::Sampler& smpl : model.samplers)
	{
		ID3D11SamplerState* sampler{};
		D3D11_SAMPLER_DESC desc{};
		desc.Filter		= D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU	= D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV	= D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW	= D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;

		sampler = m_graphics->CreateSamplerState(desc);
		m_samplers.push_back(sampler);
	}
}

/// <summary>
/// テクスチャの読み込み
/// </summary>
void ModelglTF::LoadTextures(const tinygltf::Model& model)
{
	for (const tinygltf::Texture& tex : model.textures)
	{
		const tinygltf::Image& image = model.images[tex.source];
		ID3D11SamplerState* textureSampler = nullptr;

		int samplerIndex = tex.sampler;
		if (tex.sampler == -1) 
		{
			// サンプラーがなかった場合
			D3D11_SAMPLER_DESC desc{};
			desc.Filter		= D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			desc.AddressU	= D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressV	= D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressW	= D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
			textureSampler	= m_graphics->CreateSamplerState(desc);
			samplerIndex	= int(m_samplers.size());
			m_samplers.push_back(textureSampler);
		}

		// テクスチャの生成
		ID3D11ShaderResourceView* texture;
		texture = m_graphics->CreateTextureFromBuffer(image.image.data(), image.width, image.height);
		m_textures.push_back({ tex.sampler, texture });
	}
}

/// <summary>
/// マテリアルの読み込み
/// </summary>
void ModelglTF::LoadMaterials(tinygltf::Model& model)
{
	for (tinygltf::Material& mat : model.materials)
	{
		glTF::FruitMaterial material{};
		material.doubleSided = mat.doubleSided;
		if (mat.values.find("baseColorTexture") != mat.values.end())
		{
			material.baseColorTextureIndex	= mat.values["baseColorTexture"].TextureIndex();
			material.baseColorTexCoord		= mat.values["baseColorTexture"].TextureTexCoord();
		}
		if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) 
		{
			material.metallicRoughnessTextureIndex	= mat.values["metallicRoughnessTexture"].TextureIndex();
			material.metallicRoughnessTexCoord		= mat.values["metallicRoughnessTexture"].TextureTexCoord();
		}
		if (mat.values.find("roughnessFactor") != mat.values.end()) 
		{
			material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
		}
		if (mat.values.find("metallicFactor") != mat.values.end()) 
		{
			material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
		}
		if (mat.values.find("baseColorFactor") != mat.values.end()) 
		{
			material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
		}
		if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end())
		{
			material.normalTextureIndex		= mat.additionalValues["normalTexture"].TextureIndex();
			material.normalTexCoord			= mat.additionalValues["normalTexture"].TextureTexCoord();
		}
		if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) 
		{
			material.emissiveTextureIndex	= mat.additionalValues["emissiveTexture"].TextureIndex();
			material.emissiveTexCoord		= mat.additionalValues["emissiveTexture"].TextureTexCoord();
		}
		if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end())
		{
			material.occlusionTextureIndex	= mat.additionalValues["occlusionTexture"].TextureIndex();
			material.occlusionTexCoord		= mat.additionalValues["occlusionTexture"].TextureTexCoord();
		}
		if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) 
		{
			tinygltf::Parameter param = mat.additionalValues["alphaMode"];
			if (param.string_value == "BLEND") 
			{
				material.alphaMode = glTF::ALPHA_MODE::ALPHA_MODE_BLEND;
			}
			if (param.string_value == "MASK") 
			{
				material.alphaCutoff = 0.5f;
				material.alphaMode = glTF::ALPHA_MODE::ALPHA_MODE_MASK;
			}
		}
		if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end())
		{
			material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
		}
		if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) 
		{
			material.emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
		}

		// ※いったん無視
		// Extensions
		// @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
		glTF::FruitExtensionMaterial materialEX{};
		if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) 
		{
			auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
			if (ext->second.Has("specularGlossinessTexture")) 
			{
				auto index								= ext->second.Get("specularGlossinessTexture").Get("index");
				materialEX.specularGlossinessTextureIndex = index.Get<int>();
				auto texCoordSet						= ext->second.Get("specularGlossinessTexture").Get("texCoord");
				materialEX.specularGlossinessTexCoord		= texCoordSet.Get<int>();
				materialEX.specularGlosiness				= true;
			}
			if (ext->second.Has("diffuseTexture")) 
			{
				auto index = ext->second.Get("diffuseTexture").Get("index");
				materialEX.diffuseTextureIndex = index.Get<int>();
			}
			if (ext->second.Has("diffuseFactor")) 
			{
				auto factor = ext->second.Get("diffuseFactor");
				for (uint32_t i = 0; i < factor.ArrayLen(); i++)
				{
					auto val = factor.Get(i);
					materialEX.diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
				}
			}
			if (ext->second.Has("specularFactor")) 
			{
				auto factor = ext->second.Get("specularFactor");
				for (uint32_t i = 0; i < factor.ArrayLen(); i++)
				{
					auto val = factor.Get(i);
					materialEX.specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
				}
			}
		}

		m_materials.push_back(material);
	}

	// 初期マテリアルを一番最後に追加しておく
	m_materials.push_back(glTF::FruitMaterial());
}

/// <summary>
/// 頂点数とインデックス数を取得する
/// </summary>
void ModelglTF::GetVerticesAndIndicesOfFruit(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertexCount, size_t& indexCount)
{
	// 子ノードの頂点数とインデックス数もカウントする
	if (node.children.size() > 0)
	{
		for (size_t i = 0; i < node.children.size(); i++)
			this->GetVerticesAndIndicesOfFruit(model.nodes[node.children[i]], model, vertexCount, indexCount);
	}

	// メッシュデータを持つノードであれば
	if (node.mesh > -1)
	{
		const tinygltf::Mesh mesh = model.meshes[node.mesh];
		for (size_t i = 0; i < mesh.primitives.size(); i++)
		{
			// アトリビュートがポジションのカウントを加算していく
			auto primitive = mesh.primitives[i];
			vertexCount += model.accessors[primitive.attributes.find("POSITION")->second].count;

			// インデックスを持っている場合
			if (primitive.indices > -1)
				indexCount += model.accessors[primitive.indices].count;
		}
	}
}

/// <summary>
/// ノードデータを取得する
/// </summary>
void ModelglTF::LoadFruit(glTF::Fruit* parent, const tinygltf::Model& model, const tinygltf::Node& node, unsigned int nodeIndex, LoaderInfomation& infomation)
{
	glTF::Fruit* fruit = new glTF::Fruit();
	fruit->index		= nodeIndex;
	fruit->parent		= parent;
	fruit->name			= node.name;
	fruit->skinIndex	= node.skin;
	fruit->matrix		= glm::mat4(1.0f);

	// ローカルマトリックスデータ
	glm::vec3 translation = glm::vec3(0.0f);
	if (node.translation.size() == 3)
	{// 座標
		translation = glm::make_vec3(node.translation.data());
		fruit->translation = translation;
#ifdef USING_LEFT_HANDED
		fruit->translation.x *= -1.0f;
#endif // USING_LEFT_HANDED
	}

	glm::mat4 rotation = glm::mat4(1.0f);
	if (node.rotation.size() == 4)
	{// 回転(クォータニオン)
		glm::quat q = glm::make_quat(node.rotation.data());
		fruit->rotation = glm::mat4(q);
#ifdef USING_LEFT_HANDED
		fruit->rotation.y *= -1.0f;
		fruit->rotation.z *= -1.0f;
#endif // USING_LEFT_HANDED
	}

	glm::vec3 scale = glm::vec3(1.0f);
	if (node.scale.size() == 3)
	{// スケール
		scale = glm::make_vec3(node.scale.data());
		fruit->scale = scale;
	}

	if (node.matrix.size() == 16)
		fruit->matrix = glm::make_mat4x4(node.matrix.data());

	// 子ノードの探索(再帰処理)
	if (node.children.size() > 0)
	{
		for (size_t i = 0; i < node.children.size(); i++)
		{
			const int childIndex = node.children[i];
			const tinygltf::Node& childNode = model.nodes[childIndex];
			this->LoadFruit(fruit, model, childNode, childIndex, infomation);
		}
	}

	// メッシュデータを持つノードである場合
	if (node.mesh > -1)
	{
		const tinygltf::Mesh& mesh = model.meshes[node.mesh];
		glTF::FruitMesh* fruitMesh = new glTF::FruitMesh();

		for (size_t i = 0; i < mesh.primitives.size(); ++i)
		{
			const tinygltf::Primitive& primitive = mesh.primitives[i];
			unsigned int vertexStart	= static_cast<unsigned int>(infomation.vertexPos);
			unsigned int indexStart		= static_cast<unsigned int>(infomation.indexPos);
			unsigned int vertexCount	= 0;
			unsigned int indexCount		= 0;
			bool hasSkin				= false;
			bool hasIndices				= primitive.indices > -1;

			// Vertices
			{
				BufferVariable<float> pos;	// ポジション
				BufferVariable<float> nor;	// 法線
				BufferVariable<float> uv0;	// uv値0
				BufferVariable<float> uv1;	// uv値1
				BufferVariable<float> col;	// カラー
				BufferVariable<void>  jnt;	// ジョイント
				BufferVariable<float> wgt;	// ウェイト

				int jointComponentType;

				// 頂点データがないのはおかしい
				assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

				// POSITIONデータ
				const tinygltf::Accessor& posAccessor	= model.accessors[primitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView& posView		= model.bufferViews[posAccessor.bufferView];
				pos.buffer		= reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
				pos.byteStride	= posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
				vertexCount		= static_cast<uint32_t>(posAccessor.count);

				// 法線データ
				if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
				{
					const tinygltf::Accessor& normAccessor	= model.accessors[primitive.attributes.find("NORMAL")->second];
					const tinygltf::BufferView& normView	= model.bufferViews[normAccessor.bufferView];
					nor.buffer		= reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
					nor.byteStride	= normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
				}

				// UV値
				if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& uvAccessor	= model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& uvView		= model.bufferViews[uvAccessor.bufferView];
					uv0.buffer		= reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
					uv0.byteStride	= uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
				}
				if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end())
				{
					const tinygltf::Accessor& uvAccessor	= model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
					const tinygltf::BufferView& uvView		= model.bufferViews[uvAccessor.bufferView];
					uv1.buffer		= reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
					uv1.byteStride	= uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
				}

				// 頂点カラー
				if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& accessor	= model.accessors[primitive.attributes.find("COLOR_0")->second];
					const tinygltf::BufferView& view	= model.bufferViews[accessor.bufferView];
					col.buffer		= reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					col.byteStride	= accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
				}

				// スキニング
				// ジョイント
				if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
					const tinygltf::BufferView& jointView	= model.bufferViews[jointAccessor.bufferView];
					jointComponentType	= jointAccessor.componentType;
					jnt.buffer			= &(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
					jnt.byteStride		= jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
				}

				// ウェイト
				if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& weightAccessor	= model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
					const tinygltf::BufferView& weightView		= model.bufferViews[weightAccessor.bufferView];
					wgt.buffer		= reinterpret_cast<const float*>(&(model.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
					wgt.byteStride	= weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
				}

				// スキンデータを持っているか
				hasSkin = (jnt.buffer && wgt.buffer);

				// 描画処理のために頂点を整理していく
				for (size_t v = 0; v < posAccessor.count; ++v)
				{
					glTF::Vertex3D& vertexBuffer = infomation.vertices[infomation.vertexPos];
					vertexBuffer.position		= glm::make_vec3(&pos.buffer[v * pos.byteStride]);
					vertexBuffer.normal			= nor.buffer ? glm::make_vec3(&nor.buffer[v * nor.byteStride]) : glm::vec3(0.0f);
					vertexBuffer.texcoord0		= uv0.buffer ? glm::make_vec2(&uv0.buffer[v * uv0.byteStride]) : glm::vec2(0.0f);
					vertexBuffer.texcoord1		= uv1.buffer ? glm::make_vec2(&uv1.buffer[v * uv1.byteStride]) : glm::vec2(0.0f);
					vertexBuffer.color			= col.buffer ? glm::make_vec4(&col.buffer[v * col.byteStride]) : glm::vec4(1.0f);
#ifdef USING_LEFT_HANDED
					vertexBuffer.position.x *= -1.0f;
					vertexBuffer.normal.x *= -1.0f;
#endif // USING_LEFT_HANDED

					// スキニングデータを持っている場合
					if (hasSkin)
					{
						// ジョイントを表現するバイト数によって変更する
						switch (jointComponentType)
						{
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
						{
							const uint16_t* buf = static_cast<const uint16_t*>(jnt.buffer);
							vertexBuffer.joint0 = glm::vec4(glm::make_vec4(&buf[v * jnt.byteStride]));
							break;
						}
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
						{
							const uint8_t* buf	= static_cast<const uint8_t*>(jnt.buffer);
							vertexBuffer.joint0 = glm::vec4(glm::make_vec4(&buf[v * jnt.byteStride]));
							break;
						}
						default:
							// サポートしていないインデックスフォーマット
							std::cerr << "Joint component type " << jointComponentType << " not supported!" << std::endl;
							break;
						}
					}
					else
					{// データを持たない場合は0.0f
						vertexBuffer.joint0 = glm::vec4(0.0f);
					}

					// 重み係数
					vertexBuffer.weight0 = hasSkin ? glm::make_vec4(&wgt.buffer[v * wgt.byteStride]) : glm::vec4(0.0f);

					// 重みがすべて0である場合の処理(何を意味してるか分からない)
					if (glm::length(vertexBuffer.weight0) == 0.0f)
						vertexBuffer.weight0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

					// 頂点ポジションのインクリメント
					infomation.vertexPos++;
				}
#ifdef USING_LEFT_HANDED
				if (!hasIndices)
				{
					size_t index = infomation.vertexPos - posAccessor.count;
					for (size_t v = 0; v < posAccessor.count / 3; ++v)
					{
						glTF::Vertex3D fir = infomation.vertices[index + v * 3 + 0];
						infomation.vertices[index + v * 3 + 0] = infomation.vertices[index + v * 3 + 2];
						infomation.vertices[index + v * 3 + 2] = fir;
					}
				}
#endif // USING_LEFT_HANDED
			}

			// インデックス
			if (hasIndices)
			{
				const tinygltf::Accessor& accessor		= model.accessors[primitive.indices > -1 ? primitive.indices : 0];
				const tinygltf::BufferView& bufferView	= model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer			= model.buffers[bufferView.buffer];

				indexCount = static_cast<uint32_t>(accessor.count);
				const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

				switch (accessor.componentType)
				{
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
				{
					const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						infomation.indices[infomation.indexPos] = buf[index] + vertexStart;
						infomation.indexPos++;
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
				{
					const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						infomation.indices[infomation.indexPos] = buf[index] + vertexStart;
						infomation.indexPos++;
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
				{
					const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						infomation.indices[infomation.indexPos] = buf[index] + vertexStart;
						infomation.indexPos++;
					}
					break;
				}
				default:
					std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
					return;
				}

#ifdef USING_LEFT_HANDED
				// インデックスの入れ替え
				for (size_t i = 0; i < accessor.count / 3; i++)
				{
					size_t index = (infomation.indexPos - accessor.count) + (i * 3);
					auto work = infomation.indices[index + 2];
					infomation.indices[index + 2] = infomation.indices[index + 0];
					infomation.indices[index + 0] = work;
				}
#endif // USING_LEFT_HANDED
			}

			// メッシュプリミティブの生成
			glTF::FruitPrimitive* meshPrimtive = new glTF::FruitPrimitive(
				indexStart, indexCount, vertexCount, primitive.material > -1 ? m_materials[primitive.material] : m_materials.back());
			fruitMesh->primitives.push_back(meshPrimtive);
		}
		// ノードのメッシュデータとして登録する
		fruit->mesh = fruitMesh;
	}

	// 親子関係
	if (parent)
		parent->children.push_back(fruit);
	else
		m_fruits.push_back(fruit);

	// 同ノード内のノード
	m_linearFruits.push_back(fruit);
}

/// <summary>
/// アニメーションデータを取得する
/// </summary>
void ModelglTF::LoadAnimations(const tinygltf::Model& model)
{
	for (const tinygltf::Animation& anim : model.animations)
	{
		glTF::FruitAnimation animation{};

		// アニメーションの名前を取得(ない場合は数値で管理)
		animation.name = anim.name;
		if (anim.name.empty())
			animation.name = std::to_string(m_animations.size());

		// アニメーションサンプラーの取得
		for (auto& samp : anim.samplers)
		{
			glTF::FruitAnimationSampler sampler{};

			if (samp.interpolation == "LINEAR")
				sampler.interpolation = glTF::INTERPOLATION_TYPE::INTERPOLATION_TYPE_LINEAR;

			if (samp.interpolation == "STEP")
				sampler.interpolation = glTF::INTERPOLATION_TYPE::INTERPOLATION_TYPE_STEP;

			if (samp.interpolation == "CUBICSPLINE")
				sampler.interpolation = glTF::INTERPOLATION_TYPE::INTERPOLATION_TYPE_CUBICSPLINE;

			// 入力データ取得
			{
				const tinygltf::Accessor& accessor		= model.accessors[samp.input];
				const tinygltf::BufferView& bufferView	= model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer			= model.buffers[bufferView.buffer];

				assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				const float* buf = static_cast<const float*>(dataPtr);
				for (size_t index = 0; index < accessor.count; index++)
					sampler.inputs.push_back(buf[index]);

				for (auto input : sampler.inputs)
				{
					if (input < animation.start)
						animation.start = input;

					if (input > animation.end)
						animation.end = input;
				}
			}

			// T/R/S の出力値を取得
			{
				const tinygltf::Accessor& accessor		= model.accessors[samp.output];
				const tinygltf::BufferView& bufferView	= model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer			= model.buffers[bufferView.buffer];

				assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

				switch (accessor.type)
				{
				case TINYGLTF_TYPE_VEC3:
				{
					const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
						sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
					break;
				}
				case TINYGLTF_TYPE_VEC4:
				{
					const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
						sampler.outputsVec4.push_back(buf[index]);
					break;
				}
				default:
				{
					std::cout << "unknown type" << std::endl;
					break;
				}
				}
			}

			animation.samplers.push_back(sampler);
		}

		// チャンネル
		for (auto& source : anim.channels)
		{
			glTF::FruitAnimationChannel channel{};

			if (source.target_path == "rotation")
				channel.path = glTF::PATH_TYPE::PATH_TYPE_ROTATION;

			if (source.target_path == "translation")
				channel.path = glTF::PATH_TYPE::PATH_TYPE_TRANSLATION;

			if (source.target_path == "scale")
				channel.path = glTF::PATH_TYPE::PATH_TYPE_SCALE;

			if (source.target_path == "weights") {
				std::cout << "weights not yet supported, skipping channel" << std::endl;
				continue;
			}
			channel.samplerIndex = source.sampler;
			channel.node = FruitFromIndex(source.target_node);
			if (!channel.node)
				continue;

			animation.channels.push_back(channel);
		}

		m_animations.push_back(animation);
	}
}

/// <summary>
/// スキン情報の取得
/// </summary>
void ModelglTF::LoadSkins(const tinygltf::Model& model)
{
	for (const tinygltf::Skin& source : model.skins)
	{
		glTF::FruitSkin* fruitSkin = new glTF::FruitSkin();
		fruitSkin->name = source.name;

		// ルートノードを見つける
		if (source.skeleton > -1)
			fruitSkin->skeletonRoot = this->FruitFromIndex(source.skeleton);


		// ジョイントノードを見つける
		for (int jointIndex : source.joints)
		{
			glTF::Fruit* fruit = this->FruitFromIndex(jointIndex);
			if (fruit)
				fruitSkin->joints.push_back(this->FruitFromIndex(jointIndex));
		}

		// 逆行列データを読み込む
		if (source.inverseBindMatrices > -1)
		{
			const tinygltf::Accessor& accessor		= model.accessors[source.inverseBindMatrices];
			const tinygltf::BufferView& bufferView	= model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer			= model.buffers[bufferView.buffer];
			fruitSkin->inverseBindMatrices.resize(accessor.count);
			memcpy(fruitSkin->inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));

#ifdef USING_LEFT_HANDED
			for (size_t i = 0; i < accessor.count; ++i)
			{
				fruitSkin->inverseBindMatrices[i][0].y *= -1.0f;
				fruitSkin->inverseBindMatrices[i][0].z *= -1.0f;
				fruitSkin->inverseBindMatrices[i][1].x *= -1.0f;
				fruitSkin->inverseBindMatrices[i][2].x *= -1.0f;
				fruitSkin->inverseBindMatrices[i][3].x *= -1.0f;
			}
#endif // USING_LEFT_HANDED
		}

		m_skins.push_back(fruitSkin);
	}
}

glTF::Fruit* ModelglTF::FindFruit(glTF::Fruit* parent, unsigned int index)
{
	glTF::Fruit* fruitFound = nullptr;
	if (parent->index == index)
		return parent;

	for (auto& child : parent->children)
	{
		fruitFound = FindFruit(child, index);
		if (fruitFound)
			break;
	}
	return fruitFound;
}

glTF::Fruit* ModelglTF::FruitFromIndex(unsigned int index)
{
	glTF::Fruit* fruitFound = nullptr;
	for (auto& node : m_fruits)
	{
		fruitFound = FindFruit(node, index);
		if (fruitFound)
			break;
	}
	return fruitFound;
}

/// <summary>
/// 描画処理
/// </summary>
void ModelglTF::DrawFruit(glTF::Fruit* fruit, glTF::ALPHA_MODE mode)
{
	// 親ノードの描画処理
	if (fruit->mesh)
	{
		// スキンメッシュ行列の書き込み
		glTF::FruitMesh::UniformBlock& block = fruit->mesh->uniformBlock;
		m_graphics->Map(m_jointsMatrix, &block, sizeof(glTF::FruitMesh::UniformBlock));
		m_graphics->SetShaderResource(m_jointsMatrix, 1, SET_VS);

		for (auto& primitive : fruit->mesh->primitives)
		{
			// モードが違う場合
			if (primitive->material.alphaMode != mode)
				continue;

			// マテリアルの書き込み
			m_graphics->Map(m_materialBuffer, &primitive->material, sizeof(glTF::FruitMaterial));
			m_graphics->SetShaderResource(m_materialBuffer, 1, SET_PS);

			// マテリアルデータを転送する
			this->SetMaterialDatas(primitive->material);

			if (primitive->hasIndices)
				m_graphics->DrawIndexed(primitive->indexCount, 1, primitive->firstIndex, 0, 0);
			else
				m_graphics->Draw(primitive->vertexCount, 0);
		}
	}

	// 子ノードの描画処理
	for (auto& child : fruit->children)
		this->DrawFruit(child, mode);
}

/// <summary>
/// アニメーションの更新
/// </summary>
void ModelglTF::UpdateAnimation()
{
	if (m_animations.empty())
	{// アニメーションを持っていない場合
		std::cout << ".glTF does not contain animation." << std::endl;
		return;
	}
	if (m_animationIndex > static_cast<uint32_t>(m_animations.size()) - 1)
	{// アニメーションの種類が選択範囲を逸脱している場合
		std::cout << "No animation with index " << m_animationIndex << std::endl;
		return;
	}

	// 使用するアニメーションを選択
	glTF::FruitAnimation& animation = m_animations[m_animationIndex];

	bool updated = false;
	for (auto& channel : animation.channels)
	{
		glTF::FruitAnimationSampler& sampler = animation.samplers[channel.samplerIndex];
		if (sampler.inputs.size() > sampler.outputsVec4.size())	// weight
			continue;

		for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
		{
			if ((m_animationTimer >= sampler.inputs[i]) && (m_animationTimer <= sampler.inputs[i + 1]))
			{
				float u = std::max(0.0f, m_animationTimer - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
				if (u <= 1.0f)
				{
					switch (channel.path)
					{
					case glTF::PATH_TYPE::PATH_TYPE_TRANSLATION:
					{
						glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
						channel.node->translation = glm::vec3(trans);
#ifdef USING_LEFT_HANDED
						channel.node->translation.x *= -1.0f;
#endif // USING_LEFT_HANDED
						break;
					}
					case glTF::PATH_TYPE::PATH_TYPE_SCALE:
					{
						glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
						channel.node->scale = glm::vec3(trans);
						break;
					}
					case glTF::PATH_TYPE::PATH_TYPE_ROTATION:
					{
#ifdef USING_LEFT_HANDED
						glm::quat q1;
						q1.x =  sampler.outputsVec4[i].x;
						q1.y = -sampler.outputsVec4[i].y;
						q1.z = -sampler.outputsVec4[i].z;
						q1.w =  sampler.outputsVec4[i].w;
						glm::quat q2;
						q2.x =  sampler.outputsVec4[i + 1].x;
						q2.y = -sampler.outputsVec4[i + 1].y;
						q2.z = -sampler.outputsVec4[i + 1].z;
						q2.w =  sampler.outputsVec4[i + 1].w;
						channel.node->rotation = glm::normalize(glm::slerp(q1, q2, u));
#else
						glm::quat q1;
						q1.x = sampler.outputsVec4[i].x;
						q1.y = sampler.outputsVec4[i].y;
						q1.z = sampler.outputsVec4[i].z;
						q1.w = sampler.outputsVec4[i].w;
						glm::quat q2;
						q2.x = sampler.outputsVec4[i + 1].x;
						q2.y = sampler.outputsVec4[i + 1].y;
						q2.z = sampler.outputsVec4[i + 1].z;
						q2.w = sampler.outputsVec4[i + 1].w;
						channel.node->rotation = glm::normalize(glm::slerp(q1, q2, u));
#endif // USING_LEFT_HANDED
						break;
					}
					}
					updated = true;
				}
			}
		}
	}
	if (updated)
	{
		for (auto& fruit : m_fruits)
			fruit->Update();
	}
}

/// <summary>
/// マテリアルデータの設定
/// </summary>
void ModelglTF::SetMaterialDatas(glTF::FruitMaterial& material)
{
	if (material.baseColorTextureIndex > -1)
	{// ベースカラーテクスチャを転送
		int samplerIndex = m_textures[material.baseColorTextureIndex].first;
		ID3D11ShaderResourceView* resourceView = m_textures[material.baseColorTextureIndex].second;
		m_graphics->SetTexture(resourceView, 0, SET_PS);
		m_graphics->SetSamplerState(m_samplers[samplerIndex], 0, SET_PS);
	}

	if (material.metallicRoughnessTextureIndex > -1)
	{// メタリックラフネステクスチャを転送
		int samplerIndex = m_textures[material.metallicRoughnessTextureIndex].first;
		ID3D11ShaderResourceView* resourceView = m_textures[material.metallicRoughnessTextureIndex].second;
		m_graphics->SetTexture(resourceView, 1, SET_PS);
		m_graphics->SetSamplerState(m_samplers[samplerIndex], 1, SET_PS);
	}

	if (material.normalTextureIndex > -1)
	{// 法線テクスチャを転送
		int samplerIndex = m_textures[material.normalTextureIndex].first;
		ID3D11ShaderResourceView* resourceView = m_textures[material.normalTextureIndex].second;
		m_graphics->SetTexture(resourceView, 2, SET_PS);
		m_graphics->SetSamplerState(m_samplers[samplerIndex], 2, SET_PS);
	}

	if (material.occlusionTextureIndex > -1)
	{// オクルージョンテクスチャを転送
		int samplerIndex = m_textures[material.occlusionTextureIndex].first;
		ID3D11ShaderResourceView* resourceView = m_textures[material.occlusionTextureIndex].second;
		m_graphics->SetTexture(resourceView, 3, SET_PS);
		m_graphics->SetSamplerState(m_samplers[samplerIndex], 3, SET_PS);
	}

	if (material.emissiveTextureIndex > -1)
	{// エミッシブテクスチャを転送
		int samplerIndex = m_textures[material.emissiveTextureIndex].first;
		ID3D11ShaderResourceView* resourceView = m_textures[material.emissiveTextureIndex].second;
		m_graphics->SetTexture(resourceView, 4, SET_PS);
		m_graphics->SetSamplerState(m_samplers[samplerIndex], 4, SET_PS);
	}
}
