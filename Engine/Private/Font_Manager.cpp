#include "EnginePch.h"
#include "Font_Manager.h"

#include "Shader.h"
#include "CustomFont.h"
#include "GameInstance.h"

#define KSTA_DEBUGATLASTEST
#define KSTA_FONTSCREEN_TO3D

CFont_Manager::CFont_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext }
	, m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CFont_Manager::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	if (FT_Init_FreeType(&m_pFTLibrary))
		CRASH("FT Library");
	
	m_pShaderCom = CShader::Create(m_pDevice, m_pContext,
		TEXT("../../Client/Bin/ShaderFiles/Shader_UIText.hlsl"), VTXUITEXT::Elements, VTXUITEXT::iNumElements);

	Ready_FontBuffer();

	//m_pVIBufferCom->CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

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



	//wchar_t testChars[] = { L'A', L'B', L'C', 0 };
	//_wstring strFontTagDebug = L"WW_Medium";
	//TestGlyph(*Find_Font(strFontTagDebug), testChars[0]);
	//TestGlyph(*Find_Font(strFontTagDebug), testChars[1]);
	//TestGlyph(*Find_Font(strFontTagDebug), testChars[2]);


	return S_OK;
}

void CFont_Manager::Add_FloatingText(const _wstring& strFontTag, const _wstring& strText, _float2 vScreenPos, _float fScale, _float fLifeTime, _uint iShaderFlag, _float4 vColor)
{
	FONT_SINGLEDESC fontDesc = {};
	
	fontDesc.strFontTag = strFontTag;
	fontDesc.strText = strText;
	fontDesc.vScreenPos = vScreenPos;
	fontDesc.fScale = fScale;
	fontDesc.vLifeTime = _float2{0.f, fLifeTime};
	fontDesc.iShaderFlag = iShaderFlag;
	fontDesc.vColor = vColor;

	_uint iDestLevel = m_pGameInstance->Get_CurrentLevel();
	CCustomFont* pCustomFont = dynamic_cast<CCustomFont*>(m_pGameInstance->Clone_Prototype(0, L"Prototype_GameObject_Font", PROTOTYPE::GAMEOBJECT, &fontDesc));
	m_vecActiveFonts.push_back(pCustomFont);
}

void CFont_Manager::Add_FloatingText(FONT_SINGLEDESC tDesc)
{
	_uint iDestLevel = m_pGameInstance->Get_CurrentLevel();
	CCustomFont* pCustomFont = dynamic_cast<CCustomFont*>(m_pGameInstance->Clone_Prototype(0, L"Prototype_GameObject_Font", PROTOTYPE::GAMEOBJECT, &tDesc));
	m_vecActiveFonts.push_back(pCustomFont);
}


void CFont_Manager::Priority_Update(_float fTimeDelta)
{
	for (auto& activeFont : m_vecActiveFonts)
		activeFont->Priority_Update(fTimeDelta);
}

void CFont_Manager::Update(_float fTimeDelta)
{
	for (_uint i = 0; i < m_vecActiveFonts.size(); i++)
		if (m_vecActiveFonts[i]->Get_Active() == false)
		{
			Safe_Release(m_vecActiveFonts[i]);
			m_vecActiveFonts.erase(m_vecActiveFonts.begin() + i);
			i--;
			continue;
		}

	for (auto& activeFont : m_vecActiveFonts)
		activeFont->Update(fTimeDelta);

//	// 폰트들의 시간 경과를 업데이트하며, 시간이 이미 지나버린 폰트는 제거한다.
//	for (_uint i = 0; i < m_vecActiveFonts.size(); i++)
//	{
//		if (m_vecActiveFonts[i].vLifeTime.y <= m_vecActiveFonts[i].vLifeTime.x)
//		{
//			m_vecActiveFonts.erase(m_vecActiveFonts.begin() + i);
//			i--;
//			continue;
//		}
//
//		m_vecActiveFonts[i].vLifeTime.x += fTimeDelta;
//	}
}

void CFont_Manager::Late_Update(_float fTimeDelta)
{
	for (auto& activeFont : m_vecActiveFonts)
		activeFont->Late_Update(fTimeDelta);
}


