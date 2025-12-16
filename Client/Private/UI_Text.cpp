#include "ClientPch.h"
#include "UI_Text.h"

#include "Animator_UI.h"



//#define KSTA_UI_ATLAS_DEBUG

CUI_Text::CUI_Text(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCustom_UI(pDevice, pContext)
{
}

CUI_Text::CUI_Text(const CUI_Text& Prototype)
    :CCustom_UI(Prototype)
{
}

HRESULT CUI_Text::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Text::Initialize_Clone(void* pArg)
{
    //__super::Initialize_Clone(pArg);
	CUIObject::Initialize_Clone(pArg);

    Ready_Components(pArg);
	Ready_Events();
	Bind_Description(pArg);

	TEXT_UI_DESC* tDesc = static_cast<TEXT_UI_DESC*>(pArg);
	m_eTextAlignmentType = tDesc->eTextAlignmentType;
	Update_Alignment();

	m_tTextDesc = *static_cast<TEXT_UI_DESC*>(pArg);

    return S_OK;
}

void CUI_Text::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_Text::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);            // Update Animator_UI Component
	// Update_Description(fTimeDelta);
}

void CUI_Text::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
	Update_Description(0.f);
}

void CUI_Text::Render()
{
    //__super::Render();                      // Binding Shader Variables Continuously.
	if (!m_isActivate || !m_pShaderCom)
		return;

	if (m_tUIDesc.isInstance && m_cachedVariantUIDesc.isVariant)        // 짬통 UI용. 어떤 유형의 UI에 쓸 건지의 Flag를 전역으로 던진다.
		if (FAILED(m_pShaderCom->Bind_Value("g_iVariantFlag", &m_cachedVariantUIDesc.iShaderFlag, sizeof(m_cachedVariantUIDesc.iShaderFlag))))
			CRASH("Binding_Value_Failed");

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrixPtr())))	// ksta : 부모관계 필요 시 수정
		CRASH("Binding_Matrix_Failed");

	XMStoreFloat4x4(&m_ViewMatrix, m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Binding_Matrix_Failed");
	XMStoreFloat4x4(&m_ProjMatrix, m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Binding_Matrix_Failed");

	ID3D11ShaderResourceView* pFontSRV = m_pGameInstance->Get_AtlasSRV(m_tTextDesc.strFontTag);

#ifdef KSTA_UI_ATLAS_DEBUG
	ImGui::Image(pFontSRV, ImVec2(512, 512));
#endif // KSTA_UI_ATLAS_DEBUG


	if (FAILED(m_pShaderCom->Bind_Texture("g_Texture", pFontSRV)))
		CRASH("Binding Font Atlas Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_InverseScreenDiscard", &m_tUIDesc.isInverseScreenDiscard, sizeof(m_tUIDesc.isInverseScreenDiscard))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_CutoutAlphaDiscard", &m_tUIDesc.fCutout, sizeof(m_tUIDesc.fCutout))))
		CRASH("Binding_Value_Failed");

	if (FAILED(m_pShaderCom->Bind_Value("g_SectorBorder", &m_tUIDesc.vSectorBorder, sizeof(m_tUIDesc.vSectorBorder))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_UIScale", &m_tUIDesc.fUIScale, sizeof(m_tUIDesc.fUIScale))))
		CRASH("Binding_Value_Failed");


	if (FAILED(m_pShaderCom->Bind_Value("g_isTargetExist", &m_tTextDesc.isTargetExist, sizeof(m_tTextDesc.isTargetExist))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_vTargetWorldPos", &m_tTextDesc.vTargetWorldPos, sizeof(m_tTextDesc.vTargetWorldPos))))
		CRASH("Binding_Value_Failed");

	const _float4 vCamPos = *m_pGameInstance->Get_CamPos();
	if (FAILED(m_pShaderCom->Bind_Value("g_vCamPosition", &vCamPos, sizeof(vCamPos))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_FontFlag", &m_tTextDesc.iShaderFlag, sizeof(m_tTextDesc.iShaderFlag))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_FontColor", &m_tTextDesc.vColor, sizeof(m_tTextDesc.vColor))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_FontOutlineColor", &m_tTextDesc.vOutlineColor, sizeof(m_tTextDesc.vOutlineColor))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_FontOutlineWidth", &m_tTextDesc.fFontOutlineWidth, sizeof(m_tTextDesc.fFontOutlineWidth))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_LifeTime", &m_tTextDesc.vLifeTime, sizeof(m_tTextDesc.vLifeTime))))
		CRASH("Binding_Value_Failed");
	if (FAILED(m_pShaderCom->Bind_Value("g_FontScale", &m_tTextDesc.fScale, sizeof(m_tTextDesc.fScale))))
		CRASH("Binding_Value_Failed");
	

	FTCUSTOM_FONT* pFontInfo = m_pGameInstance->Find_Font(m_tTextDesc.strFontTag);
	ID3D11Resource* pRes = nullptr;
	pFontInfo->pAtlasSRV->GetResource(&pRes);
	ID3D11Texture2D* pTex2D = nullptr;
	pRes->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTex2D);
	D3D11_TEXTURE2D_DESC desc = {};
	pTex2D->GetDesc(&desc);
	_float2 vTexPerPixel = { 1.0f / desc.Width,	1.0f / desc.Height };
	m_pShaderCom->Bind_Value("g_FontTexPerPixel", &vTexPerPixel, sizeof(_float2));
	pTex2D->Release();
	pRes->Release();
	

	m_pShaderCom->Begin(m_tUIDesc.iPassType);
	m_pVIBufferCom->Bind_Resources();
	m_pVIBufferCom->Render();
}

HRESULT CUI_Text::Ready_Components(void* pArg)
{
	// 텍스쳐 컴포넌트를 사용하지 않고, 직접 폰트매니저로부터 제작된 원본 SRV를 받아와 사용한다.
	ASSERT_CRASH(pArg);
	TEXT_UI_DESC* pDesc = static_cast<TEXT_UI_DESC*>(pArg);

	const	_wstring	strFilePath = pDesc->strFilePath;
	const	_wstring	strFileName = pDesc->strFileName;
	const	_uint		iNumFiles = pDesc->iNumFiles;
	const   _uint       iDestLevel = m_pGameInstance->Get_CurrentLevel();
	const   _bool       isInstance = pDesc->isInstance;

	const	_wstring	strFontTag = pDesc->strFontTag;

	// 1. Font Manager 로부터 SRV를 받아옴. 이후 Bind Texture 할 때에 SRV를 전달해줌. 이건 그냥 렌더측에서 바로 받도록 변경
	///FTCUSTOM_FONT* pFont = m_pGameInstance->Find_Font(pDesc->strFontTag);
	///if (!pFont) return;
	///m_SRVs.push_back(pFont->pAtlasSRV);
	
	// 2. 폰트 글자 하나마다를 조작해야 하기 때문에 인스턴스용을 사용함.
	if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Shader_Text_Instance"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return E_FAIL;
	if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect_Instance_UI"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
		return E_FAIL;

	// 3. CustomUI 와의 호환성을 위해 Animator_UI를 사용함.
	CAnimator_UI::ANIMATOR_UI_DESC tAnimatorUIDesc = { this };
	if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Animator_UI"),
		TEXT("Com_Animator_UI"), reinterpret_cast<CComponent**>(&m_pAnimator_UICom), &tAnimatorUIDesc)))
		return E_FAIL;

    return S_OK;
}

HRESULT	CUI_Text::Bind_Description(void* pArg)
{
	ASSERT_CRASH(pArg);
	TEXT_UI_DESC* pDesc = static_cast<TEXT_UI_DESC*>(pArg);

	m_tUIDesc.strFilePath   = pDesc->strFilePath;
    m_tUIDesc.strFileName   = pDesc->strFileName;
    m_tUIDesc.iNumFiles     = pDesc->iNumFiles;

    m_tUIDesc.strUIName     = ((pDesc->strUIName).empty()) ? m_tUIDesc.strFileName : pDesc->strUIName;
    m_tUIDesc.iUIType       = pDesc->iUIType;
    m_tUIDesc.strParentName = pDesc->strParentName;
    m_tUIDesc.fCutout       = pDesc->fCutout;
    m_tUIDesc.iPassType     = pDesc->iPassType;
    m_tUIDesc.vecChildNames = pDesc->vecChildNames;

	m_tUIDesc.vSectorBorder = pDesc->vSectorBorder;
	m_tUIDesc.fUIScale		= pDesc->fUIScale;
	m_tUIDesc.isInstance	= pDesc->isInstance;

	m_tUIDesc.vecInstanceDescs = pDesc->vecInstanceDescs;
	

	m_vOriginScreenPos		= pDesc->vScreenPos;

#ifdef KSTA_ON_TRANSFORM_CACHING
	m_vecCachedUITransform.resize(pDesc->vecInstanceDescs.size());




	_uint iCacheTransformAmount = (m_tUIDesc.isInstance) ? pDesc->strText.size() : 1;
	m_vecCachedUITransform.resize(iCacheTransformAmount);

	m_tUIDesc.vecInstanceDescs = pDesc->vecInstanceDescs;
#endif // KSTA_ON_TRANSFORM_CACHING


	return S_OK;
}

void CUI_Text::Update_Description(_float fTimeDelta)
{
	const _wstring& text = m_tTextDesc.strText;
	if (text.empty())
		return;

	auto* pFont = m_pGameInstance->Find_Font(m_tTextDesc.strFontTag);
	if (!pFont) return;

	const _uint iPadding = pFont->iPadding;

	// 인스턴스 컨테이너 초기화
	// (줄바꿈 문자는 인스턴스를 생성하지 않으므로 넉넉하게 잡고 나중에 줄입니다)
	if (m_tTextDesc.vecInstanceDescs.capacity() < text.size())
		m_tTextDesc.vecInstanceDescs.reserve(text.size());

	m_tTextDesc.vecInstanceDescs.clear();

	_float penY = 0.f;
	_uint instIndex = 0;

	// 텍스트를 줄 단위로 처리하기 위한 인덱스
	size_t iLineStartIdx = 0;
	size_t iTextLen = text.length();

	while (iLineStartIdx < iTextLen)
	{
		// 1. 현재 줄의 끝(\n)을 찾습니다.
		size_t iLineEndIdx = text.find(L'\n', iLineStartIdx);
		if (iLineEndIdx == _wstring::npos)
			iLineEndIdx = iTextLen;

		// 2. 현재 줄의 가로 폭(Width)을 미리 계산합니다.
		_float fLineWidth = 0.f;
		{
			_uint tempPrevCode = 0;
			for (size_t i = iLineStartIdx; i < iLineEndIdx; ++i)
			{
				_int advanceX = 0;
				// 너비 계산용이므로 advance값만 받아옵니다.
				if (m_pGameInstance->Get_GlyphAndAdvance(m_tTextDesc.strFontTag, text[i], tempPrevCode, advanceX))
				{
					fLineWidth += (_float)advanceX * m_tTextDesc.fScale;
					tempPrevCode = text[i];
				}
			}
		}

		// 3. 정렬 방식에 따라 시작 X좌표(penX) 보정
		_float penX = 0.f;
		if (m_eTextAlignmentType == TEXT_ALIGN_TYPE::CENTER)
		{
			penX = -fLineWidth * 0.5f; // 너비의 절반만큼 왼쪽으로 이동
		}
		else if (m_eTextAlignmentType == TEXT_ALIGN_TYPE::RIGHT)
		{
			penX = -fLineWidth;        // 너비 전체만큼 왼쪽으로 이동
		}
		// LEFT인 경우 penX = 0.f 유지

		// 4. 현재 줄의 글자들을 인스턴스로 생성
		_uint prevCode = 0;
		for (size_t i = iLineStartIdx; i < iLineEndIdx; ++i)
		{
			wchar_t ch = text[i];

			_int advanceX = 0;
			const FTCUSTOM_FONT_GLYPH* pGlyph =
				m_pGameInstance->Get_GlyphAndAdvance(m_tTextDesc.strFontTag, ch, prevCode, advanceX);

			if (!pGlyph) continue;

			// 벡터에 공간 확보 (push_back 대신 인덱싱을 썼던 기존 로직 대응)
			m_tTextDesc.vecInstanceDescs.emplace_back();
			auto& inst = m_tTextDesc.vecInstanceDescs.back();

			// UV 좌표
			inst.vSInstCoordX = { pGlyph->fU0, pGlyph->fU1 };
			inst.vSInstCoordY = { pGlyph->fV0, pGlyph->fV1 };

			// 크기 설정
			inst.vSInstRight = { m_tTextDesc.fScale * (pGlyph->sWidth), 0.f, 0.f ,0.f };
			inst.vSInstUp = { 0.f, m_tTextDesc.fScale * pGlyph->sHeight, 0.f ,0.f };
			inst.vSInstLook = { 0.f, 0.f, 1.f ,0.f };

			// Alpha 값 처리 (부모 UI 등에서 가져옴)
			inst.matExtraData._11 = m_pAnimator_UICom->Get_CurCombinedAnimKeyframeDesc()->fAlpha;

			// 위치 설정 (Target 존재 여부 분기)
			if (!m_tTextDesc.isTargetExist)
			{
				inst.vSInstTrans.x = m_tTextDesc.vScreenPos.x
					+ m_CombinedWorldMatrix._41
					+ penX
					+ (_float)pGlyph->sOffsetX * m_tTextDesc.fScale - iPadding * m_tTextDesc.fScale;

				inst.vSInstTrans.y = m_tTextDesc.vScreenPos.y
					- m_CombinedWorldMatrix._42
					- (_float)pGlyph->sOffsetY * m_tTextDesc.fScale + iPadding * m_tTextDesc.fScale
					+ penY;
			}
			else
			{
				// Target이 있을 때 로직 (기존 유지)
				inst.vSInstTrans.x = penX
					+ (_float)pGlyph->sOffsetX * m_tTextDesc.fScale - iPadding * m_tTextDesc.fScale;

				inst.vSInstTrans.y = -(_float)pGlyph->sOffsetY * m_tTextDesc.fScale + iPadding * m_tTextDesc.fScale
					+ penY;
			}

			// 다음 글자 위치로 이동
			penX += (_float)advanceX * m_tTextDesc.fScale;
			prevCode = ch;
		}

		// 5. 다음 줄 준비
		penY += pFont->iPixelHeight * m_tTextDesc.fScale * m_tTextDesc.fLineSpace;
		iLineStartIdx = iLineEndIdx + 1; // \n 다음 글자부터 시작
	}

	m_tUIDesc.vecInstanceDescs = m_tTextDesc.vecInstanceDescs;

#ifdef KSTA_ON_TRANSFORM_CACHING
	if (m_tTextDesc.vecInstanceDescs.size() != m_vecCachedUITransform.size())
		m_vecCachedUITransform.resize(m_tTextDesc.vecInstanceDescs.size());
#endif 
}

void CUI_Text::Update_Alignment(TEXT_ALIGN_TYPE eAlignmentType)
{
	// 정렬 타입 갱신
	if (eAlignmentType != TEXT_ALIGN_TYPE::END)
		m_eTextAlignmentType = eAlignmentType;

	// 이미 Update_Description에서 줄별 정렬을 수행하므로,
	// 여기서 인스턴스를 일괄 이동시키는 코드는 삭제하거나 
	// 필요하다면 다시 Update_Description(0.f)를 호출하여 갱신합니다.

	// 기존 로직 제거 권장:
	// fVisualLeft, fVisualRight 계산하여 delta 이동시키는 부분 삭제
}

void CUI_Text::Change_Text(_wstring strText, TEXT_ALIGN_TYPE eAlignmentType)
{
	m_tTextDesc.strText = strText;
	Update_Description(0.f);
	Update_Alignment(eAlignmentType);
}

HRESULT CUI_Text::Attach_AsChildToUI(CCustom_UI* pAttachTargetUI)
{
	CCustom_UI* pAttacher = pAttachTargetUI;
	auto& fontDesc = this->Get_UIDesc();
	auto& attacherDesc = pAttacher->Get_UIDesc();

	attacherDesc.vecChildNames.push_back(fontDesc.strUIName);

	pAttacher->Add_Child(this);
	fontDesc.strParentName = pAttacher->Get_UIDesc().strUIName;
	fontDesc.pParentObject = pAttacher;

	for (auto& inst : fontDesc.vecInstanceDescs)
		inst.matExtraData._11 = 1.f;

	this->Update_Description(0.f);


	return S_OK;
}

CUI_Text* CUI_Text::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Text* pInstance = new CUI_Text(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Text");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_Text::Clone(void* pArg)
{
    CUI_Text* pInstance = new CUI_Text(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CUI_Text");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_Text::Free()
{
	for (auto& srv : m_SRVs)
		Safe_Release(srv);

    __super::Free();
}
