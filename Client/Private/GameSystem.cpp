#include "ClientPch.h"
#include "GameSystem.h"

#include "Parser.h"
#include "Factory.h"
#include "Director.h"

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

	m_pDirector = CDirector::Create();
	ASSERT_CRASH(m_pDirector);
}
#pragma region PARSER
const vector<vector<_string>>& CGameSystem::Load_CSV(const _char* pFilePath)
{
	return m_pParser->Load_CSV(pFilePath);
}

void CGameSystem::Ready_Prototype_Map(const _char* pFilePath, LEVEL eLevel)
{
	return m_pParser->Ready_Prototype_Map(pFilePath, eLevel);
}

void CGameSystem::Clone_MapObjects(LEVEL eLevel, _uint iIndex)
{
	m_pParser->Clone_MapObjects(eLevel, iIndex);
}
#pragma endregion

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
#pragma endregion

void CGameSystem::Free()
{
	__super::Free();

	Safe_Release(m_pParser);
	Safe_Release(m_pFactory);
	Safe_Release(m_pDirector);
}
