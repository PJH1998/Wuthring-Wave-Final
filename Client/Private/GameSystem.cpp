#include "ClientPch.h"
#include "GameSystem.h"

#include "Parser.h"
#include "Factory.h"
#include "Director.h"
#include "PlayerStatus.h"
#include "Player.h"

#include "UI_FontPreset.h"
#include "UI_ControlHelper.h"
#include "UI_StatusSyncer.h"

#include"Sonoro_Manager.h"

#include "MonsterTable.h"

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

	//m_pUI_StatusSyncer = CUI_StatusSyncer::Create();
	//ASSERT_CRASH(m_pUI_ControlHelper);

	m_pDirector = CDirector::Create();
	ASSERT_CRASH(m_pDirector);

	m_pSonoro_Manager = CSonoro_Manager::Create();
	ASSERT_CRASH(m_pSonoro_Manager);

	m_pMonsterTable = CMonsterTable::Create();
	ASSERT_CRASH(m_pMonsterTable);

	// 파일 목록 만들기.
	vector<_string> AbilityFolders = {};
	AbilityFolders.resize(CPlayer::CHARACTERTYPE::TYPE_END);

	AbilityFolders[CPlayer::CHARACTERTYPE::ROVER] = "../Bin/Resource/Model/Player/Rover/Ability/";
	AbilityFolders[CPlayer::CHARACTERTYPE::AUGUSTA] = "../Bin/Resource/Model/Player/Augusta/Ability/";
	AbilityFolders[CPlayer::CHARACTERTYPE::GALBRENA] = "../Bin/Resource/Model/Player/Galbrena/Ability/";
	m_pPlayerStatus = CPlayerStatus::Create(pDevice, pContext, AbilityFolders);
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

void CGameSystem::Ready_Prototype_Map(const _char* pFilePath, LEVEL eLevel)
{
	return m_pParser->Ready_Prototype_Map(pFilePath, eLevel);
}

void CGameSystem::Clone_MapObjects(LEVEL eLevel)
{
	m_pParser->Clone_MapObjects(eLevel);
}
void CGameSystem::Clone_Spawners(LEVEL eLevel)
{
	m_pParser->Clone_Spawners(eLevel);
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
void CGameSystem::Play_Action(const _wstring& strActionTag, const _fmatrix& WorldMatrix, _bool isMaintain)
{
	m_pDirector->Play_Action(strActionTag, WorldMatrix, isMaintain);
}
void CGameSystem::Stop_Action()
{
	m_pDirector->Stop_Action();
}
#pragma endregion

#pragma region CHARACTER INFO
void CGameSystem::Sync_CharacterInfo(const CHARACTER_STAT& eCharacterStat)
{
	m_Stats = eCharacterStat;
}

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

CCustom_UI* CGameSystem::Find_RootUI(_wstring strName)
{
	return m_pUI_ControlHelper->Find_RootUI(strName);
}

CCustom_UI* CGameSystem::Find_ChildUI(_wstring strRootUIName, _wstring strChildUIName)
{
	return m_pUI_ControlHelper->Find_ChildUI(strRootUIName, strChildUIName);
}

HRESULT CGameSystem::HUD_FadeOut()
{
	return m_pUI_ControlHelper->HUD_FadeOut();
}

HRESULT CGameSystem::HUD_FadeIn()
{
	return m_pUI_ControlHelper->HUD_FadeIn();
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

void CGameSystem::HUD_Toggle_BossStatusUI(_bool isOn)
{
	return m_pUI_ControlHelper->HUD_Toggle_BossStatusUI(isOn);
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

_bool CGameSystem::Get_InteractUI_Feedback(UI_EVENT_TYPE eEventInteractType)
{
	return m_pUI_ControlHelper->Get_InteractUI_Feedback(eEventInteractType);
}

void CGameSystem::Attach_LockOnUI(CTransform* pTargetTransform)
{
	m_pUI_ControlHelper->Attach_LockOnUI(pTargetTransform);
}

void CGameSystem::Detach_LockOnUI()
{
	m_pUI_ControlHelper->Detach_LockOnUI();
}

//HRESULT	CGameSystem::Sync_Status_toHUD(CHARACTER_STAT& eStat)
//{
//	return m_pUI_StatusSyncer->Sync_Status_toHUD(eStat);
//}
#pragma region PLAYER STATUS

#pragma endregion


#pragma region TRIGGER

void CGameSystem::TriggerRegister(_uint iNumTriggerMapIndex, TriggerCallback pFunc)
{
	m_TriggerEvents[iNumTriggerMapIndex].push_back(pFunc);
}

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

void CGameSystem::Update(_float fTimeDelta)
{
	m_pSonoro_Manager->Update(fTimeDelta);
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
#pragma endregion

void CGameSystem::Release_System()
{
	Safe_Release(m_pParser);
	Safe_Release(m_pFactory);

	Safe_Release(m_pUI_FontPreset);
	Safe_Release(m_pUI_ControlHelper);
	Safe_Release(m_pDirector);
	Safe_Release(m_pPlayerStatus);
	Safe_Release(m_pSonoro_Manager);
	//Safe_Release(m_pUI_StatusSyncer);
	Safe_Release(m_pMonsterTable);

	Release();
}

void CGameSystem::Free()
{
	__super::Free();
}
