#pragma once
#include "Base.h"

#include "CustomFont.h"

NS_BEGIN(Engine)

class CFont_Manager final : public CBase
{
private:
	explicit CFont_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CFont_Manager() = default;

public:
	HRESULT						Initialize();

	HRESULT						Add_Font(const _wstring& strFontTag, const _char* pFilePath);
	HRESULT						Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRadian, const _float2& vOrigin, const _float2& vScale);

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	FT_Library					m_pFTLibrary = { nullptr };

	map<const _wstring, FT_Face>			m_Fonts;

private:
	FT_Face						Find_Font(const _wstring& strFontTag);

public:
	static		CFont_Manager*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void					Free() override;
};

NS_END

//	map<const _wstring, CCustomFont*>	m_Fonts;
//
//private:
//	CCustomFont*				Find_Font(const _wstring& strFontTag);