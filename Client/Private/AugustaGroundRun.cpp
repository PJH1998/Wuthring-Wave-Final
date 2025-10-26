#include "ClientPch.h"
#include "AugustaGroundRun.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaGroundRun::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaGroundRun::OnEnter()
{
    CGroundState::OnEnter();

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERunType eRunType = context.m_eRunType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eRunType);

    // 4. 현재 상태 초기화
    State_Reset();

    m_pAugusta->Set_Gravity(true);
}

void CAugustaGroundRun::OnUpdate(_float fTimeDelta)
{
    
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키입력 감지.
    Handle_Input();

    // 1. 애니메이션 갱신.
    Update_RunAnimation(fTimeDelta);

    // 2. 물리 체크.
    Check_Physics();

    // 3. 전환 제어
   /* if (m_pAugusta->Is_LockOn())
        LockOnCheck_StateTransition(fTimeDelta);
    else
        Check_StateTransition(fTimeDelta);*/
    Check_StateTransition(fTimeDelta);

    // 4. 현재  상태 초기화
    State_Reset();
    
}

void CAugustaGroundRun::OnExit()
{
    CGroundState::OnExit();
    m_pAugusta->Set_Gravity(true);
}

void CAugustaGroundRun::Handle_Input()
{
    // 1. 방향 계산
    m_eDir = m_pAugusta->Calculate_Direction();

    // 키 입력.
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey); // WASD 키입력 체크.
    m_States[DASH] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT) | ENUM_CLASS(KEYINPUT::RB));

    m_States[RUN_U] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
    m_States[RUN_D] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
    m_States[RUN_L] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
    m_States[RUN_R] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));
    

    // DASH보다 우선순위 높음.
    m_States[SPRINT_F] = m_States[MOVE] && m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));


    // 공격 상태가 아니라 공격 판정 상태로 전달.
    m_States[ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

    // 상태에 따라 속도 다르게.
    m_fSpeed = m_States[SPRINT_F] ? 1.f : 0.5f;
}





void CAugustaGroundRun::Update_RunAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);


    ERunType eRunType = static_cast<ERunType>(m_iCurrentAnimIdx);
    // 1. 회전 및 이동.
    if (m_pAugusta->Is_LockOn())
    {
        if (eRunType == ERunType::SPRINT_F || eRunType == ERunType::STOP_SPRINT_L)
            m_pAugusta->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, m_fSpeed);
        else 
            // 1. WASD 입력에 따른 8방향 이동
            m_pAugusta->Move_LockOn_8Way(m_eDir, fTimeDelta, m_fSpeed);
    }
    else 
        m_pAugusta->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, m_fSpeed);

}

void CAugustaGroundRun::Check_Physics()
{
    // Wall인지?
    m_States[WALL] = m_pAugusta->Check_ClimbableWall(&m_vWallNormal);
    // Land Check
    m_States[LAND] = m_pAugusta->Is_Land(&m_vLandNormal);
}

void CAugustaGroundRun::LockOnCheck_StateTransition(_float fTimeDelta)
{
    ERunType eRunType = static_cast<ERunType>(m_iCurrentAnimIdx);

    _float fOffsetY = 0.1f;
    _float fDistanceToGround = m_pAugusta->Get_DistanceToGround(fOffsetY);

    if (!m_States[LAND] && fDistanceToGround > 1.f)
    {
        m_pAugusta->GetStateContextForWrite().m_eFallType = EFallType::FALL_LOOP;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
        return;
    }

    // SPACE 누르면 바로 점프로 전환.
    if (m_States[JUMP])
    {
        m_pAugusta->GetStateContextForWrite().m_eJumpType = EJumpType::JUMP_WALK_LF;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP)); // 상위, 하위 상태
        return;
    }

    // Run => Attack
    if (m_States[ATTACK])
    {
        m_pAugusta->GetStateContextForWrite().m_eAttackType = EAttackType::ATTACK01;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::ATTACK)); // 상위, 하위 상태
        return;
    }

    // Run => Sprint
    if (m_States[SPRINT_F])
    {
        m_iCurrentAnimIdx = ENUM_CLASS(ERunType::SPRINT_F);
        return;
    }

    // 뛰다가 DASH
    if (m_States[DASH])
    {
        m_pAugusta->GetStateContextForWrite().m_eDashType = EDashType::MOVE_F;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::DASH)); // 상위, 하위 상태
        return;
    }

    if (m_States[MOVE])
    {

        if (m_States[RUN_U])
        {
            if (m_States[RUN_L])
                m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_LF);
            else if (m_States[RUN_R])
                m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_RF);
            else
                m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_F);
        }
        else if (m_States[RUN_D])
        {
            if (m_States[RUN_L])
                m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_LB);
            else if (m_States[RUN_R])
                m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_RB);
            else
                m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_B);
        }
        else if (m_States[RUN_L])
            m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_LF);
        else if (m_States[RUN_R])
            m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_RF);

        return;
    }

    // 이동 상태를 아래에 몰아둔다. => 우선 순위 낮음
    // 이동 입력 값이 안들어왔다면?
    if (!m_States[MOVE])
    {
        // 현재 상태가 Sprint 였다면?
        if (eRunType == ERunType::SPRINT_F)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(ERunType::STOP_SPRINT_L);
            return;
        }

        // 현재 상태가 STOP_RUN이 아니라면? => STOP RUN
        if (eRunType != ERunType::STOP_RUN_L)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(ERunType::STOP_RUN_L);
            return;
        }
        // Stop Run 이면서 애니메이션 재생이 끝났다면?.
        if ((eRunType == ERunType::STOP_RUN_L || eRunType == ERunType::STOP_SPRINT_L) && m_IsAnimationEnd)
        {
            m_pAugusta->GetStateContextForWrite().m_eIdleType = EIdleType::STAND1_ACTION01;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
            return;
        }
    }

}

