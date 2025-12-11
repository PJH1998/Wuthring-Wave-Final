#include "ClientPch.h"
#include "GameSystem.h"

#include "Parser.h"
#include "Factory.h"
#include "Director.h"
#include "PlayerStatus.h"
#include "Player.h"

#include "UI_FontPreset.h"
#include "UI_ControlHelper.h"
#include "UI_GrappleController.h"
#include "UI_StatusSyncer.h"

#include"Sonoro_Manager.h"

#include "MonsterTable.h"

#include "MouseController.h"
#include "Player.h"
#include "SequencePlayer.h"
#include"Potal.h"
#include "TimeLack.h"

IMPLEMENT_SINGLETON(CGameSystem)

CGameSystem::CGameSystem()
{
}

void CGameSystem::Ready_GameSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	m_pParser = CParser::Create(pDevice, pContext);
	ASSERT_CRASH(m_pParser);

	m_pFactory = CFactory::Create(pDevice, pContext);
	ASSERT_CRASH(m_pFactory);


	m_pUI_FontPreset = CUI_FontPreset::Create();
	ASSERT_CRASH(m_pUI_FontPreset);

	m_pUI_ControlHelper = CUI_ControlHelper::Create();
	ASSERT_CRASH(m_pUI_ControlHelper);

	m_pUI_GrappleController = CUI_GrappleController::Create();
	ASSERT_CRASH(m_pUI_GrappleController);

	//m_pUI_StatusSyncer = CUI_StatusSyncer::Create();
	//ASSERT_CRASH(m_pUI_ControlHelper);

	m_pDirector = CDirector::Create();
	ASSERT_CRASH(m_pDirector);

	m_pSonoro_Manager = CSonoro_Manager::Create();
	ASSERT_CRASH(m_pSonoro_Manager);

	m_pMonsterTable = CMonsterTable::Create();
	ASSERT_CRASH(m_pMonsterTable);

	m_pMouseController = CMouseController::Create();
	ASSERT_CRASH(m_pMouseController);
	
	m_pTimeLack = CTimeLack::Create();
	ASSERT_CRASH(m_pTimeLack);

	// 파일 목록 만들기.
	vector<_string> AbilityFolders = {};
	AbilityFolders.resize(CPlayer::CHARACTERTYPE::TYPE_END);

	AbilityFolders[CPlayer::CHARACTERTYPE::ROVER] = "../Bin/Resource/Model/Player/Rover/Ability/";
	AbilityFolders[CPlayer::CHARACTERTYPE::AUGUSTA] = "../Bin/Resource/Model/Player/Augusta/Ability/";
	AbilityFolders[CPlayer::CHARACTERTYPE::GALBRENA] = "../Bin/Resource/Model/Player/Galbrena/Ability/";
	m_pPlayerStatus = CPlayerStatus::Create(pDevice, pContext, AbilityFolders);
}

void CGameSystem::Update(_float fTimeDelta)
{
	m_pSonoro_Manager->Update(fTimeDelta);
}

void CGameSystem::Clear_Resource()
{
	m_pDirector->Clear_Action();
	m_pMonsterTable->Clear_NPCData();
	m_pSonoro_Manager->Clear_Resource();
	Clear_TriggerCallBack();
	Safe_Release(m_pPlayer);
	Safe_Release(m_pPotal);
	m_pPotal = nullptr;
}
#pragma region PARSER
const vector<vector<_string>>& CGameSystem::Load_CSV(const _char* pFilePath)
{
	return m_pParser->Load_CSV(pFilePath);
}

void CGameSystem::Load_Sequence(const _char* pFolderPath)
{
	m_pParser->Load_Sequence(pFolderPath);
}

void CGameSystem::Ready_Prototype_Map(const _char* pDataFilePath, LEVEL eLevel, const _char* pModelFilePath)
{
	return m_pParser->Ready_Prototype_Map(pDataFilePath, eLevel, pModelFilePath);
}

