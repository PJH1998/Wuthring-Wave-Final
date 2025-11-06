#include "ClientPch.h"
#include "HitState.h"
#include "StateMachine.h"

HRESULT CHitState::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CCharacterState::Initialize(pOwner)))
        return E_FAIL;

    return S_OK;
}

void CHitState::OnEnter(void* pArg)
{
    CCharacterState::OnEnter(pArg);
}

void CHitState::OnUpdate(_float fTimeDelta)
{
    CCharacterState::OnUpdate(fTimeDelta);

    Apply_KnockbackForce(fTimeDelta);
    Check_HitRecovery();

}

void CHitState::OnExit()
{
    CCharacterState::OnExit();
}

void CHitState::Apply_KnockbackForce(_float fTimeDelta)
{
}

void CHitState::Check_HitRecovery()
{
}

void CHitState::Change_SubState(const _string& strSubStateName, CStateMachine* pStateMachine)
{
}

void CHitState::Free()
{
    CCharacterState::Free();
}
