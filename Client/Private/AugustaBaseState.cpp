#include "ClientPch.h"
#include "AugustaBaseState.h"

HRESULT CAugustaBaseState::Initialize(const STATE_DATA& StateData)
{
    // 1. 데이터 채우기.
    CState::Initialize(StateData);

    m_pPlayer = dynamic_cast<CPlayerAugusta*>(m_StateData.pOwner);
    ASSERT_CRASH(m_pPlayer);

   
    // 2. KeyBinding;
    Ready_KeyBind();

    // 3. Transition 등록 => 상속 개체에.

    return S_OK;
}

void CAugustaBaseState::OnEnter()
{

}

void CAugustaBaseState::OnUpdate(_float fTimeDelta)
{

}

void CAugustaBaseState::OnExit()
{
}

void CAugustaBaseState::Change_State(CStateMachine* pStateMachine, const _string& strStateName)
{
    CState::Change_State(pStateMachine, strStateName);
}



void CAugustaBaseState::Ready_KeyBind()
{
    // MoveKey 설정.
    m_iMoveKey |= static_cast<_uint>(KEYINPUT::W);
    m_iMoveKey |= static_cast<_uint>(KEYINPUT::A);
    m_iMoveKey |= static_cast<_uint>(KEYINPUT::S);
    m_iMoveKey |= static_cast<_uint>(KEYINPUT::D);

    
}

void CAugustaBaseState::Free()
{
    CState::Free();
    Safe_Release(m_pPlayer);
}
