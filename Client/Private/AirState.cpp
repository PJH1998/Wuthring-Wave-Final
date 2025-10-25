#include "ClientPch.h"
#include "AirState.h"
#include "StateMachine.h"

HRESULT CAirState::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CCharacterState::Initialize(pOwner)))
        return E_FAIL;

    return S_OK;
}

void CAirState::OnEnter()
{
    CCharacterState::OnEnter();
}

void CAirState::OnUpdate(_float fTimeDelta)
{
    CCharacterState::OnUpdate(fTimeDelta);

    Apply_AirGravity(fTimeDelta);
    Check_GroundLanding();

}

void CAirState::OnExit()
{
    CCharacterState::OnExit();
}

void CAirState::Apply_AirGravity(_float fTimeDelta)
{
    // TODO: 공중 중력 적용
}

void CAirState::Check_GroundLanding()
{
    // TODO: 착지 체크
}

void CAirState::Change_SubState(const _string& strSubStateName, CStateMachine* pStateMachine)
{
    if (nullptr != m_pCurrentSubState)
    {
        m_pCurrentSubState->OnExit();
    }

}

void CAirState::Free()
{
    CCharacterState::Free();
}
