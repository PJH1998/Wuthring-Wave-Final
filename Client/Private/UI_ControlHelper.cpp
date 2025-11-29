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
#include "UI_LockOn.h"
#include "UI_Parry.h"
#include "UI_MobHPBar.h"
#include "UI_TabUtility.h"
#include "UI_Ovfl_Palette.h"
#include "UI_GrafflePoint.h"
#include "UI_QTE.h"

CUI_ControlHelper::CUI_ControlHelper()
	: m_pGameInstance{ CGameInstance::GetInstance() }
	, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pGameSystem);		// 상호참조 발생
}

HRESULT CUI_ControlHelper::Initialize()
{
	// 컨트롤 할 UI 목록 추가..
	//m_pRootUI_HUD = dynamic_cast<CUI_HUD*>(Find_RootUI(L"UI_HUD"));
	//ASSERT_CRASH(m_pRootUI_HUD);



	return S_OK;
}

void CUI_ControlHelper::PreAssign_TargetUIs()
{
	// HUD
	m_pRootUI_HUD					= Find_RootUI (L"UI_HUD");
	m_pUI_HUD_SectorR_PartyFrame	= Find_ChildUI(L"UI_HUD", L"SectorR_PartyFrame");
	m_pUI_HUD_SectorB_Status		= Find_ChildUI(L"UI_HUD", L"SectorB_Status");
	m_pUI_HUD_SectorA				= Find_ChildUI(L"UI_HUD", L"SectorA");
	m_pUI_HUD_SectorRB_SkillIcons	= Find_ChildUI(L"UI_HUD", L"SectorRB_SkillIcons");
	m_pUI_HUD_SectorT_BossStatus	= Find_ChildUI(L"UI_HUD", L"SectorT_BossStatus");

	// Props
	m_pRootUI_Interact				= Find_RootUI (L"UI_Interact");
	m_pTextUI_Interact				= Find_ChildUI(L"UI_Interact", L"UI_Text_Interact");
	m_pUI_Interact_Normal			= Find_ChildUI(L"UI_Interact", L"Interact_Normal");
	m_pUI_Interact_Multiplier		= Find_ChildUI(L"UI_Interact", L"Root_Interact_Multiplier");
	m_pUI_Interact_Pressed			= Find_ChildUI(L"UI_Interact", L"Interact_Pressed");
	m_pUI_Interact_Focused			= Find_ChildUI(L"UI_Interact", L"Interact_Focused");
	m_pRootUI_LockOn				= Find_RootUI (L"UI_LockOn");
	m_pRootUI_Parry					= Find_RootUI (L"UI_Parry");
	m_pRootUI_MobHPBar				= Find_RootUI (L"UI_MobHPBar");
	m_pRootUI_TabUtility			= Find_RootUI (L"UI_TabUtility");
	m_pRootUI_GrafflePoint			= Find_RootUI (L"UI_GrafflePoint");
	//m_pRootUI_QTE					= Find_RootUI (L"UI_QTE");

	// MiniGames
	m_pRootUI_Ovfl_Palette			= Find_RootUI (L"UI_Ovfl_Palette");
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
	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorR_PartyFrame->Get_Component(L"Com_Animator_UI"))->Change_Animation(0)))
		return E_FAIL;

	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorB_Status->Get_Component(L"Com_Animator_UI"))->Change_Animation(0)))
		return E_FAIL;

	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorA->Get_Component(L"Com_Animator_UI"))->Change_Animation(0)))
		return E_FAIL;

	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorRB_SkillIcons->Get_Component(L"Com_Animator_UI"))->Change_Animation(0)))
		return E_FAIL;

	//if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorT_BossStatus")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0)))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CUI_ControlHelper::HUD_FadeIn()
{
	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorR_PartyFrame->Get_Component(L"Com_Animator_UI"))->Change_Animation(1)))
		return E_FAIL;

	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorB_Status->Get_Component(L"Com_Animator_UI"))->Change_Animation(1)))
		return E_FAIL;

	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorA->Get_Component(L"Com_Animator_UI"))->Change_Animation(1)))
		return E_FAIL;

	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorRB_SkillIcons->Get_Component(L"Com_Animator_UI"))->Change_Animation(1)))
		return E_FAIL;

	//if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorT_BossStatus")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1)))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CUI_ControlHelper::HUD_FadeOut_BossHPBar()
{
	if (FAILED(static_cast<CAnimator_UI*>(m_pUI_HUD_SectorT_BossStatus->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"BossStatus_FadeOut")))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_ControlHelper::HUD_FadeIn_BossHPBar()
{
	if (FAILED(static_cast<CAnimator_UI*>(m_pUI_HUD_SectorT_BossStatus->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"BossStatus_FadeIn")))
		return E_FAIL;

	return S_OK;
}

void CUI_ControlHelper::HUD_Bind_BossStatus(_wstring strUIBosssName, const _char* pMonsterKey, _float* pCurBossHP, _float* pCurBossSA, _bool* pIsGroggy, _float* pGroggyLeftRatio)
{
	CUI_HUD* pTargetUI = dynamic_cast<CUI_HUD*>(m_pRootUI_HUD);
	pTargetUI->Bind_BossStatus(strUIBosssName, pMonsterKey, pCurBossHP, pCurBossSA, pIsGroggy, pGroggyLeftRatio);
}

void CUI_ControlHelper::HUD_Toggle_BossStatusUI(_bool isOn)
{
	if (isOn)	HUD_FadeIn_BossHPBar();
	else		HUD_FadeOut_BossHPBar();

	CUI_HUD* pTargetUI = dynamic_cast<CUI_HUD*>(m_pRootUI_HUD);
	pTargetUI->Toggle_BossStatusUI(isOn);
}

void CUI_ControlHelper::Show_InteractUI(_wstring strText)
{
	// 상호작용을 생성하며, 해당 상호작용의 텍스트 자식을 찾아 출력할 텍스트를 변경함
	_uint iDestLevel = m_pGameInstance->Get_CurrentLevel();

	m_pGameInstance->Spawn_PoolingObject(L"Pool_Button_Interact", _fmatrix());


	auto attacher = m_pUI_Interact_Multiplier;

	CCustom_UI* pFont = m_pTextUI_Interact;

	auto& textDesc = static_cast<CUI_Text*>(pFont)->Get_TextUIDesc();
	textDesc.strText = strText;
	static_cast<CUI_Text*>(pFont)->Set_TextUIDesc(textDesc);
}

void CUI_ControlHelper::Hide_InteractUI(_bool isPressedAs)
{
	// 즉시 제거되는 것이 아닌, 끄도록 요청함. 내부적으로 사라지는 애니메이션 거친 뒤 비활성화됨.
	CUI_Button_Interact* pInteractBtn = dynamic_cast<CUI_Button_Interact*>(m_pRootUI_Interact);
	if (pInteractBtn)
	{
		if (isPressedAs)
		{
			m_pUI_Interact_Pressed->SetActivate(true);
			static_cast<CAnimator_UI*>(m_pUI_Interact_Pressed->Get_Component(L"Com_Animator_UI"))
				->Change_Animation(L"Interact_Pressed_Trigger", true);

			m_pUI_Interact_Focused->SetActivate(true);
			static_cast<CAnimator_UI*>(m_pUI_Interact_Focused->Get_Component(L"Com_Animator_UI"))
				->Change_Animation(L"Interact_Focused_On", true);
		}

		pInteractBtn->Req_OffInteract();
	}
}

_bool CUI_ControlHelper::Get_InteractUI_Feedback(UI_EVENT_TYPE eEventInteractType)
{
	// UI_Interact 가 Root UI, 내부적으로 Interact_Normal 커스텀UI를 통해 엔터/호버 등 이벤트를 처리함
	CCustom_UI* pRootUI = m_pRootUI_Interact; //->Find_ChildObject(L"Interact_Normal");

	if (pRootUI == nullptr || pRootUI->IsActivate() == false)
		return false; // 없는데!

	return m_pUI_Interact_Normal->Check_OnInteract(ENUM_CLASS(eEventInteractType), 0);
}

void CUI_ControlHelper::Attach_LockOnUI(_float3* pTargetPos)
{
	CCustom_UI* pRootUI = m_pRootUI_LockOn;

	if (!pRootUI)
		return; 

	// 풀링으로부터 꺼내기
	CUI_LockOn::UI_LOCKON_DESC tDesc = { pTargetPos };
	m_pGameInstance->Spawn_PoolingObject(L"Pool_Button_LockOn", _fmatrix(), &tDesc);
}
void CUI_ControlHelper::Detach_LockOnUI()
{
	CCustom_UI* pRootUI = m_pRootUI_LockOn;

	if (!pRootUI)
		return;

	pRootUI->SetActivate(false);
}

void CUI_ControlHelper::Attach_Parry(_float3* pTargetPos)
{
	CCustom_UI* pRootUI = m_pRootUI_Parry;

	if (!pRootUI)
		return;

	CUI_Parry::UI_PARRY_DESC tDesc = { pTargetPos };

	m_pGameInstance->Spawn_PoolingObject(L"Pool_Image_Parry", _fmatrix(), &tDesc);
}

void CUI_ControlHelper::Enable_Parried()
{
	CCustom_UI* pRootUI = m_pRootUI_Parry;

	if (!pRootUI)
		return;

	static_cast<CUI_Parry*>(pRootUI)->Enable_Parried();
}

void CUI_ControlHelper::Update_MobStatus(const UI_MOBINFO_DESC& tDesc)
{
	CCustom_UI* pRootUI = m_pRootUI_MobHPBar;

	if (!pRootUI)	// 만들어진 적 없으면 리턴
		return;

	if (pRootUI && !pRootUI->IsActivate())	// 풀링 꺼져있으면 켬
		m_pGameInstance->Spawn_PoolingObject(L"Pool_Image_MobHPBar", _fmatrix(), nullptr);

	static_cast<CUI_MobHPBar*>(pRootUI)->Update_MobStatus(tDesc);
}

void CUI_ControlHelper::Show_TabUtilityUI(_uint iCurSelectedUtilityIndex)
{
	CCustom_UI* pRootUI = m_pRootUI_TabUtility;

	if (!pRootUI)	// 만들어진 적 없으면 리턴
		return;


	CUI_TabUtility::UI_TABUTIL_DESC tDesc = {};
	tDesc.iCharSelectedUtilityIndex = iCurSelectedUtilityIndex;


	if (pRootUI && !pRootUI->IsActivate())	// 풀링 꺼져있으면 켬
		m_pGameInstance->Spawn_PoolingObject(L"Pool_Custom_TabUtility", _fmatrix(), &tDesc);
	else
		pRootUI->Reset(_fmatrix(), &tDesc);
}

_uint CUI_ControlHelper::HideNGet_TabUtilityUI()
{
	CCustom_UI* pRootUI = m_pRootUI_TabUtility;

	if (!pRootUI)
		return ENUM_CLASS(UI_TAB_UTILITY::NOTHING);

	return static_cast<CUI_TabUtility*>(pRootUI)->Req_OffTabUI();
}

void CUI_ControlHelper::Open_Game_OverflowPalette(_uint iTargetLevel)
{
	CCustom_UI* pRootUI = m_pRootUI_Ovfl_Palette;

	if (!pRootUI)
		return;

	CUI_Ovfl_Palette::UI_OVFLPALETTE_DESC tDesc = { iTargetLevel };

	m_pGameInstance->Spawn_PoolingObject(L"Pool_Custom_Ovfl_Palette", _fmatrix(), &tDesc);
}

void CUI_ControlHelper::Close_Game_OverflowPalette()
{
	CCustom_UI* pRootUI = m_pRootUI_Ovfl_Palette;

	if (!pRootUI)
		return;

	static_cast<CUI_Ovfl_Palette*>(pRootUI)->Req_OffPalette();
}

void CUI_ControlHelper::Attach_GrafflePoint(_float3* pTargetPos)
{
	//CCustom_UI* pRootUI = m_pRootUI_GrafflePoint;
	//
	//if (!pRootUI)
	//	return;

	CUI_GrafflePoint::UI_GRAFFLEPOINT_DESC tDesc = { pTargetPos };

	m_pGameInstance->Spawn_PoolingObject(L"Pool_Custom_GrafflePoint", _fmatrix(), &tDesc);
}

void CUI_ControlHelper::Play_QTE(_float2 vSpawnPos)
{
	//CCustom_UI* pRootUI = m_pRootUI_QTE;

	CUI_QTE::UI_QTE_DESC tDesc = { vSpawnPos };

	//if (!pRootUI)
	//	return;

	m_pGameInstance->Spawn_PoolingObject(L"Pool_Image_QTE", _fmatrix(), &tDesc);
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
	__super::Free();
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pGameSystem);		// 상호참조 발생
}
