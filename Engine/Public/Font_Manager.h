#pragma once
#include "Base.h"

#include "CustomFont.h"

NS_BEGIN(Engine)

class CFont_Manager final : public CBase
{
public:
    typedef struct SGlyph
    {
        _uint   iCodepoint;         // 유니코드 코드포인트
        _short  sOffsetX;           // bearingX
        _short  sOffsetY;           // bearingY (상향 +)
        _short  sWidth;             // bitmap.width
        _short  sHeight;            // bitmap.rows
        _short  sAdvance;           // advance.x >> 6
        _float  U0, V0, U1, V1;     // 아틀라스 UV
    }FTCUSTOM_FONT_GLYPH;

    typedef struct SFont
    {
        FT_Face                                     pFace;                      // 폰트 객체

        _int                                        iPixelHeight;               // 설정한 픽셀 사이즈
        ID3D11ShaderResourceView*                   pAtlasSRV;
        ID3D11Texture2D*                            pAtlasTex;
        ID3D11SamplerState*                         pSampler;
        unordered_map<_uint, FTCUSTOM_FONT_GLYPH>   mapGlyphs;                  // 코드포인트→글리프

        //  iAtlasW / iAtlasH   : 아틀라스(폰트 텍스처)의 전체 너비·높이.
        //  iPenX / iPenY       : 현재 글리프를 채워 넣을 "펜" 위치(다음 글리프 배치 시작 좌표).
        //  iRowH               : 현재 줄(row)에서 가장 높은 글리프의 높이(줄바꿈 간격 계산용).
        _int                                        iAtlasW, iAtlasH, iPenX, iPenY, iRowH;
        _bool                                       isHasKerning;
    }FTCUSTOM_FONT;


private:
	explicit CFont_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CFont_Manager() = default;

public:
	HRESULT								Initialize();

	HRESULT								Add_Font(const _wstring& strFontTag, const _char* pFilePath, const _uint iPixelHeight);
	HRESULT								Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRadian, const _float2& vOrigin, const _float2& vScale);

public:
    static _bool                        Atlas_AllocRect(FTCUSTOM_FONT* pFont, _int iGlyphWidth, _int iGlyphHeight, _int& outX, _int& outY);

private:
    HRESULT                             Load_Font(FTCUSTOM_FONT* pFontInfo, const _char* pFilePath, _uint iPixelHeight);
    HRESULT                             Create_EmptyAtlas(FTCUSTOM_FONT* pFontInfo, _uint iAtlasW = 1024, _uint iAtlasH = 1024);

private:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };

	FT_Library							m_pFTLibrary = { nullptr };

	//map<const _wstring, FT_Face>		m_Fonts;

	map<_wstring, FTCUSTOM_FONT*>	    m_Fonts;

private:
    FTCUSTOM_FONT*						Find_Font(const _wstring& strFontTag);

public:
	static		CFont_Manager*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void					Free() override;
};

NS_END

//	map<const _wstring, CCustomFont*>	m_Fonts;
//
//private:
//	CCustomFont*				Find_Font(const _wstring& strFontTag);