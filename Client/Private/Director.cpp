#include "ClientPch.h"
#include "Director.h"

CDirector::CDirector()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

CDirector* CDirector::Create()
{
    return new CDirector();
}

void CDirector::Free()
{
	Safe_Release(m_pGameInstance);
}
