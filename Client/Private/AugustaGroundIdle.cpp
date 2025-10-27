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

    // 바꿀 파트타입?
    m_iPartType = CAugusta::PARTTYPE::PART_BAYONET;
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


    if (eIdleType == EIdleType::STAND1_ACTION01 || eIdleType == EIdleType::STAND1_ACTION02
        || eIdleType == EIdleType::STAND2)
    {
        m_pAugusta->PartAcitvate(m_iPartType, true);
    }

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

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    // 3. 상태 전환.
    Check_StateTransition(fTimeDelta);

    // 4. 상태 초기화
    State_Reset();
    
}

void CAugustaGroundIdle::OnExit()
{
    CGroundState::OnExit();

    m_pAugusta->PartAcitvate(m_iPartType, false);
}

void CAugustaGroundIdle::Handle_Input()
{
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    
    m_States[DASH] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT) | ENUM_CLASS(KEYINPUT::RB));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);

    m_States[SPRINT] = m_States[MOVE] && m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));

    // AttackState에서 판별.
    m_States[ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

    // LockOn인 경우에는 W, A, S, D 입력값을 모두 판별.
    if (m_pAugusta->Is_LockOn())
    {
        m_States[MOVE_U] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
        m_States[MOVE_D] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
        m_States[MOVE_L] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
        m_States[MOVE_R] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));
    }
}


// Idle 간의 전환 지정.
void CAugustaGroundIdle::Update_IdleAnimations(_float fTimeDelta)
{
    if (m_pAugusta->Is_LockOn()) // 오히려  안돌리는게 자연스러움?
    {
    }

    // 1. 현재 애니메이션 재생
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);


    EIdleType eIdleType = static_cast<EIdleType>(m_iCurrentAnimIdx);

    // 2. 파츠도 재생.
    if (eIdleType == EIdleType::STAND1_ACTION01 || eIdleType == EIdleType::STAND1_ACTION02
        || eIdleType == EIdleType::STAND2)
    {
        m_pAugusta->Play_PartAnimation(
            m_iPartType,
            m_Animations[m_iCurrentAnimIdx].strAnimName,
            fTimeDelta, nullptr
        );
    }
    
}

void CAugustaGroundIdle::Check_Physics(_float fTimeDelta)
{

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

    // 기본 공격
    if (m_States[ATTACK])
    {
        m_pAugusta->GetStateContextForWrite().m_eAttackType = EAttackType::ATTACK01;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::ATTACK)); // 상위, 하위 상태
        return;
    }

    // Sprint => 빠르게 달리기.
    if (m_States[SPRINT])
    {
        m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::SPRINT_F;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN)); // 상위, 하위 상태
        return;
    }

    // DASH
    if (m_States[DASH])
    {
        m_pAugusta->GetStateContextForWrite().m_eDashType = EDashType::MOVE_F;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::DASH)); // 상위, 하위 상태
        return;
    }

    
    // 이동은 Run State에서 조절.
    if (m_States[MOVE])
    {
        // 더 우선순위 높은 것. => Sprint
        

        // LockOn일때 전환 로직 변경.
        if (m_pAugusta->Is_LockOn())
        {
            if (m_States[MOVE_U])
            {
                if (m_States[MOVE_L])
                    m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_LF; // 애니메이션 상태 => 블랙보드에 기입.        
                else if (m_States[MOVE_R])
                    m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_RF; // 애니메이션 상태 => 블랙보드에 기입.        
                else
                    m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        
            }
            else if (m_States[MOVE_D])
            {
                if (m_States[MOVE_L])
                    m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_LB; // 애니메이션 상태 => 블랙보드에 기입.        
                else if (m_States[MOVE_R])
                    m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_RB; // 애니메이션 상태 => 블랙보드에 기입.        
                else
                    m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_B; // 애니메이션 상태 => 블랙보드에 기입.        
            }
            else if (m_States[MOVE_L])
                m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_LF; // 애니메이션 상태 => 블랙보드에 기입.        
            else if (m_States[MOVE_R])
                m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_RF; // 애니메이션 상태 => 블랙보드에 기입.        
        }
        else
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
