#include "EnginePch.h"
#include "GameInstance.h"
#include "AnimNotify.h"

CAnimNotify::CAnimNotify(_float fTrackPosition)
    : m_fTrackPosition{ fTrackPosition }
    , m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

void CAnimNotify::Free()
{
    CBase::Free();
    Safe_Release(m_pGameInstance);
}