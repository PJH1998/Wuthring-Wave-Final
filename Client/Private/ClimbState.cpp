#include "ClientPch.h"
#include "ClimbState.h"
#include "StateMachine.h"


HRESULT CClimbState::Initialize(CCharacter* pOwner)
{
    if (FAILED(CCharacterState::Initialize(pOwner)))
        return E_FAIL;

    return S_OK;
}

void CClimbState::OnEnter(void* pArg)
{
    CCharacterState::OnEnter(pArg);
}

void CClimbState::OnUpdate(_float fTimeDelta)
{
    CCharacterState::OnUpdate(fTimeDelta);

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

}

void CClimbState::Free()
{
    CCharacterState::Free();
}