void CGameSystem::Clone_MapObjects(LEVEL eLevel)
{
	m_pParser->Clone_MapObjects(eLevel);
}
void CGameSystem::Clone_Spawners(LEVEL eLevel)
{
	m_pParser->Clone_Spawners(eLevel);
}
void CGameSystem::Create_MapEffects()
{
	m_pParser->Create_MapEffect();
}
#pragma endregion

void CGameSystem::Create_Effect(const string& strFolderPath, LEVEL eLevel)
{
	return m_pParser->Create_Effect(strFolderPath, eLevel);
}

void CGameSystem::Create_Prefab(const string& strFolderPath, LEVEL eLevel, _int PoolingNum)
{
	return m_pParser->Create_Prefab(strFolderPath, eLevel, PoolingNum);
}

void CGameSystem::Load_EffectTexture_FromFolder(const string& strFolderPath, LEVEL eLevel)
{
	return m_pParser->Load_EffectTexture_FromFolder(strFolderPath, eLevel);
}

void CGameSystem::Load_EffectMeshDat_FromFolder(const string& strFolderPath, LEVEL eLevel)
{
	return m_pParser->Load_EffectMeshDat_FromFolder(strFolderPath, eLevel);
}

void CGameSystem::Load_EffectDecalData_FromFolder(const string& strFolderPath)
{
	return m_pParser->Load_FXDecal_Data_FromFolder(strFolderPath);
}

void CGameSystem::Load_EffectVATexture_FromFolder(const string& strFolderPath, LEVEL eLevel)
{
	return m_pParser->Load_EffectVATexture_FromFolder(strFolderPath, eLevel);
}

void CGameSystem::Load_EffectVAMeshDat_FromFolder(const string& strFolderPath, LEVEL eLevel)
{
	return m_pParser->Load_EffectVAMeshDat_FromFolder(strFolderPath, eLevel);
}

void CGameSystem::Load_EffectLightData_FromFolder(const string& strFolderPath)
{
	return m_pParser->Load_FXLight_Data_FromFolder(strFolderPath);
}

void CGameSystem::Load_EffectSpecturmTexture_FromFolder(const string& strFolderPath, LEVEL eLevel)
{
	return m_pParser->Load_EffectSpectrumTexture_FromFolder(strFolderPath, eLevel);
}

void CGameSystem::Load_EffectSpectrumVB_FromFolder(const string& strFolderPath, LEVEL eLevel)
{
	return m_pParser->Load_Spectrum_VB_FromFolder(strFolderPath, eLevel);
}

void CGameSystem::Create_Spertrum(const string& strFolderPath, LEVEL eLevel, _uint PoolingNum)
{
	return m_pParser->Create_Spectrum(strFolderPath, eLevel, PoolingNum);
}


#pragma region FACTORY

void CGameSystem::Create_MonsterDummy(LEVEL eLayerLevel, _float3 vPos, const _fmatrix& PreTransformationMatrix)
{
	m_pFactory->Create_MonsterDummy(eLayerLevel, vPos, PreTransformationMatrix);
}
#pragma endregion

#pragma region DIRECTOR
void CGameSystem::Add_Action(const _char* pFolderPath)
{
	m_pDirector->Add_Action(pFolderPath);
}
void CGameSystem::Play_Action(const _wstring& strActionTag, const _fmatrix& WorldMatrix, _bool isMaintain, _bool isEscape)
{
	m_pDirector->Play_Action(strActionTag, WorldMatrix, isMaintain, isEscape);
}
void CGameSystem::Stop_Action()
{
	m_pDirector->Stop_Action();
}
#pragma endregion

#pragma region CHARACTER INFO


#pragma endregion

void CGameSystem::Render_Damage(_float4 vTargetPos, _int iDamage, TEXT_COLOR_TYPE eColorType, _float fSpawnRange)
{
	m_pUI_FontPreset->Render_Damage(vTargetPos, to_wstring(iDamage), eColorType, fSpawnRange);
}

void CGameSystem::Render_Damage(_float4 vTargetPos, _wstring strText, TEXT_COLOR_TYPE eColorType, _float fSpawnRange)
{
	m_pUI_FontPreset->Render_Damage(vTargetPos, strText, eColorType, fSpawnRange);
}

