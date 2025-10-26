#include "EnginePch.h"
#include "Font_Manager.h"

CFont_Manager::CFont_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CFont_Manager::Initialize()
{
	if (FT_Init_FreeType(&m_pFTLibrary))
		CRASH("FT Library");

	return S_OK;
}

HRESULT CFont_Manager::Add_Font(const _wstring& strFontTag, const _char* pFilePath)
{
	if (nullptr != Find_Font(strFontTag))
		return E_FAIL;

	FT_Face pFont = { nullptr };
	if (FT_New_Face(m_pFTLibrary, pFilePath, 0, &pFont))
		CRASH("Add Font");

	m_Fonts.emplace(strFontTag, pFont);

	return S_OK;
}

HRESULT CFont_Manager::Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRadian, const _float2& vOrigin, const _float2& vScale)
{
	//FT_Face pFont = Find_Font(strFontTag);
	//if (nullptr == pFont)
	//	return E_FAIL;
	//
	//return pFont->Render(pText, vPosition, vColor, fRadian, vOrigin, vScale);
	return S_OK;
}

FT_Face CFont_Manager::Find_Font(const _wstring& strFontTag)
{
	auto iter = m_Fonts.find(strFontTag);

	if (iter == m_Fonts.end())
		return nullptr;

	return iter->second;
}

CFont_Manager* CFont_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFont_Manager* pInstance = new CFont_Manager(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
		CRASH("FontManager");

	return pInstance;
}

void CFont_Manager::Free()
{
	__super::Free();

	for (auto& Pair : m_Fonts)
		Pair.second = nullptr;
	m_Fonts.clear();

	m_pFTLibrary = nullptr;

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
