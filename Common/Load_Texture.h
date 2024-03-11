/*===================================================================================
*    Date : 2022/12/04(日)
*        Author		: Gakuto.S
*        File		: Load_Texture.h
*        Detail		:
===================================================================================*/
#ifndef LOAD_TEXTURE_H_
#define LOAD_TEXTURE_H_

namespace common
{
	class LoadTexture
	{
	public:
		LoadTexture() = delete;
		LoadTexture(LoadTexture*) = delete;
		LoadTexture(LoadTexture&) = delete;

        //**************************************************
        /// \brief コンストラクタ
        /// 
        /// \param[in] fileName
        /// 
        /// \return なし
        //**************************************************
        LoadTexture(const char* fileName);

        //**************************************************
        /// \brief コンストラクタ
        /// 
        /// \param[in] buffer
        /// 
        /// \return なし
        //**************************************************
        LoadTexture(const unsigned char* buffer, int length);

        //**************************************************
        /// \brief デストラクタ
        /// 
        /// \return なし
        //**************************************************
        ~LoadTexture();

        //**************************************************
        /// \brief 画像の幅を取得
        /// 
        /// \return 画像の幅
        //**************************************************
        int GetWidth() { return m_width; }

        //**************************************************
        /// \brief 画像の高さを取得
        /// 
        /// \return 画像の高さ
        //**************************************************
        int GetHeight() { return m_height; }

        //**************************************************
        /// \brief 画像のデータのアドレス
        /// 
        /// \return アドレス
        //**************************************************
        unsigned char* GetImage() { return m_image; }

    private:
        unsigned char*  m_image;
        int             m_width;
        int             m_height;

    public:
        static constexpr int       k_RGBA = 4;
	};
}

#endif // !LOAD_TEXTURE_H_