//void CFont_Manager::Render()
//{
//	// 폰트들을 그린다.
//	for (auto& activeFont : m_vecActiveFonts)
//	{
//		Draw_Font(
//			Find_Font(activeFont.strFontTag), 
//			activeFont.strText.c_str(), 
//			activeFont.vScreenPos, 
//			activeFont.fScale, 
//			activeFont.vColor,
//			activeFont.iPassIndex
//		);
//	}
//
//}

HRESULT CFont_Manager::Ready_FontBuffer()
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(VTXUITEXT) * 3000;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&desc, nullptr, &m_pFontVertexBuffer)))
		return E_FAIL;

	return S_OK;
}

HRESULT CFont_Manager::Load_Font(FTCUSTOM_FONT* pFontInfo, const _char* pFilePath, _uint iPixelHeight)
{
	FT_Select_Charmap(pFontInfo->pFace, FT_ENCODING_UNICODE);	// 유니코드 문자맵
	FT_Set_Pixel_Sizes(pFontInfo->pFace, 0, iPixelHeight);

	pFontInfo->iPixelHeight = iPixelHeight;
	pFontInfo->isHasKerning = FT_HAS_KERNING(pFontInfo->pFace) ? true : false;

	Create_EmptyAtlas(pFontInfo);

	return S_OK;
}

_bool CFont_Manager::Reset_AtlasTexture(FTCUSTOM_FONT* pFont, _uint newW, _uint newH)
{
	Safe_Release(pFont->pAtlasSRV);
	Safe_Release(pFont->pAtlasTex);

	return SUCCEEDED(Create_EmptyAtlas(pFont, newW, newH));
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
	if (FAILED(hr)) 
		return hr;

	D3D11_SHADER_RESOURCE_VIEW_DESC sd = {};
	sd.Format = td.Format;
	sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sd.Texture2D.MipLevels = 1;

	hr = m_pDevice->CreateShaderResourceView(pFontInfo->pAtlasTex, &sd, &pFontInfo->pAtlasSRV);
	if (FAILED(hr)) 
		return hr;

	D3D11_SAMPLER_DESC smp = {};
	smp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	smp.AddressU = smp.AddressV = smp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	hr = m_pDevice->CreateSamplerState(&smp, &pFontInfo->pSampler);
	if (FAILED(hr)) 
		return hr;

	return S_OK;
}

_bool CFont_Manager::Rebuild_Atlas(FTCUSTOM_FONT* pFontInfo, _uint iAtlasW, _uint iAtlasH, _uint iPadding)
{
	// 1) 텍스처만 재생성
	if (!Reset_AtlasTexture(pFontInfo, iAtlasW, iAtlasH))
		return false;

	// 2) 기존 글리프 다시 채우기 (repack)
	pFontInfo->iPenX = pFontInfo->iPenY = pFontInfo->iRowH = 0;

	for (auto& [code, glyph] : pFontInfo->mapGlyphs)
	{
		FT_GlyphSlot slot = nullptr;
		if (!FT_RenderGlyph(pFontInfo->pFace, code, slot))
			return false;

		FT_Bitmap& bmp = slot->bitmap;
		int gw = bmp.width;
		int gh = bmp.rows;
		int paddedW = gw + iPadding * 2;
		int paddedH = gh + iPadding * 2;

		int x, y;
		if (!Atlas_AllocRect(pFontInfo, paddedW, paddedH, x, y))
			return false; // (이 경우는 새 크기로도 부족하다는 뜻)

		if (gw > 0 && gh > 0)
			Atlas_UploadBitmap(*pFontInfo, x + iPadding, y + iPadding, gw, gh, bmp.buffer, bmp.pitch);

		glyph.sOffsetX = slot->bitmap_left;
		glyph.sOffsetY = slot->bitmap_top;
		glyph.sWidth = gw;
		glyph.sHeight = gh;
		glyph.sAdvance = (slot->advance.x >> 6);

		glyph.fU0 = float(x + iPadding) / pFontInfo->iAtlasW;
		glyph.fV0 = float(y + iPadding) / pFontInfo->iAtlasH;
		glyph.fU1 = float(x + iPadding + gw) / pFontInfo->iAtlasW;
		glyph.fV1 = float(y + iPadding + gh) / pFontInfo->iAtlasH;
	}
	return true;
}