CUI_Text* CGameSystem::Create_FontToScreen(_float2 vScreenPos, _wstring strText, TEXT_COLOR_TYPE eColorType, _float fFontScale, _wstring strUIName, _wstring strFontTag)
{
	return m_pUI_FontPreset->Create_FontToScreen(vScreenPos, strText, eColorType, fFontScale, strUIName, strFontTag);
}

CUI_Text* CGameSystem::Create_FontToScreen_Alpha(_float2 vScreenPos, _wstring strText, TEXT_COLOR_TYPE eColorType, _float fFontScale, _wstring strUIName, _wstring strFontTag)
{
	return m_pUI_FontPreset->Create_FontToScreen_Alpha(vScreenPos, strText, eColorType, fFontScale, strUIName, strFontTag);
}

void CGameSystem::PreAssign_TargetUIs()
{
	return m_pUI_ControlHelper->PreAssign_TargetUIs();
}

CCustom_UI* CGameSystem::Find_RootUI(_wstring strName)
{
	return m_pUI_ControlHelper->Find_RootUI(strName);
}

CCustom_UI* CGameSystem::Find_ChildUI(_wstring strRootUIName, _wstring strChildUIName)
{
	return m_pUI_ControlHelper->Find_ChildUI(strRootUIName, strChildUIName);
}

HRESULT CGameSystem::HUD_FadeOut(_bool isForceChange)
{
	return m_pUI_ControlHelper->HUD_FadeOut(isForceChange);
}

HRESULT CGameSystem::HUD_FadeIn(_bool isForceChange)
{
	return m_pUI_ControlHelper->HUD_FadeIn(isForceChange);
}

//HRESULT CGameSystem::HUD_FadeOut_BossHPBar()
//{
//	return m_pUI_ControlHelper->HUD_FadeOut_BossHPBar();
//}
//HRESULT CGameSystem::HUD_FadeIn_BossHPBar()
//{
//	return m_pUI_ControlHelper->HUD_FadeIn_BossHPBar();
//}

void CGameSystem::HUD_Bind_BossStatus(_wstring strUIBosssName, const _char* pMonsterKey, _float* pCurBossHP, _float* pCurBossSA, _bool* pIsGroggy, _float* pGroggyLeftRatio)
{
	return m_pUI_ControlHelper->HUD_Bind_BossStatus(strUIBosssName, pMonsterKey, pCurBossHP, pCurBossSA, pIsGroggy, pGroggyLeftRatio);
}

void CGameSystem::HUD_Toggle_BossStatusUI(_bool isOn, _bool isForceChange)
{
	return m_pUI_ControlHelper->HUD_Toggle_BossStatusUI(isOn, isForceChange);
}	

//void CGameSystem::Toggle_InteractUI(_bool isOn, _wstring strText)
//{
//	m_pUI_ControlHelper->Toggle_InteractUI(isOn, strText);++
//}

void CGameSystem::Show_InteractUI(_wstring strText)
{
	m_pUI_ControlHelper->Show_InteractUI(strText);
}

void CGameSystem::Hide_InteractUI(_bool isPressedAs)
{
	m_pUI_ControlHelper->Hide_InteractUI(isPressedAs);
}

void CGameSystem::Req_Render_InteractUI(_wstring strText, _bool	isPressedAs)
{
	m_pUI_ControlHelper->Req_Render_InteractUI(strText, isPressedAs);
}

_bool CGameSystem::Get_InteractUI_Feedback(UI_EVENT_TYPE eEventInteractType)
{
	return m_pUI_ControlHelper->Get_InteractUI_Feedback(eEventInteractType);
}

void CGameSystem::Attach_LockOnUI(_float3* pTargetPos)
{
	m_pUI_ControlHelper->Attach_LockOnUI(pTargetPos);
}

void CGameSystem::Detach_LockOnUI()
{
	m_pUI_ControlHelper->Detach_LockOnUI();
}

void CGameSystem::Attach_Parry(_float3* pTargetPos)
{
	m_pUI_ControlHelper->Attach_Parry(pTargetPos);
}

