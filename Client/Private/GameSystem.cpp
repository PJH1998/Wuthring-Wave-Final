#include "ClientPch.h"
#include "GameSystem.h"

#include "Parser.h"
#include "Factory.h"

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
}

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

void CGameSystem::Create_Effect(const string& strFolderPath, LEVEL eLevel)
{
	return m_pParser->Create_Effect(strFolderPath, eLevel);
}

void CGameSystem::Create_Prefab(const string& strFolderPath, LEVEL eLevel)
{
	return m_pParser->Create_Prefab(strFolderPath, eLevel);
}

void CGameSystem::Load_EffectTexture_FromFolder(const string& strFolderPath, LEVEL eLevel)
{
	return m_pParser->Load_EffectTexture_FromFolder(strFolderPath, eLevel);
}

void CGameSystem::Load_EffectMeshDat_FromFolder(const string& strFolderPath, LEVEL eLevel)
{
	return m_pParser->Load_EffectMeshDat_FromFolder(strFolderPath, eLevel);
}

void CGameSystem::Create_MonsterDummy(LEVEL eLayerLevel, _float3 vPos, const _fmatrix& PreTransformationMatrix)
{
	m_pFactory->Create_MonsterDummy(eLayerLevel, vPos, PreTransformationMatrix);
}

void CGameSystem::Sync_CharacterInfo(const CHARACTER_STAT& eCharacterStat)
{
	m_Stats = eCharacterStat;
}



void CGameSystem::Free()
{
	__super::Free();

	Safe_Release(m_pParser);
	Safe_Release(m_pFactory);
}
