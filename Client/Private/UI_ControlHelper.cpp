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
#include "UI_GrapplePoint.h"
#include "UI_QTE.h"
#include "UI_HUD_Sector_Minimap.h"
#include "UI_CurveTrace.h"
#include "UI_Dialog.h"
#include "UI_FinalEnd.h"
#include "UI_QuestIndicator.h"


CUI_ControlHelper::CUI_ControlHelper()
	: m_pGameInstance{ CGameInstance::GetInstance() }
	, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pGameSystem);
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

	m_pRootUI_HUD_Minimap			= Find_RootUI(L"UI_HUD_Sector_Minimap");
	m_pUI_UHD_SectorA_Minimap_All	= Find_ChildUI(L"UI_HUD_Sector_Minimap", L"Sub_All");	
	m_pUI_UHD_SectorA_FuncIcons_All	= Find_ChildUI(L"UI_HUD_Sector_FuncIcons", L"Sub_All");	

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
	m_pRootUI_GrapplePoint			= Find_RootUI (L"UI_GrapplePoint");
	//m_pRootUI_QTE					= Find_RootUI (L"UI_QTE");
	m_pRootUI_Dialog				= Find_RootUI (L"UI_Dialog");

	// MiniGames
	m_pRootUI_Ovfl_Palette			= Find_RootUI (L"UI_Ovfl_Palette");

	m_pRootUI_CurveTrace			= Find_RootUI (L"UI_Custom_CurveTrace");
}

CCustom_UI* CUI_ControlHelper::Find_RootUI(_wstring strName)
{
	return dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(strName));
}

CCustom_UI* CUI_ControlHelper::Find_ChildUI(_wstring strRootUIName, _wstring strChildUIName)
{
	return dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(strRootUIName))->Find_ChildObject(strChildUIName);
}

HRESULT CUI_ControlHelper::HUD_FadeOut(_bool isForceChange)
{
	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorR_PartyFrame->Get_Component(L"Com_Animator_UI"))->Change_Animation(0, isForceChange)))
		return E_FAIL;
	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorB_Status->Get_Component(L"Com_Animator_UI"))->Change_Animation(0, isForceChange)))
		return E_FAIL;
	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorA->Get_Component(L"Com_Animator_UI"))->Change_Animation(0, isForceChange)))
		return E_FAIL;
	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorRB_SkillIcons->Get_Component(L"Com_Animator_UI"))->Change_Animation(0, isForceChange)))
		return E_FAIL;

	if (m_pUI_UHD_SectorA_Minimap_All)
		if (FAILED (static_cast<CAnimator_UI*>(m_pUI_UHD_SectorA_Minimap_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"HUD_Minimap_FadeOut", isForceChange)))
			return E_FAIL;
	if (m_pUI_UHD_SectorA_FuncIcons_All)
		if (FAILED (static_cast<CAnimator_UI*>(m_pUI_UHD_SectorA_FuncIcons_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"HUD_FuncIcons_FadeOut", isForceChange)))
			return E_FAIL;


	//if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorT_BossStatus")->Get_Component(L"Com_Animator_UI"))->Change_Animation(0)))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CUI_ControlHelper::HUD_FadeIn(_bool isForceChange)
{
	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorR_PartyFrame->Get_Component(L"Com_Animator_UI"))->Change_Animation(1, isForceChange)))
		return E_FAIL;
	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorB_Status->Get_Component(L"Com_Animator_UI"))->Change_Animation(1, isForceChange)))
		return E_FAIL;
	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorA->Get_Component(L"Com_Animator_UI"))->Change_Animation(1, isForceChange)))
		return E_FAIL;
	if (FAILED (static_cast<CAnimator_UI*>(m_pUI_HUD_SectorRB_SkillIcons->Get_Component(L"Com_Animator_UI"))->Change_Animation(1, isForceChange)))
		return E_FAIL;

	if (m_pUI_UHD_SectorA_Minimap_All)
		if (FAILED(static_cast<CAnimator_UI*>(m_pUI_UHD_SectorA_Minimap_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"HUD_Minimap_FadeIn", isForceChange)))
			return E_FAIL;
	if (m_pUI_UHD_SectorA_FuncIcons_All)
		if (FAILED(static_cast<CAnimator_UI*>(m_pUI_UHD_SectorA_FuncIcons_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"HUD_FuncIcons_FadeIn", isForceChange)))
			return E_FAIL;


	//if (FAILED (static_cast<CAnimator_UI*>(Find_ChildUI(L"UI_HUD", L"SectorT_BossStatus")->Get_Component(L"Com_Animator_UI"))->Change_Animation(1)))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CUI_ControlHelper::HUD_FadeOut_BossHPBar(_bool isForceChange)
{
	if (FAILED(static_cast<CAnimator_UI*>(m_pUI_HUD_SectorT_BossStatus->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"BossStatus_FadeOut", isForceChange)))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_ControlHelper::HUD_FadeIn_BossHPBar(_bool isForceChange)
{
	if (FAILED(static_cast<CAnimator_UI*>(m_pUI_HUD_SectorT_BossStatus->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"BossStatus_FadeIn", isForceChange)))
		return E_FAIL;

	return S_OK;
}

void CUI_ControlHelper::HUD_Bind_BossStatus(_wstring strUIBosssName, const _char* pMonsterKey, _float* pCurBossHP, _float* pCurBossSA, _bool* pIsGroggy, _float* pGroggyLeftRatio)
{
	CUI_HUD* pTargetUI = dynamic_cast<CUI_HUD*>(m_pRootUI_HUD);
	pTargetUI->Bind_BossStatus(strUIBosssName, pMonsterKey, pCurBossHP, pCurBossSA, pIsGroggy, pGroggyLeftRatio);
}

void CUI_ControlHelper::HUD_Toggle_BossStatusUI(_bool isOn, _bool isForceChange)
{
	if (isOn)	HUD_FadeIn_BossHPBar(isForceChange);
	else		HUD_FadeOut_BossHPBar(isForceChange);

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

	
	//auto& textDesc = static_cast<CUI_Text*>(pFont)->Get_TextUIDesc();
	//textDesc.strText = strText;
	//static_cast<CUI_Text*>(pFont)->Set_TextUIDesc(textDesc);
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

			m_pGameInstance->Play_Sound(L"UI_ClickHide", ENUM_CLASS(CHANNEL::UI_INTERACT), 0.5f);
		}

		pInteractBtn->Req_OffInteract();
	}
}

