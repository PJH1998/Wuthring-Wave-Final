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

void CGameSystem::Free()
{
	__super::Free();

	Safe_Release(m_pParser);
}
