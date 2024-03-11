/*===================================================================================
*    Date : 2022/12/04(��)
*        Author		: Gakuto.S
*        File		: Load_Texture.cpp
*        Detail		:
===================================================================================*/
#include "Load_Texture.h"

// �摜�ǂݍ���
#include <stb_image.h>

common::LoadTexture::LoadTexture(const char* fileName)
	:m_image(),
	m_width(),
	m_height()
{
	int			channels{};
	stbi_uc*	metaImage{};
	
	// �t�@�C������摜��ǂݍ���
	metaImage = stbi_load(fileName, &m_width, &m_height, &channels, k_RGBA);

	m_image = (unsigned char*)metaImage;
}

common::LoadTexture::LoadTexture(const unsigned char* buffer, int length)
	:m_image(),
	m_width(),
	m_height()
{
	int			channels{};
	stbi_uc*	metaImage{};

	// ����������摜��ǂݍ���
	metaImage = stbi_load_from_memory(buffer, length, &m_width, &m_height, &channels, 0);
	m_image = (unsigned char*)metaImage;
}

common::LoadTexture::~LoadTexture()
{
	// �ǂݍ��񂾃f�[�^�̉��
	stbi_image_free(m_image);
}