_bool CFont_Manager::Draw_Font(_wstring strFontTag, const _tchar* pText, _float2 vPos, _float fScale, _float4 vColor, _uint iShaderFlag)
{
	FONT_SINGLEDESC tDesc = {};

	tDesc.strFontTag = strFontTag;
	tDesc.strText = pText;
	tDesc.vScreenPos = vPos;
	tDesc.fScale = fScale;
	tDesc.vColor = vColor;
	tDesc.iShaderFlag = iShaderFlag;

	return Draw_Font(&tDesc);
}

_bool CFont_Manager::Draw_Font(FONT_SINGLEDESC* pDesc)
{
	_wstring strFontTag			= pDesc->strFontTag;
	_wstring strText			= pDesc->strText;
	const _tchar* pText			= pDesc->strText.c_str();

	_float2 vScreenPos			= pDesc->vScreenPos;
	_float  fScale				= pDesc->fScale;

	_float2 vLifeTime			= pDesc->vLifeTime;
	_int	iShaderFlag			= pDesc->iShaderFlag;

	// for shader
	_float4 vColor				= pDesc->vColor;				// Font Color

	// for shader : additional info for extra pass 
	// - outline
	_float4 vOutlineColor		= pDesc->vOutlineColor;
	_float fFontOutlineWidth	= pDesc->fFontOutlineWidth;
	// - grad
	_float4 vFontGradColor		= pDesc->vFontGradColor;		// Right Dir
	// - Fixed
	_bool isTargetExist			= pDesc->isTargetExist;
	_float4 vTargetWorldPos		= pDesc->vTargetWorldPos;		// Right Dir




	FTCUSTOM_FONT* pFontInfo = Find_Font(strFontTag);
	if (!pFontInfo || !pText)
		return false;

	vector<VTXUITEXT> vecVertices;
	vecVertices.reserve(512); // 대략 문자 80~100개 정도 버퍼 확보

	_float penX = vScreenPos.x;
	_float penY = vScreenPos.y;

#ifdef KSTA_FONTSCREEN_TO3D
	if (isTargetExist)
	{
		_float4x4 matPipelineView = *m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW);
		_float4x4 matPipelineProj = *m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ);

		_float3 world = { vTargetWorldPos.x, vTargetWorldPos.y, vTargetWorldPos.z };   // (x,y,z)
		_matrix view = XMLoadFloat4x4(&matPipelineView);
		_matrix proj = XMLoadFloat4x4(&matPipelineProj);
		_vector pos = XMVectorSet(world.x, world.y, world.z, 1.0f);

		pos = XMVector3Transform(pos, view);
		pos = XMVector3Transform(pos, proj);
		_vector ndc = pos / XMVectorSplatW(pos);

		_float3 ndc3;
		XMStoreFloat3(&ndc3, ndc);
		_float screenX = (ndc3.x * 0.5f + 0.5f) * m_iWinSizeX;     // 화면 해상도 X
		_float screenY = (1.0f - (ndc3.y * 0.5f + 0.5f)) * m_iWinSizeY; // Y 반전

		pDesc->vScreenPos = _float2(screenX, screenY);
	}
