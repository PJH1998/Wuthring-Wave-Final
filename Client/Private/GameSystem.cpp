#include "ClientPch.h"
#include "GameSystem.h"

#include "Parser.h"

IMPLEMENT_SINGLETON(CGameSystem)

CGameSystem::CGameSystem()
{
}

void CGameSystem::Ready_GameSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	m_pParser = CParser::Create(pDevice, pContext);
	ASSERT_CRASH(m_pParser);
}

const vector<vector<_string>>& CGameSystem::Load_CSV(const _char* pFilePath)
{
	return m_pParser->Load_CSV(pFilePath);
}

void CGameSystem::Create_Map_Model(const _char* pFilePath, LEVEL eLevel)
{
	return m_pParser->Create_Map_Model(pFilePath, eLevel);
}

void CGameSystem::Free()
{
	__super::Free();

	Safe_Release(m_pParser);
}