void CAugustaGroundRun::Check_StateTransition(_float fTimeDelta)
{
 
    ERunType eRunType = static_cast<ERunType>(m_iCurrentAnimIdx);
    _float3 vNormal = {}; // 벽타기 전환 용도 Normal
    // 이 조건은 추후 디테일 잡아보기.
    _float fOffsetY = 0.1f;
    _float fDistanceToGround = m_pAugusta->Get_DistanceToGround(fOffsetY);

    //// 전방 벽감지.
    //if (m_States[RUN_U] && m_States[WALL])
    //{
    //    m_pAugusta->GetStateContextForWrite().m_eClimbMoveType = EClimbMoveType::CLIMB_U_1;
    //    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_MOVE)); // 상위, 하위 상태
    //    return;
    //}
    
    if (!m_States[LAND] && fDistanceToGround > 1.f)
    {
        m_pAugusta->GetStateContextForWrite().m_eFallType = EFallType::FALL_LOOP;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
        return;
    }

    // SPACE 누르면 바로 점프로 전환.
    if (m_States[JUMP])
    {
        m_pAugusta->GetStateContextForWrite().m_eJumpType = EJumpType::JUMP_WALK_LF;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP)); // 상위, 하위 상태
        return;
    }

    // Run => Attack
    if (m_States[ATTACK])
    {
        m_pAugusta->GetStateContextForWrite().m_eAttackType = EAttackType::ATTACK01;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::ATTACK)); // 상위, 하위 상태
        return;
    }
  
    // Dash 보다 우선순위 높음.
    if (m_States[SPRINT_F])
    {
        m_iCurrentAnimIdx = ENUM_CLASS(ERunType::SPRINT_F);
        return;
    }

    // 뛰다가 Dash
    if (m_States[DASH])
    {
        m_pAugusta->GetStateContextForWrite().m_eDashType = EDashType::MOVE_F;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::DASH)); // 상위, 하위 상태
        return;
    }

    if (m_States[MOVE])
    {
        // 만약에 현재 상태가 Sprint 였으면? => 애니메이션 변경을 하지 않음.

        if (m_pAugusta->Is_LockOn())
        {
            if (m_States[RUN_U])
            {
                if (m_States[RUN_L])
                    m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_LF);
                else if (m_States[RUN_R])
                    m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_RF);
                else
                    m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_F);
            }
            else if (m_States[RUN_D])
            {
                if (m_States[RUN_L])
                    m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_LB);
                else if (m_States[RUN_R])
                    m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_RB);
                else
                    m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_B);
            }
            else if (m_States[RUN_L])
                m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_LF);
            else if (m_States[RUN_R])
                m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_RF);

            return;
        }

        // 이동 값이 들어왔는데 Stop Run 상태라면?
        if (eRunType == ERunType::STOP_RUN_L)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(ERunType::RUN_F);
            return;
        }
    }

    // 이동 입력 값이 안들어왔다면?
    if (!m_States[MOVE])
    {
        // 현재 상태가 Sprint 였다면?
        if (eRunType == ERunType::SPRINT_F)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(ERunType::STOP_SPRINT_L);
            return;
        }

        // 현재 상태가 STOP_RUN이 아니라면? => STOP RUN
        if (eRunType != ERunType::STOP_RUN_L)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(ERunType::STOP_RUN_L);
            return;
        }
        // Stop Run 이면서 애니메이션 재생이 끝났다면?.
        if ((eRunType == ERunType::STOP_RUN_L || eRunType == ERunType::STOP_SPRINT_L) && m_IsAnimationEnd)
        {
            m_pAugusta->GetStateContextForWrite().m_eIdleType = EIdleType::STAND1_ACTION01;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
            return;
        }
    }

    
}



void CAugustaGroundRun::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_B), "Run_B", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_F), "Run_F", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_LB), "Run_LB", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_LF), "Run_LF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_RB), "Run_RB", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_RF), "Run_RF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_BASEPOSE), "Run_BasePose", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_POSE_F), "Run_Pose_F", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_POSE_L), "Run_Pose_L", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_POSE_R), "Run_Pose_R", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_TURNBACK), "Run_Turnback", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::SPRINT_F), "Sprint_F", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::STOP_RUN_L), "Stop_Run_L", 1.f, 0.f); // 왼발로 멈추기.
    CState::Add_Animations(ENUM_CLASS(ERunType::STOP_SPRINT_L), "Stop_Sprint_L", 1.f, 0.f); // 왼발로 멈추기
}

void CAugustaGroundRun::State_Reset()
{
    for (_uint i = 0; i < RUNSTATE::END; ++i)
    {
        m_States[i] = false;
    }
}



CAugustaGroundRun* CAugustaGroundRun::Create(class CGameObject* pOwner)
{
    CAugustaGroundRun* pInstance = new CAugustaGroundRun();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundRun");
        return nullptr;
    }

    return pInstance;
}

void CAugustaGroundRun::Free()
{
    CGroundState::Free();
}
