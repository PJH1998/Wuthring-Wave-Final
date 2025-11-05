#include "ClientPch.h"
#include "UI_ControlHelper.h"
#include "GameInstance.h"

#include "Animator_UI.h"

#include "UI_HUD.h"


CUI_ControlHelper::CUI_ControlHelper()
	: m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CUI_ControlHelper::Initialize()
{
	// 컨트롤 할 UI 목록 추가..
	//m_pRootUI_HUD = dynamic_cast<CUI_HUD*>(Find_RootUI(L"UI_UHD"));
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
	vector<HRESULT> vecHr;

	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_UHD", L"SectorR_PartyFrame")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_UHD", L"SectorB_Status")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_UHD", L"SectorA")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_UHD", L"SectorRB_SkillIcons")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_UHD", L"SectorT_BossStatus")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0));

	for (auto hr : vecHr)
		if (hr == E_FAIL) return E_FAIL;

	return S_OK;
}

HRESULT CUI_ControlHelper::HUD_FadeIn()
{
	vector<HRESULT> vecHr;

	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_UHD", L"SectorR_PartyFrame")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_UHD", L"SectorB_Status")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_UHD", L"SectorA")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_UHD", L"SectorRB_SkillIcons")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_UHD", L"SectorT_BossStatus")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1));

	for (auto hr : vecHr)
		if (hr == E_FAIL) return E_FAIL;

	return S_OK;
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
	__super::Free();
}
