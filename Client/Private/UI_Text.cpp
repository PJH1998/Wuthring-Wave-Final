#include "ClientPch.h"
#include "UI_Text.h"

#include "Animator_UI.h"

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

	m_tTextDesc = *static_cast<TEXT_UI_DESC*>(pArg);

    return S_OK;
}

void CUI_Text::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_Text::Update(_float fTimeDelta)
{
	Update_Description();

    __super::Update(fTimeDelta);            // Update Animator_UI Component
}

void CUI_Text::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
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
	ImGui::Image(pFontSRV, ImVec2(512, 512));
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

	const _float4* vCamPos = m_pGameInstance->Get_CamPos();
	if (FAILED(m_pShaderCom->Bind_Value("g_vCamPosition", &vCamPos, sizeof(vCamPos))))
		CRASH("Binding_Value_Failed");


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


	m_vecCachedUITransform.resize(pDesc->vecInstanceDescs.size());

	m_tUIDesc.vecInstanceDescs = pDesc->vecInstanceDescs;



	_uint iCacheTransformAmount = (m_tUIDesc.isInstance) ? pDesc->strText.size() : 1;
	m_vecCachedUITransform.resize(iCacheTransformAmount);

	m_tUIDesc.vecInstanceDescs = pDesc->vecInstanceDescs;

	return S_OK;
}

void CUI_Text::Update_Description()
{

	// m_tTextDesc 갱신

	const _wstring& text = m_tTextDesc.strText;
	if (text.empty())
		return;

	// 1. 폰트 매니저에서 폰트 정보 받아오기
	auto* pFont = m_pGameInstance->Find_Font(m_tTextDesc.strFontTag);
	if (!pFont) return;

	_float penX = 0.f;
	_float penY = 0.f;
	_uint instIndex = 0;
	_uint prevCode = 0;

	m_tTextDesc.vecInstanceDescs.resize(text.size());	// 임시로 리사이징. (이만치 정의 안해줬어도 일단 보이게?)

	for (auto ch : text)
	{
		if (ch == L'\n')
		{
			penX = 0.f;
			penY += pFont->iPixelHeight * m_tTextDesc.fScale;
			prevCode = 0;
			continue;
		}

		_int advanceX = 0;
		const FTCUSTOM_FONT_GLYPH* pGlyph =
			m_pGameInstance->Get_GlyphAndAdvance(m_tTextDesc.strFontTag, ch, prevCode, advanceX);

		if (!pGlyph)
			continue;
		if (instIndex >= m_tTextDesc.vecInstanceDescs.size())
			break;

		auto& inst = m_tTextDesc.vecInstanceDescs[instIndex++];

		// UV 좌표 설정 (아틀라스에서 잘라낼 위치)
		inst.vSInstCoordX = { pGlyph->fU0, pGlyph->fU1 };
		inst.vSInstCoordY = { pGlyph->fV0, pGlyph->fV1 };


		// ksta : 크기 설정!!!!!!!!!!!!!!!!!!!!!!!!!!!
		inst.vSInstRight	=	{ m_tTextDesc.fScale * pGlyph->sWidth, 0.f, 0.f ,0.f };
		inst.vSInstUp		=	{ 0.f, m_tTextDesc.fScale * pGlyph->sHeight, 0.f ,0.f };
		inst.vSInstLook		=	{ 0.f, 0.f, 1.f ,0.f };

		// 화면 좌표 (기준 위치 + bearing + 현재 pen 이동량)
		inst.vSInstTrans.x = m_tTextDesc.vScreenPos.x
			+ penX
			+ (_float)pGlyph->sOffsetX * m_tTextDesc.fScale;

		inst.vSInstTrans.y = m_tTextDesc.vScreenPos.y
			- (_float)pGlyph->sOffsetY * m_tTextDesc.fScale
			+ penY;


		// 다음 글자 위치 커서 이동 (커닝 반영된 advanceX 사용)
		penX += (_float)advanceX * m_tTextDesc.fScale;
		prevCode = ch;
	}


	// m_tUIDesc 갱신 (부모에서 사용)

	m_tUIDesc.vecInstanceDescs = m_tTextDesc.vecInstanceDescs;

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
