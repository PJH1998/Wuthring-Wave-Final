#pragma once
#include "Base.h"

#include "CustomFont.h"

NS_BEGIN(Engine)

class CFont_Manager final : public CBase
{
public:
	typedef struct tFontMgrDesc
	{
		_float2 vScreenSize = {};
	}FONT_MANAGER_DESC;

private:
	explicit CFont_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CFont_Manager() = default;

public:
	HRESULT								Initialize(_uint iWinSizeX, _uint iWinSizeY);

	HRESULT								Add_Font(const _wstring& strFontTag, const _char* pFilePath, const _uint iPixelHeight);

	//HRESULT								Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float2 vScale);
	//void								Add_FloatingText(/*const _wstring& strFontTag, const _tchar* pText, */FONT_SINGLEDESC tSingleFontDesc);
	void								Add_FloatingText(const _wstring& strFontTag, const _wstring& pText, _float2 vScreenPos, _float fScale, _float fLifeTime, _uint iShaderFlag, _float4 vColor);
	void								Add_FloatingText(FONT_SINGLEDESC tDesc);	// for transfer additional infos

	void								Priority_Update(_float fTimeDelta);
	void								Update(_float fTimeDelta);
	void								Late_Update(_float fTimeDelta);
	//void								Render();

public:
	_bool								Draw_Font(_wstring strFontTag, const _tchar* pText, _float2 vPos, _float fScale, _float4 vColor, _uint iShaderFlag);
	_bool								Draw_Font(FONT_SINGLEDESC* pSingleDesc);
	
private:
	HRESULT								Ready_FontBuffer();
	HRESULT                             Load_Font(FTCUSTOM_FONT* pFontInfo, const _char* pFilePath, _uint iPixelHeight);


	_bool								Reset_AtlasTexture(FTCUSTOM_FONT* pFont, _uint newW, _uint newH);
    HRESULT                             Create_EmptyAtlas(FTCUSTOM_FONT* pFontInfo, _uint iAtlasW = 1024, _uint iAtlasH = 1024);


	static _bool                        Atlas_AllocRect(FTCUSTOM_FONT* pFontInfo, _int iGlyphWidth, _int iGlyphHeight, _int& outX, _int& outY);
	_bool								Atlas_CheckSize(FTCUSTOM_FONT* pFontInfo, _int gw, _int gh, _int& outX, _int& outY, _uint iPadding);
	_bool								BakeOneGlyph(FTCUSTOM_FONT* pFontInfo, _uint iCodePoint, _uint iPadding);
	_bool								Atlas_UploadBitmap(FTCUSTOM_FONT& Font, _int x, _int y, _int w, _int h,
														const uint8_t* pSrc, _int srcPitch);
	static _bool						FT_RenderGlyph(FT_Face face, _uint iCodePoint, FT_GlyphSlot& outSlot);
	_bool								Rebuild_Atlas(FTCUSTOM_FONT* pFontInfo, _uint iAtlasW, _uint iAtlasH, _uint iPadding);

private:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };
	ID3D11Buffer*						m_pFontVertexBuffer = { nullptr };

	class CShader*						m_pShaderCom = { nullptr };
	//class CVIBuffer*					m_pVIBufferCom  = { nullptr };
	
	FT_Library							m_pFTLibrary = { nullptr };
	//map<const _wstring, FT_Face>		m_Fonts;
	map<_wstring, FTCUSTOM_FONT*>	    m_Fonts;


	//vector<FONT_SINGLEDESC>				m_vecActiveFonts;
	vector<CCustomFont*>				m_vecActiveFonts;


	_uint								m_iWinSizeX = {};
	_uint								m_iWinSizeY = {};



	class CGameInstance*				m_pGameInstance = { nullptr };

public:
    FTCUSTOM_FONT*						Find_Font(const _wstring& strFontTag);

public:
	static		CFont_Manager*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual		void					Free() override;
};

NS_END

//	map<const _wstring, CCustomFont*>	m_Fonts;
//
//private:
//	CCustomFont*				Find_Font(const _wstring& strFontTag);