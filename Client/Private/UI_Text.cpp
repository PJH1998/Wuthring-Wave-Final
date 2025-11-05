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

    Ready_Components(pArg);
	Ready_Events();
	Bind_Description(pArg);

    return S_OK;
}

void CUI_Text::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_Text::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);            // Update Animator_UI Component
}

void CUI_Text::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_Text::Render()
{
    //__super::Render();                      // Binding Shader Variables Continuously.
	if (!m_isActivate)
		return;

	if (m_tUIDesc.isInstance && m_cachedVariantUIDesc.isVariant)
		for (_uint i = 0; i < m_tUIDesc.vecInstanceDescs.size(); i++)
			m_tUIDesc.vecInstanceDescs[i].matExtraData = m_cachedVariantUIDesc.matVariantValues[i];

	m_pAnimator_UICom->Render();    // Updates Shader Keyframe Variables.

	if (m_pShaderCom) 
	{
		if (m_tUIDesc.isInstance && m_cachedVariantUIDesc.isVariant)        // 짬통 UI용. 어떤 유형의 UI에 쓸 건지의 Flag를 전역으로 던진다.
			if (FAILED(m_pShaderCom->Bind_Value("g_iVariantFlag", &m_cachedVariantUIDesc.iShaderFlag, sizeof(m_cachedVariantUIDesc.iShaderFlag))))
				CRASH("Binding_Value_Failed");

		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
			CRASH("Binding_Matrix_Failed");

		if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
			CRASH("Binding_Matrix_Failed");
		if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
			CRASH("Binding_Matrix_Failed");

		if (FAILED(m_pShaderCom->Bind_Texture("g_Texture", m_SRVs[0])))			// 텍스쳐는 아까 받아온 SRV를 던진다.
			CRASH("Binding_Matrix_Failed");
		if (FAILED(m_pShaderCom->Bind_Value("g_InverseScreenDiscard", &m_tUIDesc.isInverseScreenDiscard, sizeof(m_tUIDesc.isInverseScreenDiscard))))
			CRASH("Binding_Value_Failed");
		if (FAILED(m_pShaderCom->Bind_Value("g_CutoutAlphaDiscard", &m_tUIDesc.fCutout, sizeof(m_tUIDesc.fCutout))))
			CRASH("Binding_Value_Failed");

		// �̹��� ũ�� �Ѱ��ֱ�
		if (FAILED(m_pShaderCom->Bind_Value("g_ImageSize", &m_tUIDesc.vecSize[m_iCurTexIndex], sizeof(m_tUIDesc.vecSize[m_iCurTexIndex]))))
			CRASH("Binding_Value_Failed");
		if (FAILED(m_pShaderCom->Bind_Value("g_SectorBorder", &m_tUIDesc.vSectorBorder, sizeof(m_tUIDesc.vSectorBorder))))
			CRASH("Binding_Value_Failed");
		if (FAILED(m_pShaderCom->Bind_Value("g_UIScale", &m_tUIDesc.fUIScale, sizeof(m_tUIDesc.fUIScale))))
			CRASH("Binding_Value_Failed");


		m_pShaderCom->Begin(m_tUIDesc.iPassType);
		m_pVIBufferCom->Bind_Resources();
		m_pVIBufferCom->Render();
	}
}

HRESULT CUI_Text::Ready_Components(void* pArg)
{
	// 텍스쳐 컴포넌트를 사용하지 않고, 직접 폰트매니저로부터 제작된 원본 SRV를 받아와 사용한다.
	ASSERT_CRASH(pArg);
	TEXT_UI_DESC* pDesc = static_cast<TEXT_UI_DESC*>(pArg);

	const	_wstring	strFilePath = pDesc->strFilePath;
	const	_wstring	zstrFileName = pDesc->strFileName;
	const	_uint		iNumFiles = pDesc->iNumFiles;
	const   _uint       iDestLevel = m_pGameInstance->Get_CurrentLevel();
	const   _bool       isInstance = pDesc->isInstance;

	const	_wstring	strFontTag = pDesc->strFontTag;

	// 1. Font Manager 로부터 SRV를 받아옴. 이후 Bind Texture 할 때에 SRV를 전달해줌.
	FTCUSTOM_FONT* pFont = m_pGameInstance->Find_Font(pDesc->strFontTag);
	if (!pFont) return;
	m_SRVs.push_back(pFont->pAtlasSRV);
	
	// 2. 폰트 글자 하나마다를 조작해야 하기 때문에 인스턴스용을 사용함.
	if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex_Instance"),
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
