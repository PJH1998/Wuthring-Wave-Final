#include "ClientPch.h"
#include "Director.h"

CDirector::CDirector()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

void CDirector::Add_Action(const _char* pFolderPath)
{

}

void CDirector::Play_Action(const _wstring& strActionTag, _bool isMaintain)
{
}

CDirector* CDirector::Create()
{
    return new CDirector();
}

void CDirector::Free()
{
	Safe_Release(m_pGameInstance);
}
