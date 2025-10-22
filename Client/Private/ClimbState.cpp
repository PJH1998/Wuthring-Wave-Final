#include "ClientPch.h"
#include "ClimbState.h"
#include "StateMachine.h"


HRESULT CClimbState::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CCharacterState::Initialize(pOwner)))
        return E_FAIL;

    return S_OK;
}

void CClimbState::OnEnter()
{
    CCharacterState::OnEnter();
}

void CClimbState::OnUpdate(_float fTimeDelta)
{
    CCharacterState::OnUpdate(fTimeDelta);

    Check_ClimbSurface();
    Check_ClimbExit();

    if (nullptr != m_pCurrentSubState)
    {
        m_pCurrentSubState->OnUpdate(fTimeDelta);
    }
}

void CClimbState::OnExit()
{
    CCharacterState::OnExit();
}

void CClimbState::Check_ClimbSurface()
{
    // TODO: 등반 가능한 표면 체크
}

void CClimbState::Check_ClimbExit()
{
    // TODO: 등반 탈출 조건 체크
}

void CClimbState::Change_SubState(const _string& strSubStateName, CStateMachine* pStateMachine)
{
    if (nullptr != m_pCurrentSubState)
    {
        m_pCurrentSubState->OnExit();
    }
}

void CClimbState::Free()
{
    CCharacterState::Free();
}
