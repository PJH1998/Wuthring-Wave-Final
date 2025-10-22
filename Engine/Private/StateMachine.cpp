#include "EnginePch.h"
#include "StateMachine.h"
#include "State.h"


#pragma region 기본 함수들
CStateMachine::CStateMachine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{ pDevice, pContext }
{
}

CStateMachine::CStateMachine(const CStateMachine& Prototype)
    : CComponent(Prototype)
    , m_States{ Prototype.m_States }
    , m_StateMap { Prototype.m_StateMap }
{
    for (auto& pState : m_States)
        Safe_AddRef(pState);
}



HRESULT CStateMachine::Initialize_Prototype()
{

    return S_OK;
}

HRESULT CStateMachine::Initialize_Clone(void* pArg)
{
    return S_OK;
}

void CStateMachine::Update(_float fTimeDelta)
{
    // 1. 전환조건 체크
    _string strNextState = m_States[m_iCurrentStateIndex]->Check_Transition(this);

    // 2. 전환 State이름이 들어왔다면?
    if (!strNextState.empty())
    {
        Change_State(strNextState);
        return;
    }

    // 3. 아무 일도 없다면? 현재 값 업데이트
    m_States[m_iCurrentStateIndex]->OnUpdate(fTimeDelta);
}

// State에서 StateMachine에 호출.
void CStateMachine::Change_State(const _string& strStateName)
{
    // 1. 현재 State의 종료 처리 진행.
    _uint iPrevStateIndex = m_iCurrentStateIndex;
    m_States[iPrevStateIndex]->OnExit();

    // 2. 현재 State 변경.
    m_iCurrentStateIndex = m_StateMap[strStateName];
    m_States[m_iCurrentStateIndex]->OnEnter();
}

void CStateMachine::Add_State(const _string& strStateName, CState* pState)
{
    // 1. 맵에 찾아주기.
    m_StateMap.emplace(strStateName, m_States.size());
    // 2. State 넣어주기.
    m_States.emplace_back(pState);
}


#pragma endregion


CStateMachine* CStateMachine::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CStateMachine* pInstance = new CStateMachine(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CPlayerParty");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CStateMachine::Clone(void* pArg)
{
    CStateMachine* pInstance = new CStateMachine(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CStateMachine");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CStateMachine::Free()
{
    CComponent::Free();
    for (auto& pState : m_States)
        Safe_Release(pState);
    m_States.clear();

    m_StateMap.clear();
}
