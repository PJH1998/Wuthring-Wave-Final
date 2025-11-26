#include "ClientPch.h"
#include "InteractionState.h"

HRESULT CInteractionState::Initialize(CGameObject* pOwner)
{
	if (FAILED(CCharacterState::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CInteractionState::OnEnter(void* pArg)
{
	CCharacterState::OnEnter(pArg);
}

void CInteractionState::OnUpdate(_float fTimeDelta)
{
	CCharacterState::OnUpdate(fTimeDelta);
}

void CInteractionState::OnExit()
{
	CCharacterState::OnExit();
	m_iNotLandFrames = 0;
}

void CInteractionState::Free()
{
	CCharacterState::Free();
}
