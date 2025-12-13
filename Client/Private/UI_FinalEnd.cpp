#include "ClientPch.h"

#include "UI_FinalEnd.h"
#include "GameSystem.h"
#include "Animator_UI.h"


CUI_FinalEnd::CUI_FinalEnd(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_FinalEnd::CUI_FinalEnd(const CUI_FinalEnd& Prototype)
	: CUI_Image(Prototype)
	, m_pGameSystem (CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CUI_FinalEnd::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_FinalEnd::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	CUI_FinalEnd::Ready_Components(pArg);

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_FinalEnd.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Image_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Image_Play.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Grad_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Grad_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Grad_FadeOut.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Black_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Black_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Black_FadeOut.json",
	};
	Load_Animations(vecAnimFilePaths);

	m_pUIAnim_MainImage	->Change_Animation(L"End_Image_Initialize");
	m_pUIAnim_FadeAll	->Change_Animation(L"End_Black_Initialize");
	m_pUIAnim_FadeGrad	->Change_Animation(L"End_Grad_Initialize");


	m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_FinalEnd", this);


	return S_OK;
}

void CUI_FinalEnd::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_FinalEnd::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;
	if (!m_isStart)
		return;

	Update_AnimOrder(fTimeDelta);

	m_fElapsedTime += fTimeDelta;
	__super::Update(fTimeDelta);
}

void CUI_FinalEnd::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);
}

void CUI_FinalEnd::Render()
{
	if (!m_isActivate)
		return;
}

void CUI_FinalEnd::PreAssign_ChildUIs()
{
	m_pRUI_All		= Find_ChildObject(L"Sub_All");

	m_pUI_MainImage	= Find_ChildObject(L"SectorA_Image");
	m_pUI_FadeAll	= Find_ChildObject(L"SectorA_FadeAll");
	m_pUI_FadeGrad	= Find_ChildObject(L"SectorA_FadeGrad");

	m_pUIAnim_MainImage	= dynamic_cast<CAnimator_UI*>(m_pUI_MainImage	->Get_Component(L"Com_Animator_UI"));
	m_pUIAnim_FadeAll	= dynamic_cast<CAnimator_UI*>(m_pUI_FadeAll		->Get_Component(L"Com_Animator_UI"));
	m_pUIAnim_FadeGrad	= dynamic_cast<CAnimator_UI*>(m_pUI_FadeGrad	->Get_Component(L"Com_Animator_UI"));
}

void CUI_FinalEnd::Update_AnimOrder(_float fTimeDelta)
{
	const array<_float, 4> arrKeyframes = { 0.f, 3.f, 6.f, 9.f };

	if		(m_fElapsedTime >= arrKeyframes[0] &&
			m_iAnimOrder == 0)
	{
		m_pUIAnim_FadeAll->Change_Animation(L"End_Black_FadeIn", true);

		m_iAnimOrder++;
	}
	else if (m_fElapsedTime >= arrKeyframes[1] &&
			m_iAnimOrder == 1)
	{
		m_pUIAnim_FadeAll->Change_Animation(L"End_Black_FadeOut", true);
		m_pUIAnim_MainImage->Change_Animation(L"End_Image_Play", true);

		m_iAnimOrder++;
	}
	else if ((m_fElapsedTime >= arrKeyframes[2] &&
			m_iAnimOrder == 2))
	{
		m_pUIAnim_FadeGrad->Change_Animation(L"End_Black_FadeIn", true);

		m_iAnimOrder++;
	}
	else if ((m_fElapsedTime >= arrKeyframes[3] &&
			m_iAnimOrder == 3))
	{
		m_pUIAnim_FadeGrad->Change_Animation(L"End_Grad_FadeIn", true);

		m_iAnimOrder++;
	}
}

HRESULT CUI_FinalEnd::Ready_Components(void* pArg)
{
	return S_OK;
}

void CUI_FinalEnd::Ready_Presets()
{

}

CUI_FinalEnd* CUI_FinalEnd::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_FinalEnd* pInstance = new CUI_FinalEnd(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_FinalEnd");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_FinalEnd::Clone(void* pArg)
{
	CUI_FinalEnd* pInstance = new CUI_FinalEnd(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_FinalEnd");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_FinalEnd::Free()
{
	Safe_Release(m_pGameSystem);

	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_FinalEnd");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
