// Font Manager Experimant Header(1)

#pragma once
#include "Base.h"

#include "CustomFont.h"

NS_BEGIN(Engine)

class ENGINE_DLL CFont_Manager final : public CBase
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
	FTCUSTOM_FONT*						Find_Font(const _wstring& strFontTag);
	ID3D11ShaderResourceView*			Get_AtlasSRV(const _wstring& tag);
	const FTCUSTOM_FONT_GLYPH*			Get_Glyph(const _wstring& tag, _uint code);
	const FTCUSTOM_FONT_GLYPH*			Get_GlyphAndAdvance(const _wstring& fontTag, _uint codePoint, _uint prevCodePoint, _int& outAdvanceX);    // 커닝 포함된 실제 이동값
	


private:
	//HRESULT								Ready_FontBuffer();
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

	FT_Library							m_pFTLibrary = { nullptr };
	unordered_map
		<_wstring, FTCUSTOM_FONT*>	    m_Fonts;

	_uint								m_iWinSizeX = {};
	_uint								m_iWinSizeY = {};

	class CGameInstance*				m_pGameInstance = { nullptr };

public:
	static		CFont_Manager*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual		void					Free() override;
};

NS_END
