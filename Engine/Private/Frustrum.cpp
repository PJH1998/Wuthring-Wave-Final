#include "EnginePch.h"
#include "Frustrum.h"

#include "GameInstance.h"

CFrustrum::CFrustrum()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CFrustrum::Initialize()
{
    return S_OK;
}

void CFrustrum::Update()
{
}

CFrustrum* CFrustrum::Create()
{
	CFrustrum* pInstance = new CFrustrum();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Frustrum");
		Safe_Release(pInstance);
	}

    return pInstance;
}

void CFrustrum::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
}
