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
	vector<HRESULT> vecHr;

	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorR_PartyFrame")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorB_Status")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorA")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorRB_SkillIcons")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorT_BossStatus")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0));

	for (auto hr : vecHr)
		if (hr == E_FAIL) return E_FAIL;

	return S_OK;
}

HRESULT CUI_ControlHelper::HUD_FadeIn()
{
	vector<HRESULT> vecHr;

	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorR_PartyFrame")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorB_Status")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorA")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorRB_SkillIcons")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1));
	vecHr.push_back(static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorT_BossStatus")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1));

	for (auto hr : vecHr)
		if (hr == E_FAIL) return E_FAIL;

	return S_OK;
}

void CUI_ControlHelper::Render_InteractUI(_wstring strText)
{
	_uint iDestLevel = m_pGameInstance->Get_CurrentLevel();
	CUI_Button_Interact* pInteractBtn = nullptr;

	m_pGameInstance->Spawn_PoolingObject(L"Pool_Button_Interact", _fmatrix());

	// 글자도 넣어야 함
	// 그냥 만들어만 두고, 커스텀UI로써 이거 하위로 집어넣으면 될 것 같은데? 아닌가?

	// ksta : 마저 제작 필요

	auto attacher = Find_RootUI(L"UI_Interact");
	auto attacherDesc = attacher->Get_UIDesc();
	
	// 이미 텍스트 가지고있음?
	_bool isTextExist = attacher->Find_ChildObject(L"UI_Text_Interact") != nullptr;

	if (!isTextExist)
	{	// 없다 -> 폰트 새로 만들고 넣음.

		CCustom_UI* pFont = m_pGameSystem->Create_FontToScreen(
			_float2{ 300.f, 0.f },
			strText,	// 상호작용 글씨
			TEXT_COLOR_TYPE::TT_NORMAL,
			0.5f,
			L"UI_Text_Interact"
		);

		auto fontDesc = pFont->Get_UIDesc();
		attacherDesc.vecChildNames.push_back(fontDesc.strUIName);
		fontDesc.pParentObject = Find_RootUI(L"UI_Interact");
		pFont->Set_UIDesc(fontDesc);
	}
	else
	{	// 있다 -> 기존 폰트의 텍스트만 변경

		CCustom_UI* pFont = attacher->Find_ChildObject(L"UI_Text_Interact");

		auto& textDesc = static_cast<CUI_Text*>(pFont)->Get_TextUIDesc();
		textDesc.strText = strText;
		static_cast<CUI_Text*>(pFont)->Set_TextUIDesc(textDesc);
	}
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