void CUI_ControlHelper::Req_Render_InteractUI(_wstring strText, _bool isPressedAs)
{
	CUI_Button_Interact* pRootUI = dynamic_cast<CUI_Button_Interact*>(m_pRootUI_Interact);

	if (pRootUI == nullptr || pRootUI->IsActivate() == false)
	{
		m_pGameInstance->Spawn_PoolingObject(L"Pool_Button_Interact", _fmatrix());
		return;
	}

	CCustom_UI* pFont = m_pTextUI_Interact;
	auto& textDesc = static_cast<CUI_Text*>(pFont)->Get_TextUIDesc();
	textDesc.strText = strText;
	static_cast<CUI_Text*>(pFont)->Set_TextUIDesc(textDesc);

	if (isPressedAs)
	{
		m_pUI_Interact_Pressed->SetActivate(true);
		static_cast<CAnimator_UI*>(m_pUI_Interact_Pressed->Get_Component(L"Com_Animator_UI"))
			->Change_Animation(L"Interact_Pressed_Trigger", true);

		m_pUI_Interact_Focused->SetActivate(true);
		static_cast<CAnimator_UI*>(m_pUI_Interact_Focused->Get_Component(L"Com_Animator_UI"))
			->Change_Animation(L"Interact_Focused_On", true);

		m_pGameInstance->Play_Sound(L"UI_ClickHide", ENUM_CLASS(CHANNEL::UI_INTERACT), 0.5f);

		pRootUI->Req_OffInteract();
	}

	static_cast<CUI_Button_Interact*>(pRootUI)->Req_Render();
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

	if (pRootUI->IsActivate())
	{
		// 켜져있으면 현재좌표 갱신.
		static_cast<CUI_LockOn*>(pRootUI)->Change_TargetPos(pTargetPos);
	}
	else
	{
		// 꺼져있으면 풀링으로부터 꺼내기
		CUI_LockOn::UI_LOCKON_DESC tDesc = { pTargetPos };
		m_pGameInstance->Spawn_PoolingObject(L"Pool_Button_LockOn", _fmatrix(), &tDesc);
	}


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

	Bind_ObjectPos_PerFrame_ToMinimap(tDesc.vMobPos, UI_MINIMAP_OBJTYPE::MONSTER);

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

//void CUI_ControlHelper::Attach_GrapplePoint(_float3* pTargetPos, UI_GRAPPLE_TYPE eType)
//{
//	// 인스턴싱하는 단일 클래스가 아니기에 rootUI 등록 불가 (중복등록때문)
//	//CCustom_UI* pRootUI = m_pRootUI_GrapplePoint;
//	//
//	//if (!pRootUI)
//	//	return;
//
//
//	CUI_GrapplePoint::UI_GRAPPLEPOINT_DESC tDesc = { pTargetPos, eType };
//
//	m_pGameInstance->Spawn_PoolingObject(L"Pool_Custom_GrapplePoint", _fmatrix(), &tDesc);
//}

void CUI_ControlHelper::Play_QTE(_float2 vSpawnPos, UI_QTE_TYPE eQTEType, UI_QTE_BTN eIconIndex, _float2 vScale)
{
	//CCustom_UI* pRootUI = m_pRootUI_QTE;
	
		
	CUI_QTE::UI_QTE_DESC tDesc;

	tDesc.vSpawnPos = vSpawnPos;
	tDesc.eQTEType = eQTEType;
	tDesc.eIconIndex = eIconIndex;
	tDesc.vSpawnScale = vScale;

	//if (!pRootUI)
	//	return;

	m_pGameInstance->Spawn_PoolingObject(L"Pool_Image_QTE", _fmatrix(), &tDesc);
}

void CUI_ControlHelper::Bind_ObjectPos_PerFrame_ToMinimap(const _float3& vPosition, UI_MINIMAP_OBJTYPE eType)
{
	CUI_HUD_Sector_Minimap* pRootUI = dynamic_cast<CUI_HUD_Sector_Minimap*>(m_pRootUI_HUD_Minimap);

	if (!pRootUI)
		return;

	pRootUI->Bind_ObjectPos_PerFrame(vPosition, eType);
}

void CUI_ControlHelper::Attach_ObjectPos_ToMinimap(const _float3& vPosition, UI_MINIMAP_OBJTYPE eType, void* pOwner)
{
	CUI_HUD_Sector_Minimap* pRootUI = dynamic_cast<CUI_HUD_Sector_Minimap*>(m_pRootUI_HUD_Minimap);

	if (!pRootUI)
		return;

	pRootUI->Attach_ObjectPos(vPosition, eType, pOwner);
}

void CUI_ControlHelper::Detach_ObjectPos_ToMinimap(void* pOwner)
{
	CUI_HUD_Sector_Minimap* pRootUI = dynamic_cast<CUI_HUD_Sector_Minimap*>(m_pRootUI_HUD_Minimap);

	if (!pRootUI)
		return;

	pRootUI->Detach_ObjectPos(pOwner);
}

void CUI_ControlHelper::Req_Render_CurveTrace(	_float3& vStartPos,
												_float3& vStartVelocity,
												_float3& vAcceleration,
												_float3* pCustomSpherePos,
												_float fMaxTime,
												_uint iSegmentCount,
												_float fRibbonWidth,
												_bool isUseCustomColor,
												_float4 vBaseColor,
												_float4 vHeadColor,
												_float4 vTailColor)
{
	CUI_CurveTrace* pRootUI = dynamic_cast<CUI_CurveTrace*>(m_pRootUI_CurveTrace);

	if (pRootUI == nullptr)
		return;
	if (pRootUI->IsActivate() == false)
	{
		CUI_CurveTrace::UI_CURVETRACE_DESC tDesc = {};
		tDesc.vStartPos = vStartPos;// _float3(0.f, 0.f, 0.f);
		tDesc.vStartVel = vStartVelocity;// _float3(0.f, 10.f, 10.f);
		tDesc.vAcceleration = vAcceleration;// _float3(0.f, -9.8f, 0.f);
		tDesc.pCustomSpherePos = pCustomSpherePos;
		tDesc.fMaxTime = fMaxTime;
		tDesc.iSegmentCount = iSegmentCount;
		tDesc.fWidth = fRibbonWidth;
		tDesc.isUseCustomColor = isUseCustomColor;
		tDesc.vBaseColor = vBaseColor;
		tDesc.vHeadColor = vHeadColor;
		tDesc.vTailColor = vTailColor;

		_vector vPos = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		_vector vSca = XMVectorSet(2.f, 2.f, 2.f, 1.f);
		_matrix matPos = XMMatrixScalingFromVector(vSca) * XMMatrixTranslationFromVector(vPos);
		m_pGameInstance->Spawn_PoolingObject(L"Pool_Custom_CurveTrace", matPos, &tDesc);

		return;
	}
	else
	{
		pRootUI->Req_Render_CurveTrace(	vStartPos,
										vStartVelocity,
										vAcceleration,
										pCustomSpherePos,
										fMaxTime,
										iSegmentCount,
										fRibbonWidth,
										isUseCustomColor,
										vBaseColor,
										vHeadColor,
										vTailColor);
	}
}

void CUI_ControlHelper::Open_DialogUI(const _char* pFilePath)
{
	CUI_Dialog* pRootUI = dynamic_cast<CUI_Dialog*>(m_pRootUI_Dialog);

	if (!pRootUI)
		return;
	if (pRootUI->IsActivate())
		return;

	CUI_Dialog::UI_DIALOG_DESC tDesc = {};
	tDesc.strFilePath = pFilePath;

	m_pGameInstance->Spawn_PoolingObject(L"Pool_Custom_Dialog", _matrix(), &tDesc);
}

void CUI_ControlHelper::Req_Interact_DialogUI(_bool isChangeNext_Forcely)
{
	CUI_Dialog* pRootUI = dynamic_cast<CUI_Dialog*>(m_pRootUI_Dialog);

	if (!pRootUI)
		return;
	if (pRootUI->IsActivate() == false)
		return;

	if (!isChangeNext_Forcely)		
		pRootUI->Req_InteractExternally();
	else							
	{
		//if (pRootUI->Get_isFinished_CurDialog() &&
		//	pRootUI->Get_isLast_CurDialog())			pRootUI->Req_Close_Dialog();
		//else if (pRootUI->Get_isFinished_CurDialog())	pRootUI->Req_Next_Dialog();
		//else											pRootUI->Req_Finish_CurDialog();
		
		if (pRootUI->Get_isLast_CurDialog())
		{
			pRootUI->Req_Close_Dialog();
			return;
		}
		else 											
		{
			pRootUI->Req_Next_Dialog();
			return;
		}
	}
}

void CUI_ControlHelper::Close_DialogUI()
{
	CUI_Dialog* pRootUI = dynamic_cast<CUI_Dialog*>(m_pRootUI_Dialog);

	if (!pRootUI)
		return;

	pRootUI->Req_Close_Dialog();
}

void CUI_ControlHelper::Trigger_PlayEndImage()
{
	CUI_FinalEnd* pRootUI = dynamic_cast<CUI_FinalEnd*>(Find_RootUI(L"UI_FinalEnd"));

	if (!pRootUI)
		return;

	pRootUI->Trigger_PlayEndImage(true);
}

#ifdef _DEBUG
void CUI_ControlHelper::Trigger_StopEndImageForcely()
{
	CUI_FinalEnd* pRootUI = dynamic_cast<CUI_FinalEnd*>(Find_RootUI(L"UI_FinalEnd"));

	if (!pRootUI)
		return;

	pRootUI->Trigger_PlayEndImage(false);
}
#endif // _DEBUG

void CUI_ControlHelper::Trigger_ActivateQuest()
{
	CUI_QuestIndicator* pRootUI = dynamic_cast<CUI_QuestIndicator*>(Find_RootUI(L"UI_QuestIndicator"));

	if (!pRootUI)
		return;

	pRootUI->Trigger_ActivateQuest();
}

void CUI_ControlHelper::Trigger_AddQuestProgress()
{
	CUI_QuestIndicator* pRootUI = dynamic_cast<CUI_QuestIndicator*>(Find_RootUI(L"UI_QuestIndicator"));

	if (!pRootUI)
		return;

	pRootUI->Trigger_AddQuestProgress();
}

#ifdef _DEBUG
void CUI_ControlHelper::Trigger_AllReset()
{
	CUI_QuestIndicator* pRootUI = dynamic_cast<CUI_QuestIndicator*>(Find_RootUI(L"UI_QuestIndicator"));

	if (!pRootUI)
		return;

	pRootUI->Trigger_AllReset();
}
#endif // _DEBUG



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
	Safe_Release(m_pGameSystem);
}
