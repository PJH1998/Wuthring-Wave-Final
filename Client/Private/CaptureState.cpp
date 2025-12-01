#include "ClientPch.h"
#include "CaptureState.h"

HRESULT CCaptureState::Initialize(CCharacter* pOwner)
{
	if (FAILED(CCharacterState::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CCaptureState::OnEnter(void* pArg)
{
	CCharacterState::OnEnter(pArg);
}

void CCaptureState::OnUpdate(_float fTimeDelta)
{
	CCharacterState::OnUpdate(fTimeDelta);
}

void CCaptureState::OnExit()
{
	CCharacterState::OnExit();
}

void CCaptureState::Free()
{
	CCharacterState::Free();
}