void CGameSystem::Enable_Parried()
{
	m_pUI_ControlHelper->Enable_Parried();
}

void CGameSystem::Update_MobStatus(const UI_MOBINFO_DESC& tDesc)
{
	m_pUI_ControlHelper->Update_MobStatus(tDesc);
}

void CGameSystem::Show_TabUtilityUI(_uint iCurSelectedUtilityIndex)
{
	m_pUI_ControlHelper->Show_TabUtilityUI(iCurSelectedUtilityIndex);
}

_uint CGameSystem::HideNGet_TabUtilityUI()
{
	return m_pUI_ControlHelper->HideNGet_TabUtilityUI();
}

void CGameSystem::Open_Game_OverflowPalette(_uint iTargetLevel)
{
	m_pUI_ControlHelper->Open_Game_OverflowPalette(iTargetLevel);
}

void CGameSystem::Close_Game_OverflowPalette()
{
	m_pUI_ControlHelper->Close_Game_OverflowPalette();
}

//void CGameSystem::Attach_GrapplePoint(_float3* pTargetPos, UI_GRAPPLE_TYPE eType)
//{
//	m_pUI_ControlHelper->Attach_GrapplePoint(pTargetPos, eType);
//}

void CGameSystem::Play_QTE(_float2 vSpawnPos, UI_QTE_TYPE eQTEType, UI_QTE_BTN eIconIndex, _float2 vScale)
{
	m_pUI_ControlHelper->Play_QTE(vSpawnPos, eQTEType, eIconIndex, vScale);
}

void CGameSystem::Bind_ObjectPos_PerFrame_ToMinimap(const _float3& vPosition, UI_MINIMAP_OBJTYPE eType)
{
	m_pUI_ControlHelper->Bind_ObjectPos_PerFrame_ToMinimap(vPosition, eType);
}

void CGameSystem::Attach_ObjectPos_ToMinimap(const _float3& vPosition, UI_MINIMAP_OBJTYPE eType, void* pOwner)
{
	m_pUI_ControlHelper->Attach_ObjectPos_ToMinimap(vPosition, eType, pOwner);
}

void CGameSystem::Detach_ObjectPos_ToMinimap(void* pOwner)
{
	m_pUI_ControlHelper->Detach_ObjectPos_ToMinimap(pOwner);
}

