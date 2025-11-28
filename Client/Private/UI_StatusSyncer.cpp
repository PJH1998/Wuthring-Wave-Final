#include "ClientPch.h"

#include "UI_StatusSyncer.h"
#include "GameInstance.h"

#include "UI_HUD.h"

NS_BEGIN(Client)

CUI_StatusSyncer::CUI_StatusSyncer()
	: m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CUI_StatusSyncer::Initialize()
{
	//m_pRootUI_HUD = dynamic_cast<CUI_HUD*>(m_pGameInstance->Find_UIObject(L"UI_HUD"));
	//ASSERT_CRASH(m_pRootUI_HUD);
		
	return S_OK;
}


CUI_StatusSyncer* CUI_StatusSyncer::Create()
{
	CUI_StatusSyncer* pInstance = new CUI_StatusSyncer();

	if (FAILED(pInstance->Initialize()))
	{
		Safe_Release(pInstance);
		return nullptr;
	}
	return pInstance;
}

void CUI_StatusSyncer::Free()
{
	//Safe_Release(m_pRootUI_HUD);
	Safe_Release(m_pGameInstance);

	__super::Free();
}

NS_END
