/*===================================================================================
*    Date : 2022/11/28(��)
*        Author		: Gakuto.S
*        File		: FileIO.h
*        Detail		:
===================================================================================*/
#ifndef FILEIO_H_
#define FILEIO_H_

#include <vector>
#include <fstream>

namespace common
{
    class FileIO
    {
    public:
        FileIO()        = delete;
        FileIO(FileIO*) = delete;
        FileIO(FileIO&) = delete;

        //**************************************************
        /// \brief �R���X�g���N�^
        /// 
        /// \param[in] fileName
        /// 
        /// \return �Ȃ�
        //**************************************************
        FileIO(const char* fileName)
        {
            // �t�@�C���I�[�v��
            std::ifstream ifs(fileName, std::ios::binary);
            if (ifs.fail())
                return;

            ifs.seekg(0, std::ifstream::end);
            m_buffer.resize(size_t(ifs.tellg()));
            ifs.seekg(std::ifstream::beg);

            ifs.read(&m_buffer[0], m_buffer.size());

            // �t�@�C�������
            ifs.close();
        }

    public:
        //**************************************************
        /// \brief �ǂݍ��񂾃f�[�^�̎擾
        /// 
        /// \return �t�@�C���f�[�^
        //**************************************************
        std::string GetInfo() { return m_buffer; }

    private:
        std::string m_buffer;
    };
}

#endif // !FILEIO_H_