#endif // KSTA_FONTSCREEN_POSTEST




	_uint prevCode = 0;

	for (_uint i = 0; pText[i] != 0; )
	{
		_uint cp = (_uint)pText[i++]; // 단순 ASCII 또는 한글 BMP 영역까지는 OK
		if (cp == L'\n')
		{
			penX = vScreenPos.x;
			penY += pFontInfo->iPixelHeight * fScale;
			prevCode = 0;
			continue;
		}

		// 글리프가 atlas에 없으면 Bake
		if (!BakeOneGlyph(pFontInfo, cp, fFontOutlineWidth /** 2.f*/)) // ksta : 
			continue;
		FTCUSTOM_FONT_GLYPH glyph = pFontInfo->mapGlyphs[cp];

		// 커닝 적용 시
		if (pFontInfo->isHasKerning && prevCode != 0)
		{
			FT_Vector kern = {};
			FT_Get_Kerning(pFontInfo->pFace,
				FT_Get_Char_Index(pFontInfo->pFace, prevCode),
				FT_Get_Char_Index(pFontInfo->pFace, cp),
				FT_KERNING_DEFAULT, &kern);

			penX += (kern.x >> 6) * fScale;
		}

		// 실제 그려질 사각형 위치 계산 (bearing 적용)
		_float pad = fFontOutlineWidth;
		_float x0 = penX + (glyph.sOffsetX - pad) * fScale;
		_float y0 = penY - (glyph.sOffsetY + pad) * fScale;
		_float x1 = x0 + glyph.sWidth * fScale;
		_float y1 = y0 + glyph.sHeight * fScale;

		// UV
		_float u0 = glyph.fU0;
		_float v0 = glyph.fV0;
		_float u1 = glyph.fU1;
		_float v1 = glyph.fV1;

		VTXUITEXT vtx[6] =				// 정점 6개
		{	{{x0, y0}, {u0, v0}},	{{x1, y0}, {u1, v0}},	{{x1, y1}, {u1, v1}},
			{{x0, y0}, {u0, v0}},	{{x1, y1}, {u1, v1}},	{{x0, y1}, {u0, v1}} };
		vecVertices.insert(vecVertices.end(), vtx, vtx + 6);

		// 펜 이동
		penX += glyph.sAdvance * fScale;
		prevCode = cp;
	}

	if (vecVertices.empty())
		return true;

	// Dynamic VB에 업로드
	D3D11_MAPPED_SUBRESOURCE mapped;
	if (FAILED(m_pContext->Map(m_pFontVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		return false;

	memcpy(mapped.pData, vecVertices.data(), sizeof(VTXUITEXT) * static_cast<_uint>(vecVertices.size()));
	m_pContext->Unmap(m_pFontVertexBuffer, 0);


	// 셰이더 상수 설정
	_float2 screen = { static_cast<_float>(m_iWinSizeX), static_cast<_float>(m_iWinSizeY) };
	m_pShaderCom->Bind_Textures("g_FontAtlas", &pFontInfo->pAtlasSRV, 1);

	m_pShaderCom->Bind_Value("g_ScreenSize", &screen, sizeof(screen));
	m_pShaderCom->Bind_Value("g_FontColor", &vColor, sizeof(vColor));

	m_pShaderCom->Bind_Value("g_FontFlag", &iShaderFlag, sizeof(iShaderFlag));
	m_pShaderCom->Bind_Value("g_FontOutlineColor", &vOutlineColor, sizeof(vOutlineColor));
	//m_pShaderCom->Bind_Value("g_FontTexPerPixel", &vFontTexPerPixel, sizeof(vFontTexPerPixel)); // 이건 아래에서
	m_pShaderCom->Bind_Value("g_FontOutlineWidth", &fFontOutlineWidth, sizeof(fFontOutlineWidth));
	m_pShaderCom->Bind_Value("g_FontGradColor", &vFontGradColor, sizeof(vFontGradColor));
	_float4x4 matPipelineView = *m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW);
	_float4x4 matPipelineProj = *m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ);
	_float4 vCamPos = *m_pGameInstance->Get_CamPos();;
	//m_pShaderCom->Bind_Value("g_TargetWorldPos", &vTargetWorldPos, sizeof(vTargetWorldPos));
	//m_pShaderCom->Bind_Value("g_ViewMatrix", &matPipelineView, sizeof(matPipelineView));
	//m_pShaderCom->Bind_Value("g_ProjMatrix", &matPipelineProj, sizeof(matPipelineProj));
	//m_pShaderCom->Bind_Value("g_CamPos", &vCamPos, sizeof(vCamPos));

	// 상수.. Atlas Texel
	ID3D11Resource* pRes = nullptr;
	pFontInfo->pAtlasSRV->GetResource(&pRes);
	ID3D11Texture2D* pTex2D = nullptr;
	pRes->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTex2D);
	D3D11_TEXTURE2D_DESC desc = {};
	pTex2D->GetDesc(&desc);
	_float2 vTexPerPixel = { 1.0f / desc.Width,	1.0f / desc.Height };

	m_pShaderCom->Bind_Value("g_FontTexPerPixel", &vTexPerPixel, sizeof(_float2));
	Safe_Release(pTex2D);
	Safe_Release(pRes);

	
	// 렌더 상태 적용
	m_pShaderCom->Begin(0);

	// Bind Resource, Draw
	UINT stride = sizeof(VTXUITEXT);
	UINT offset = 0;
	m_pContext->IASetVertexBuffers(0, 1, &m_pFontVertexBuffer, &stride, &offset);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->Draw(vecVertices.size(), 0);


