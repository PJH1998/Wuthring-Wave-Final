#include "ClientPch.h"
#include "AugustaGroundSprint.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"


HRESULT CAugustaGroundSprint::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();

    return S_OK;
}

void CAugustaGroundSprint::OnEnter()
{
    // 상위 객체 수행 작업.
    CGroundState::OnEnter();

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ESprintType eSprintType = context.m_eSprintType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eSprintType);

    State_Reset();
}

void CAugustaGroundSprint::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키 입력
    Handle_Input();

    // 1. 애니메이션 실행
    Update_SprintAnimation(fTimeDelta);
    
    // 2. 상태 제어.
    if (m_pAugusta->Is_LockOn())
        LockOnCheck_StateTransition(fTimeDelta);
    else       
        Check_StateTransition(fTimeDelta);

    // 3. 상태 초기화.
    State_Reset();
}

void CAugustaGroundSprint::OnExit()
{
    CGroundState::OnExit();
}

void CAugustaGroundSprint::Handle_Input()
{
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
}





void CAugustaGroundSprint::Update_SprintAnimation(_float fTimeDelta)
{
    // 1. 방향 계산
    m_eDir = m_pAugusta->Calculate_Direction();
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
}

void CAugustaGroundSprint::LockOnCheck_StateTransition(_float fTimeDelta)
{
}

void CAugustaGroundSprint::Check_StateTransition(_float fTimeDelta)
{
    ESprintType eSprintType = static_cast<ESprintType>(m_iCurrentAnimIdx);

    // 애니메이션 끝나면?
    if (m_IsAnimationEnd)
    {
        m_pAugusta->GetStateContextForWrite().m_eIdleType = EIdleType::STAND1_ACTION01;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE)); // 상위, 하위 상태
        return;
    }
  

    switch (eSprintType)
    {
    case ESprintType::MOVE_F:
    {
        if (CState::Is_EscapePossible())
        {
            if (m_States[JUMP])
            {
                m_pAugusta->GetStateContextForWrite().m_eJumpType = EJumpType::JUMP_WALK_LF;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP)); // 상위, 하위 상태
                return;
            }

            if (m_States[MOVE])
            {
                m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN)); // 상위, 하위 상태
                return;
            }
        }
    }
        break;
    }

}

void CAugustaGroundSprint::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ESprintType::STOP_SPRINT_L), "Stop_Sprint_L", 1.f, 20.f);
    CState::Add_Animations(ENUM_CLASS(ESprintType::STOP_SPRINT_R), "Stop_Sprint_R", 1.f, 20.f);
    CState::Add_Animations(ENUM_CLASS(ESprintType::MOVE_F), "Move_F", 1.f, 20.f);
    CState::Add_Animations(ENUM_CLASS(ESprintType::MOVE_B), "Move_B", 1.f, 20.f);
    CState::Add_Animations(ENUM_CLASS(ESprintType::MOVE_LIMIT_B), "Move_Limit_B", 30.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESprintType::MOVE_LIMIT_F), "Move_Limit_F", 30.f, 0.f);
}

void CAugustaGroundSprint::State_Reset()
{
    for (_uint i = 0; i < SPRINTSTATE::END; ++i)
        m_States[i] = false;
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
