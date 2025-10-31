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

HRESULT CFont_Manager::Add_Font(const _wstring& strFontTag, const _char* pFilePath, const _uint iPixelHeight)
{
	if (nullptr != Find_Font(strFontTag))
		return E_FAIL;

	// load for freetype

	//FT_Face pFont = { nullptr };
	FTCUSTOM_FONT* pFontInfo = new FTCUSTOM_FONT();
	
	if (FT_New_Face(m_pFTLibrary, pFilePath, 0, &pFontInfo->pFace))
		CRASH("Add Font");

	Load_Font(pFontInfo, pFilePath, iPixelHeight);	// height : font height
	m_Fonts.emplace(strFontTag, pFontInfo);

	return S_OK;
}

HRESULT CFont_Manager::Load_Font(FTCUSTOM_FONT* pFontInfo, const _char* pFilePath, _uint iPixelHeight)
{
	FT_Select_Charmap(pFontInfo->pFace, FT_ENCODING_UNICODE);	// 유니코드 문자맵
	FT_Set_Pixel_Sizes(pFontInfo->pFace, 0, iPixelHeight);

	pFontInfo->iPixelHeight = iPixelHeight;
	pFontInfo->isHasKerning = FT_HAS_KERNING(pFontInfo->pFace) ? TRUE : FALSE;

	Create_EmptyAtlas(pFontInfo);

	return S_OK;
}

HRESULT CFont_Manager::Create_EmptyAtlas(FTCUSTOM_FONT* pFontInfo, _uint iAtlasW, _uint iAtlasH)
{
	// 입력받은 크기에 맞게 빈 아틀라스를 만듭니다.

	pFontInfo->iAtlasW = iAtlasW;
	pFontInfo->iAtlasH = iAtlasH;
	pFontInfo->iPenX = pFontInfo->iPenY = pFontInfo->iRowH = 0;

	D3D11_TEXTURE2D_DESC td = {};
	td.Width = iAtlasW; td.Height = iAtlasH;
	td.MipLevels = 1; td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8_UNORM;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = m_pDevice->CreateTexture2D(&td, nullptr, &pFontInfo->pAtlasTex);
	if (FAILED(hr)) return hr;

	D3D11_SHADER_RESOURCE_VIEW_DESC sd = {};
	sd.Format = td.Format;
	sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sd.Texture2D.MipLevels = 1;

	hr = m_pDevice->CreateShaderResourceView(pFontInfo->pAtlasTex, &sd, &pFontInfo->pAtlasSRV);
	if (FAILED(hr)) return hr;

	D3D11_SAMPLER_DESC smp = {};
	smp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	smp.AddressU = smp.AddressV = smp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	hr = m_pDevice->CreateSamplerState(&smp, &pFontInfo->pSampler);
	if (FAILED(hr)) return hr;

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

_bool CFont_Manager::Atlas_AllocRect(FTCUSTOM_FONT* pFont, _int iGlyphWidth, _int iGlyphHeight, _int& outX, _int& outY)
{
	// 생성했던 아틀라스에 빈 영역을 할당하여, 글리프를 담을 공간을 인자로 내보냅니다.
	// 
	// 1. 폰트 정보와 폰트가 차지할 크기의 글리프 크기를 받아와,
	// 2. 빈 공간 또는 당장 사용하지 않을 공간을 찾아내서,
	// 3. 해당 공간을 사용할 공간으로써 내보냄.
	//
	// 
	// 공간을 찾는 기준?
	// 
	// 매 호출마다 오른쪽, 아래로 인덱스를 넘기며 글리프를 채우고
	// 마지막에 도달하여 더이상 공간이 없을 경우 false를 떨궈, 해당 함수가 쓰이는 곳에서 리빌드를 하게 됨


	// 공백문자와 같이, 할당할 공간이 필요 없는 경우에는 무시합니다.
	if (iGlyphWidth <= 0 || iGlyphHeight <= 0)
    {
        outX = outY = 0;
        return true;
    }

    // 아틀라스의 현재 행에 공간이 없으면 줄바꿈을 시도합니다.
    if (pFont->iPenX + iGlyphWidth > pFont->iAtlasW)
    {
        pFont->iPenX = 0;
        pFont->iPenY += pFont->iRowH + 1;
        pFont->iRowH = 0;
    }

    // 아틀라스 공간이 부족한 경우
    if (pFont->iPenY + iGlyphHeight > pFont->iAtlasH)
        return false; // 나중에 리빌드 (더 큰 아틀라스 이미지 사용)

    outX = pFont->iPenX;
    outY = pFont->iPenY;

    pFont->iPenX += iGlyphWidth + 1;
    if (iGlyphHeight > pFont->iRowH)
        pFont->iRowH = iGlyphHeight;

    return true;
}

CFont_Manager::FTCUSTOM_FONT* CFont_Manager::Find_Font(const _wstring& strFontTag)
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

	//for (auto& Pair : m_Fonts)
	//	Pair.second = nullptr;

	for (auto& [tag, face] : m_Fonts)
	{
		if (face->pSampler)			face->pSampler->Release();
		if (face->pAtlasSRV)		face->pAtlasSRV->Release();
		if (face->pAtlasTex)		face->pAtlasTex->Release();

		FT_Done_Face(face->pFace);
		Safe_Delete(face);
	}

	m_Fonts.clear();

	//m_pFTLibrary = nullptr;

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
