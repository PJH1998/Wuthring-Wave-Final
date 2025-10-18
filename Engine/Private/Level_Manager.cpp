#include "EnginePch.h"
#include "Level_Manager.h"
#include "GameInstance.h"

#include "Level.h"

CLevel_Manager::CLevel_Manager()
    : m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CLevel_Manager::Open_Level(_uint iCurrentLevel, CLevel* pCurrentLevel)
{
    if (nullptr == pCurrentLevel)
        return E_FAIL;

    // ?꾩옱 Level Resource ?뺣━
     if (FAILED(Clear_Resources()))
        return E_FAIL;

    m_iCurrentLevel = iCurrentLevel;

    Safe_Release(m_pCurrentLevel);
    m_pCurrentLevel = pCurrentLevel;

    return S_OK;
}

void CLevel_Manager::Update_Level(_float fTimeDelta)
{
    if (nullptr == m_pCurrentLevel)
        return;
    m_pCurrentLevel->Update(fTimeDelta);
}

HRESULT CLevel_Manager::Render()
{
    if (nullptr == m_pCurrentLevel)
        return S_OK;
    m_pCurrentLevel->Render();
	return S_OK;
}

HRESULT CLevel_Manager::Clear_Resources()
{
    if (nullptr == m_pCurrentLevel)
        return S_OK;
    return m_pGameInstance->Clear_Resource(m_iCurrentLevel);
}

CLevel_Manager* CLevel_Manager::Create()
{
    return new CLevel_Manager();
}

void CLevel_Manager::Free()
{
    __super::Free();

    Safe_Release(m_pCurrentLevel);
    Safe_Release(m_pGameInstance);
}