#ifdef KSTA_DEBUGATLASTEST

	ImGui::Image(pFontInfo->pAtlasSRV, ImVec2(512, 512));

#endif // KSTA_DEBUGATLASTEST



	return true;

}

static _bool FT_RenderGlyph(FT_Face face, _uint iCodePoint, FT_GlyphSlot& outSlot)
{
	// FreeType이 글자를 그레이스케일 비트맵으로 변환.
	if (FT_Load_Char(face, iCodePoint, FT_LOAD_RENDER))
		return false;

	outSlot = face->glyph;
	return true;
}

_bool CFont_Manager::Atlas_AllocRect(FTCUSTOM_FONT* pFontInfo, _int iGlyphWidth, _int iGlyphHeight, _int& outX, _int& outY)
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
    if (pFontInfo->iPenX + iGlyphWidth > pFontInfo->iAtlasW)
    {
		pFontInfo->iPenX = 0;
		pFontInfo->iPenY += pFontInfo->iRowH + 1;
		pFontInfo->iRowH = 0;
    }

    // 아틀라스 공간이 부족한 경우
    if (pFontInfo->iPenY + iGlyphHeight > pFontInfo->iAtlasH)
        return false; // 나중에 리빌드 (더 큰 아틀라스 이미지 사용)

    outX = pFontInfo->iPenX;
    outY = pFontInfo->iPenY;

	pFontInfo->iPenX += iGlyphWidth + 1;
    if (iGlyphHeight > pFontInfo->iRowH)
		pFontInfo->iRowH = iGlyphHeight;

    return true;
}

_bool CFont_Manager::Atlas_CheckSize(FTCUSTOM_FONT* pFontInfo, _int gw, _int gh, _int& outX, _int& outY, _uint iPadding)
{
	// 먼저 시도
	if (Atlas_AllocRect(pFontInfo, gw, gh, outX, outY))
		return true;

	// 리빌드 정책: 가로/세로 중 더 필요한 방향 위주로 2배 증가
	_uint newW = pFontInfo->iAtlasW;
	_uint newH = pFontInfo->iAtlasH;

	// 간단 정책: 둘 다 2배 (안전)
	newW = max(newW * 2, (_uint)(pFontInfo->iAtlasW + gw + 8));
	newH = max(newH * 2, (_uint)(pFontInfo->iAtlasH + gh + 8));

	// 상한 (원하면 제한)
	const _uint MAX_ATLAS = 4096;
	newW = min(newW, MAX_ATLAS);
	newH = min(newH, MAX_ATLAS);

	if (!Rebuild_Atlas(pFontInfo, newW, newH, iPadding))
		return false;

	// 리빌드 후 다시 시도
	return Atlas_AllocRect(pFontInfo, gw, gh, outX, outY);
}

_bool CFont_Manager::Atlas_UploadBitmap(FTCUSTOM_FONT& Font, _int x, _int y, _int w, _int h, const uint8_t* pSrc, _int srcPitch)
{
	// 얻은 비트맵 데이터를 실제 GPU 텍스쳐로 복사합니다.
	if (w <= 0 || h <= 0)
		return true;

	for (int row = 0; row < h; ++row)
	{
		D3D11_BOX box = { (UINT)x, (UINT)(y + row), 0, (UINT)(x + w), (UINT)(y + row + 1), 1 };
		const void* pRow = pSrc + row * srcPitch;
		m_pContext->UpdateSubresource(Font.pAtlasTex, 0, &box, pRow, w, 0);
	}
	return true;
}

_bool CFont_Manager::FT_RenderGlyph(FT_Face face, _uint iCodePoint, FT_GlyphSlot& outSlot)
{
	if (FT_Load_Char(face, iCodePoint, FT_LOAD_RENDER)) return FALSE;
	outSlot = face->glyph;
	return true;
}

