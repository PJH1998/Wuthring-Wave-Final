#include "ClientPch.h"
#include "AugustaGroundSprint.h"
#include "Augusta.h"
#include "StateMachine.h"


HRESULT CAugustaGroundSprint::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    return S_OK;
}

void CAugustaGroundSprint::OnEnter()
{
    // 상위 객체 수행 작업.
    CGroundState::OnEnter();

    m_fSprintTime = 0.f;

    // Sprint 시작 애니메이션
    // Super_Sprint_Start 또는 Sprint_Impulse_F
}

void CAugustaGroundSprint::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    m_fSprintTime += fTimeDelta;
    Update_SprintSpeed(fTimeDelta);
}

void CAugustaGroundSprint::OnExit()
{
    CGroundState::OnExit();

    // Sprint 종료 애니메이션
    // Super_Sprint_End 또는 Stop_Sprint_L/R
}

void CAugustaGroundSprint::Update_SprintSpeed(_float fTimeDelta)
{
    // TODO: Sprint 속도 가속
    // Sprint_F → Super_Sprint_Start_F → Super_Sprint_End
}

void CAugustaGroundSprint::Check_StateTransition()
{
    // TODO: Shift 떼면 Run으로, 입력 없으면 Idle로
}

CAugustaGroundSprint* CAugustaGroundSprint::Create(class CGameObject* pOwner)
{
    CAugustaGroundSprint* pInstance = new CAugustaGroundSprint();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundSprint");
    }

    return pInstance;
}

void CAugustaGroundSprint::Free()
{
    CGroundState::Free();
}
