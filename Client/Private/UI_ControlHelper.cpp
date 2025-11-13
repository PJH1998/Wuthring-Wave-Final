#include "ClientPch.h"
#include "UI_ControlHelper.h"
#include "GameInstance.h"
#include "GameSystem.h"

#include "Animator_UI.h"

#include "UI_HUD.h"

#include "UI_Button.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "UI_Button_Interact.h"
//#include "UI_Interact.h"


CUI_ControlHelper::CUI_ControlHelper()
	: m_pGameInstance{ CGameInstance::GetInstance() }
	, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameInstance);
	//Safe_AddRef(m_pGameSystem);		// 상호참조 발생
}

HRESULT CUI_ControlHelper::Initialize()
{
	// 컨트롤 할 UI 목록 추가..
	//m_pRootUI_HUD = dynamic_cast<CUI_HUD*>(Find_RootUI(L"UI_HUD"));
	//ASSERT_CRASH(m_pRootUI_HUD);

	return S_OK;
}

CCustom_UI* CUI_ControlHelper::Find_RootUI(_wstring strName)
{
	return dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(strName));
}

CCustom_UI* CUI_ControlHelper::Find_ChildUI(_wstring strRootUIName, _wstring strChildUIName)
{
	return dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(strRootUIName))->Find_ChildObject(strChildUIName);
}

HRESULT CUI_ControlHelper::HUD_FadeOut()
{
	if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorR_PartyFrame")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0)))
		return E_FAIL;

	if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorB_Status")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0)))
		return E_FAIL;

	if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorA")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0)))
		return E_FAIL;

	if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorRB_SkillIcons")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0)))
		return E_FAIL;

	//if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorT_BossStatus")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0)))
	//	return E_FAIL;
}

HRESULT CUI_ControlHelper::HUD_FadeIn()
{
	if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorR_PartyFrame")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1)))
		return E_FAIL;

	if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorB_Status")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1)))
		return E_FAIL;

	if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorA")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1)))
		return E_FAIL;

	if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorRB_SkillIcons")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1)))
		return E_FAIL;

	//if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorT_BossStatus")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1)))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CUI_ControlHelper::HUD_FadeOut_BossHPBar()
{
	if (FAILED(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorT_BossStatus")->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"BossStatus_FadeOut")))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_ControlHelper::HUD_FadeIn_BossHPBar()
{
	if (FAILED(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorT_BossStatus")->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"BossStatus_FadeIn")))
		return E_FAIL;

	return S_OK;
}

void CUI_ControlHelper::HUD_Bind_BossStatus(_wstring strUIBosssName, const _char* pMonsterKey, _float* pCurBossHP, _float* pCurBossSA, _bool* pIsGroggy, _float* pGroggyLeftRatio)
{
	CUI_HUD* pTargetUI = dynamic_cast<CUI_HUD*>(Find_RootUI(L"UI_HUD"));
	pTargetUI->Bind_BossStatus(strUIBosssName, pMonsterKey, pCurBossHP, pCurBossSA, pIsGroggy, pGroggyLeftRatio);
}

void CUI_ControlHelper::HUD_Toggle_BossStatusUI(_bool isOn)
{
	if (isOn)	HUD_FadeIn_BossHPBar();
	else		HUD_FadeOut_BossHPBar();

	CUI_HUD* pTargetUI = dynamic_cast<CUI_HUD*>(Find_RootUI(L"UI_HUD"));
	pTargetUI->Toggle_BossStatusUI(isOn);
}

void CUI_ControlHelper::Render_InteractUI(_wstring strText)
{
	_uint iDestLevel = m_pGameInstance->Get_CurrentLevel();
	CUI_Button_Interact* pInteractBtn = nullptr;

	m_pGameInstance->Spawn_PoolingObject(L"Pool_Button_Interact", _fmatrix());


	auto attacher = Find_RootUI(L"UI_Interact")->Find_ChildObject(L"Root_Interact_Multiplier");

	CCustom_UI* pFont = attacher->Find_ChildObject(L"UI_Text_Interact");

	auto& textDesc = static_cast<CUI_Text*>(pFont)->Get_TextUIDesc();
	textDesc.strText = strText;
	static_cast<CUI_Text*>(pFont)->Set_TextUIDesc(textDesc);
}

_bool CUI_ControlHelper::Get_InteractUI_Feedback(UI_EVENT_TYPE eEventInteractType)
{
	// UI_Interact 가 Root UI, 내부적으로 Interact_Normal 커스텀UI를 통해 엔터/호버 등 이벤트를 처리함
	CCustom_UI* pRootUI = Find_RootUI(L"UI_Interact"); //->Find_ChildObject(L"Interact_Normal");

	if (pRootUI == nullptr || pRootUI->IsActivate() == false)
		return false; // 없는데!

	return pRootUI->Check_OnInteract(L"Interact_Normal", ENUM_CLASS(eEventInteractType), 0);
}

CUI_ControlHelper* CUI_ControlHelper::Create()
{
	CUI_ControlHelper* pInstance = new CUI_ControlHelper();

	if (FAILED(pInstance->Initialize()))
	{
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CUI_ControlHelper::Free()
{
	Safe_Release(m_pGameInstance);
	//Safe_Release(m_pGameSystem);		// 상호참조 발생
	__super::Free();
}