_bool CFont_Manager::BakeOneGlyph(FTCUSTOM_FONT* pFontInfo, _uint iCodePoint, _uint iPadding)
{
	unordered_map<_uint, FTCUSTOM_FONT_GLYPH>& vecGlyphMap = pFontInfo->mapGlyphs;

	// 이미 존재하면 스킵
	if (vecGlyphMap.find(iCodePoint) != vecGlyphMap.end())
		return true;

	FT_GlyphSlot slot = nullptr;
	if (!FT_RenderGlyph(pFontInfo->pFace, iCodePoint, slot))
		return false;

	FT_Bitmap& bmp = slot->bitmap;
	int gw = (int)bmp.width;
	int gh = (int)bmp.rows;
	int paddedW = gw + iPadding * 2;
	int paddedH = gh + iPadding * 2;

	int x, y;
	if (!Atlas_CheckSize(pFontInfo, paddedW, paddedH, x, y, iPadding))
		return false; // 공간 부족

	if (gw > 0 && gh > 0)
		Atlas_UploadBitmap(*pFontInfo, x + iPadding, y + iPadding, gw, gh, bmp.buffer, bmp.pitch);

	FTCUSTOM_FONT_GLYPH tFontGlyph = {};
	tFontGlyph.iCodepoint	= iCodePoint;
	tFontGlyph.sOffsetX		= (_short)slot->bitmap_left;   // bearing X
	tFontGlyph.sOffsetY		= (_short)slot->bitmap_top;    // bearing Y
	tFontGlyph.sWidth		= (_short)(gw + iPadding * 2);
	tFontGlyph.sHeight		= (_short)(gh + iPadding * 2);
	tFontGlyph.sAdvance		= (_short)(slot->advance.x >> 6); // 픽셀 단위 advance

	tFontGlyph.fU0 = float(x) / pFontInfo->iAtlasW;
	tFontGlyph.fV0 = float(y) / pFontInfo->iAtlasH;
	tFontGlyph.fU1 = float(x + iPadding * 2 + gw) / pFontInfo->iAtlasW;
	tFontGlyph.fV1 = float(y + iPadding * 2 + gh) / pFontInfo->iAtlasH;

	vecGlyphMap[iCodePoint] = tFontGlyph;
	return true;
}

//_bool CFont_Manager::TestGlyph(FTCUSTOM_FONT& font, wchar_t ch)
//{
//	if (FT_Load_Char(font.pFace, ch, FT_LOAD_RENDER))
//	{
//		std::cout << "Failed to render char: " << (char)ch << "\n";
//		return false;
//	}
//
//	FT_GlyphSlot g = font.pFace->glyph;
//	int w = g->bitmap.width;
//	int h = g->bitmap.rows;
//	int x, y;
//
//	if (!Atlas_AllocRect(&font, w, h, x, y))
//	{
//		std::cout << "No space in atlas for char " << (char)ch << "\n";
//		return false;
//	}
//
//	std::cout << "Char '" << (char)ch << "' → "
//		<< "Size(" << w << "x" << h << "), "
//		<< "AtlasPos(" << x << ", " << y << ")\n";
//
//	return true;
//}

FTCUSTOM_FONT* CFont_Manager::Find_Font(const _wstring& strFontTag)
{
	auto iter = m_Fonts.find(strFontTag);

	if (iter == m_Fonts.end())
		return nullptr;

	return iter->second;
}

CFont_Manager* CFont_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CFont_Manager* pInstance = new CFont_Manager(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
		CRASH("FontManager");

	return pInstance;
}

void CFont_Manager::Free()
{
	__super::Free();

	//for (auto& Pair : m_Fonts)
	//	Pair.second = nullptr;

	for (auto& activeFont : m_vecActiveFonts)
	{
		Safe_Release(activeFont);
		activeFont = nullptr;
	}

	for (auto& [tag, face] : m_Fonts)
	{
		if (face->pSampler)			face->pSampler->Release();
		if (face->pAtlasSRV)		face->pAtlasSRV->Release();
		if (face->pAtlasTex)		face->pAtlasTex->Release();

		FT_Done_Face(face->pFace);
		Safe_Delete(face);
		face = nullptr;
	}

	m_Fonts.clear();

	if (m_pFTLibrary)
	{
		FT_Done_FreeType(m_pFTLibrary);
		m_pFTLibrary = nullptr;
	}

	Safe_Release(m_pFontVertexBuffer);

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
