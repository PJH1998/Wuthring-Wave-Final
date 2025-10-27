#include "ClientPch.h"
#include "HitState.h"
#include "StateMachine.h"

HRESULT CHitState::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CCharacterState::Initialize(pOwner)))
        return E_FAIL;

    return S_OK;
}

void CHitState::OnEnter()
{
    CCharacterState::OnEnter();
}

void CHitState::OnUpdate(_float fTimeDelta)
{
    CCharacterState::OnUpdate(fTimeDelta);

    Apply_KnockbackForce(fTimeDelta);
    Check_HitRecovery();

    if (nullptr != m_pCurrentSubState)
    {
        m_pCurrentSubState->OnUpdate(fTimeDelta);
    }
}

void CHitState::OnExit()
{
    CCharacterState::OnExit();
}

void CHitState::Apply_KnockbackForce(_float fTimeDelta)
{
    // TODO: 넉백 힘 적용
}

void CHitState::Check_HitRecovery()
{
    // TODO: 피격 복구 체크
}

void CHitState::Change_SubState(const _string& strSubStateName, CStateMachine* pStateMachine)
{
    if (nullptr != m_pCurrentSubState)
    {
        m_pCurrentSubState->OnExit();
    }

}

void CHitState::Free()
{
    CCharacterState::Free();
}
