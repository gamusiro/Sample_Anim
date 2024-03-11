/*===================================================================================
*    Date : 2022/11/28(月)
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
        /// \brief コンストラクタ
        /// 
        /// \param[in] fileName
        /// 
        /// \return なし
        //**************************************************
        FileIO(const char* fileName)
        {
            // ファイルオープン
            std::ifstream ifs(fileName, std::ios::binary);
            if (ifs.fail())
                return;

            ifs.seekg(0, std::ifstream::end);
            m_buffer.resize(size_t(ifs.tellg()));
            ifs.seekg(std::ifstream::beg);

            ifs.read(&m_buffer[0], m_buffer.size());

            // ファイルを閉じる
            ifs.close();
        }

    public:
        //**************************************************
        /// \brief 読み込んだデータの取得
        /// 
        /// \return ファイルデータ
        //**************************************************
        std::string GetInfo() { return m_buffer; }

    private:
        std::string m_buffer;
    };
}

#endif // !FILEIO_H_