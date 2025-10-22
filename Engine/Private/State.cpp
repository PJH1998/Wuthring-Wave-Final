#include "EnginePch.h"
#include "State.h"
#include "StateMachine.h"
#include "ComputeShader.h"
#include "Model.h"


HRESULT CState::Initialize(const STATE_DATA& StateData)
{
    m_StateData = StateData;
    return S_OK;
}

void CState::OnEnter()
{
    m_fTrackPosition = 0.f;
    m_IsAnimationEnd = false;
}

void CState::OnUpdate(_float fTimeDelta)
{

}


void CState::OnExit()
{

}

void CState::Change_State(CStateMachine* pStateMachine, const _string& strStateName)
{
    pStateMachine->Change_State(strStateName);
}


// Transition에 해당되면 전환
const _string& CState::Check_Transition(CStateMachine* pStateMachine)
{
    for (auto& condition : m_InputTransitions)
    {
        if (condition.second())
        {
            //pStateMachine->Change_State(condition.first);
            return condition.first;
        }
    }

    return "";
}

void CState::Add_Transition(const _string& strStateName, InputCondition condition)
{
    m_InputTransitions.emplace_back(make_pair(strStateName, condition));
}


void CState::Free()
{
    CBase::Free();
    m_InputTransitions.clear();
}