void CGameSystem::Req_Render_CurveTrace(_float3& vStartPos,
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
	m_pUI_ControlHelper->Req_Render_CurveTrace(	vStartPos,
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

void* CGameSystem::Create_GrapplePoint(const _float3& vPointPos, UI_GRAPPLE_TYPE eType, _bool isDisabledOnSpawn)
{
	return m_pUI_GrappleController->Create_GrapplePoint(vPointPos, eType, isDisabledOnSpawn);
}

CUI_GrapplePoint* CGameSystem::Find_NearGrapplePoint(const _float3& vBasePos, UI_GRAPPLE_TYPE eType, _float* pOutDistance, _bool isIncludeInactive)
{
	return m_pUI_GrappleController->Find_NearGrapplePoint(vBasePos, eType, pOutDistance, isIncludeInactive);
}

void CGameSystem::Toggle_GrapplePoint(void* pTargetUIPtr, _bool isActive)
{
	m_pUI_GrappleController->Toggle_GrapplePoint(pTargetUIPtr, isActive);
}




//HRESULT	CGameSystem::Sync_Status_toHUD(CHARACTER_STAT& eStat)
//{
//	return m_pUI_StatusSyncer->Sync_Status_toHUD(eStat);
//}
#pragma region PLAYER STATUS

#pragma endregion


#pragma region TRIGGER

_uint CGameSystem::Get_CurrentCharacterIndex() const
{
	return m_pPlayerStatus->Get_CurrentCharIndex();
}

// Trigger 등록
void CGameSystem::TriggerRegister(_uint iNumTriggerMapIndex, TriggerCallback pFunc)
{
	{
		lock_guard<mutex>lock(m_Mutex);
		m_TriggerEvents[iNumTriggerMapIndex].push_back(pFunc);
	}
}

// Trigger 실행.
void CGameSystem::OnTriggerActivate(_uint iNumTriggerMapIndex, void* pArg)
{
	auto iter = m_TriggerEvents.find(iNumTriggerMapIndex);
	if (iter == m_TriggerEvents.end())
		return;

	for (auto& pTriggerFunc : m_TriggerEvents[iNumTriggerMapIndex])
	{
		pTriggerFunc(pArg);
	}
}
void CGameSystem::Clear_TriggerCallBack()
{
	for (auto& TriggerVector : m_TriggerEvents)
		TriggerVector.second.clear();
	m_TriggerEvents.clear();
}
const _tchar* CGameSystem::Get_SonoroText()
{
	return m_pSonoro_Manager->Get_SonoroText();
}
#pragma endregion


#pragma region SONORO_MANAGER
_bool* CGameSystem::Add_To_Management(OBJECTTYPE eType, CMapObject_Sonoro* pObjects, _bool** SonoroMode)
{
	return m_pSonoro_Manager->Add_To_Management(eType, pObjects, SonoroMode);
}

_bool* CGameSystem::Add_To_Management(OBJECTTYPE eType, CMapObject_NonSonoro* pObjects, _bool** SonoroMode)
{
	return m_pSonoro_Manager->Add_To_Management(eType, pObjects, SonoroMode);
}

_bool* CGameSystem::Add_To_Management(INSTANCETYPE eType, CMapObject_Instance* pObjects, _bool** SonoroMode)
{
	return m_pSonoro_Manager->Add_To_Management(eType, pObjects, SonoroMode);
}

_bool  CGameSystem::Change_Sonoro(_bool IsSonoro)
{
	return m_pSonoro_Manager->Change_Sonoro(IsSonoro);
}
_bool CGameSystem::IsSonoro()
{
	return m_pSonoro_Manager->IsSonoro();
}
#pragma endregion

#pragma region MONSTER_TABLE
HRESULT CGameSystem::LoadMonsterTable(const _char* pFilePath)
{
	return m_pMonsterTable->LoadDataTable(pFilePath);
}
MONSTER_INFO* CGameSystem::Get_MonsterInfo(const _char* pMonsterKey) const
{
	return m_pMonsterTable->Get_MonsterInfo(pMonsterKey);
}
HRESULT CGameSystem::LoadNPCDataTable(const _char* pFilePath, _uint iType)
{
	return m_pMonsterTable->LoadNPCDataTable(pFilePath, iType);;
}
_uint CGameSystem::Get_NumNPCInstance(_uint iType) const
{
	return m_pMonsterTable->Get_NumNPCInstance(iType);
}
const vector<NPCINFO>& CGameSystem::Get_NpcData(_uint iType) const
{
	return m_pMonsterTable->Get_NpcData(iType);
}
#pragma endregion

#pragma region SFX_PREFAB
void CGameSystem::Ready_SFX_Prefab(const _char* pFolderPath, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex)
{
	m_pParser->Ready_SFX_Prefab(pFolderPath, iPrototypeLevelIndex, strPrototypeTag, iLayerLevelIndex);
}
void CGameSystem::Register_Mouse(CMouse* pMouse)
{
	m_pMouseController->Register_Mouse(pMouse);
}
void CGameSystem::Set_MouseFix(_bool isFix)
{
	m_pMouseController->Set_MouseFix(isFix);
}
_bool CGameSystem::IsFix()
{
    return m_pMouseController->IsFix();
}
#pragma endregion

#pragma region GRAB_INTERACT
void CGameSystem::Bind_Condition_ToPlayer(const _string& strTransition, void* pArg)
{
	if (nullptr == m_pPlayer)
		return;

	
	if (strTransition == "LeviatanGrab")
	{
		m_pPlayer->Bind_EventLock(true);
		m_pPlayer->Notify_Event(CHARACTER_EVENT::LEVIATAN_QTE, pArg);
	}
	else if (strTransition == "LeviatanQTEStart")
	{
		_float2 vPos = { 500.f, -200.f };
		Play_QTE(vPos, UI_QTE_TYPE::FILLGUAGE, UI_QTE_BTN::F);
	}
	else if (strTransition == "LeviatanQTESuccess")
	{
		m_pPlayer->Notify_Event(CHARACTER_EVENT::LEVIATAN_QTE_SUCCESS);
	}
	else if (strTransition == "GrabRelease")
	{
		m_pPlayer->Notify_EscapeGrabReady(); // 여기서 탈출애니메이션 실행하고
	}
	else if (strTransition == "GrabUnbined")
	{
		m_pPlayer->Notify_EscapeGrabExecute(); // 여기서 뼈 해제하라.
	}
	else if (strTransition == "Teleport")
	{
		m_pPlayer->Notify_Event(CHARACTER_EVENT::TELEPORT, pArg);
	}
	
	
	
	
}

void CGameSystem::Lock_Input_ToPlayer(_bool IsLock)
{
	if (nullptr == m_pPlayer)
		return;

	m_pPlayer->Lock_Input(IsLock);
}

#pragma endregion

#pragma region PLAYER
void CGameSystem::Register_SequencePlayer(CSequencePlayer* pSequencePlayer)
{
	m_pSequencePlayer = pSequencePlayer;
	Safe_AddRef(m_pSequencePlayer);
}
void CGameSystem::Register_Player(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
	Safe_AddRef(m_pPlayer);
}

_vector CGameSystem::Get_PlayerLookVector()
{
	return m_pPlayer->Get_LookVector();
}

_vector CGameSystem::Get_PlayerPosition()
{
	return m_pPlayer->Get_Position();
}

const _float4x4* CGameSystem::Get_PlayerMatrixPtr()
{
	return m_pPlayer->Get_PlayerMatrixPtr();
}

// 보스 근처에 소환.
void CGameSystem::Summon_SequenceCharacter(class CTransform* pTransform)
{
	if (nullptr == m_pSequencePlayer || 
		nullptr == pTransform)
		return;

	// 
	m_pSequencePlayer->Summon_Squad_Near_Boss(pTransform);
}
#pragma endregion

#pragma region TIMELACK
void CGameSystem::Update_TimeLack(_float fTimeDelta)
{
	m_pTimeLack->Update(fTimeDelta);
}
void CGameSystem::Change_TimeRate(COLLISIONLAYER eLayer, _float fRate)
{
	m_pTimeLack->Change_TimeRate(eLayer, fRate);
}

void CGameSystem::Change_TimeRate(COLLISIONLAYER eLayer, _float fRate, _float fDuration)
{
	m_pTimeLack->Change_TimeRate(eLayer, fRate, fDuration);
}

_float CGameSystem::TimeLack(COLLISIONLAYER eLayer)
{
	return m_pTimeLack->TimeLack(eLayer);
}

#pragma endregion

#pragma region POTAL
void CGameSystem::Potal_Register(CPotal* pPotal)
{
	m_pPotal = pPotal;
	Safe_AddRef(m_pPotal);
}

void CGameSystem::Set_Potal_Active(_bool B)
{
	m_pPotal->PotalActive(B);
}

#pragma endregion


void CGameSystem::Release_System()
{
	Safe_Release(m_pParser);
	Safe_Release(m_pFactory);
	if (m_pPotal)
		Safe_Release(m_pPotal);

	Safe_Release(m_pUI_FontPreset);
	Safe_Release(m_pUI_ControlHelper);
	Safe_Release(m_pUI_GrappleController);

	Safe_Release(m_pDirector);
	Safe_Release(m_pPlayerStatus);
	Safe_Release(m_pSonoro_Manager);
	//Safe_Release(m_pUI_StatusSyncer);
	Safe_Release(m_pMonsterTable);
	Safe_Release(m_pMouseController);
	Safe_Release(m_pPlayer);
	Safe_Release(m_pSequencePlayer);

	Safe_Release(m_pTimeLack);

	Release();
}

void CGameSystem::Free()
{
	__super::Free();
}
