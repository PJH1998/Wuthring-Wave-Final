#include "ClientPch.h"
#include "AugustaGroundIdle.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaGroundIdle::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    // Idle 애니메이션 리스트 셋업
    Setup_Animations();

    // 기본 애니메이션 셋업.
    m_iCurrentAnimIdx = 0;
    return S_OK;
}

void CAugustaGroundIdle::OnEnter()
{
    CGroundState::OnEnter();

    // 1. 복사본 Context 받아오기
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EIdleType eIdleType = context.m_eIdleType;

    m_iCurrentAnimIdx = ENUM_CLASS(eIdleType);

    // 3. Idle 상태 초기화
    State_Reset();
}

void CAugustaGroundIdle::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 입력 확인
    Handle_Input();

    // 1. Idle 업데이트
    Update_IdleAnimations(fTimeDelta);

    // 2. LockOn 여부 확인 및 상태 전환.
    if (m_pAugusta->Is_LockOn())
        LockOn_StateTransition(fTimeDelta);
    else
        Check_StateTransition(fTimeDelta);

    // 3. 상태 초기화
    State_Reset();
    
}

void CAugustaGroundIdle::OnExit()
{
    CGroundState::OnExit();
}

void CAugustaGroundIdle::Handle_Input()
{
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[SPRINT] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT) | ENUM_CLASS(KEYINPUT::RB));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
}

// Idle 간의 전환 지정.
void CAugustaGroundIdle::Update_IdleAnimations(_float fTimeDelta)
{
    // 1. 현재 애니메이션 재생
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
}

// Idles 조건이 아닌 것들.
void CAugustaGroundIdle::Check_StateTransition(_float fTimeDelta)
{

    EIdleType eIdleType = static_cast<EIdleType>(m_iCurrentAnimIdx);

    _uint iKeyInput = {};

    // 점프
    if (m_States[JUMP])
    {
        m_pAugusta->GetStateContextForWrite().m_eJumpType = EJumpType::JUMP_WALK_LF;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP)); // 상위, 하위 상태
        return;
    }

    // Sprint
    if (m_States[SPRINT])
    {
        m_pAugusta->GetStateContextForWrite().m_eSprintType = ESprintType::MOVE_F;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SPRINT)); // 상위, 하위 상태
        return;
    }

    // 이동은 Run State에서 조절.
    if (m_States[MOVE])
    {
        m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN)); // 상위, 하위 상태
        return;
    }

    // 자동 변환.
    if (m_IsAnimationEnd)
    {
        EIdleType nextIdle = EIdleType::STAND1_ACTION01;

        switch (static_cast<EIdleType>(m_iCurrentAnimIdx))
        {
        case EIdleType::STAND1_ACTION01:
            nextIdle = EIdleType::STAND1_ACTION02;
            break;
        case EIdleType::STAND1_ACTION02:
            nextIdle = EIdleType::STAND1_ACTION03;
            break;
        case EIdleType::STAND1_ACTION03:
            nextIdle = EIdleType::STAND1_ACTION01;  // 다시 처음으로
            break;
        default:
            nextIdle = EIdleType::STAND1_ACTION01;
            break;
        }

        m_iCurrentAnimIdx = ENUM_CLASS(nextIdle);
        return;
    }

}

void CAugustaGroundIdle::LockOn_StateTransition(_float fTimeDelta)
{

}

void CAugustaGroundIdle::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EIdleType::STAND1_ACTION01), "Stand1_Action01", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EIdleType::STAND1_ACTION02), "Stand1_Action02", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EIdleType::STAND1_ACTION03), "Stand1_Action03", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EIdleType::STAND2), "Stand2", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EIdleType::STAND_CONTROL), "Stand_Control", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EIdleType::STANDCHANGE), "StandChange", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EIdleType::STANDUP), "StandUp", 1.f, 0.f);
}


void CAugustaGroundIdle::State_Reset()
{
    for (_uint i = 0; i < IDLESTATE::END; ++i)
        m_States[i] = false;
}

CAugustaGroundIdle* CAugustaGroundIdle::Create(class CGameObject* pOwner)
{
    CAugustaGroundIdle* pInstance = new CAugustaGroundIdle();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundIdle");
        return nullptr;
    }

    return pInstance;
}

void CAugustaGroundIdle::Free()
{
    CGroundState::Free();
}
