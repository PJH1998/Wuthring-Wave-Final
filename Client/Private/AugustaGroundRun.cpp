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

}

void CAugustaGroundRun::OnUpdate(_float fTimeDelta)
{
    
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // 1. 방향 계산
    m_eDir = m_pAugusta->Calculate_Direction();
    
    // 2. 상태 제어.
    if (m_pAugusta->Is_LockOn())
    {
        LockOnUpdate_RunAnimation(fTimeDelta);
        LockOnCheck_StateTransition(fTimeDelta);
    }
    else
    {
        Update_RunAnimation(fTimeDelta);
        Check_StateTransition(fTimeDelta);
    }

    
}

void CAugustaGroundRun::OnExit()
{
    CGroundState::OnExit();
}

void CAugustaGroundRun::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_B), "Run_B", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_BASEPOSE), "Run_BasePose", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_F), "Run_F", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_LB), "Run_LB", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_LF), "Run_LF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_POSE_F), "Run_Pose_F", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_POSE_L), "Run_Pose_L", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_POSE_R), "Run_Pose_R", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_RB), "Run_RB", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_RF), "Run_RF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::RUN_TURNBACK), "Run_Turnback", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERunType::STOP_RUN_L), "Stop_Run_L", 1.f, 0.f);
    //CState::Add_Animations(static_cast<_uint>(ERunType::STOP_RUN_R), "Stop_Run_R", 1.f, 0.f);
}

void CAugustaGroundRun::LockOnUpdate_RunAnimation(_float fTimeDelta)
{
}

void CAugustaGroundRun::LockOnCheck_StateTransition(_float fTimeDelta)
{
}

void CAugustaGroundRun::Update_RunAnimation(_float fTimeDelta)
{
    // 회전 및 이동.
    m_pAugusta->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, 5.f);
}

void CAugustaGroundRun::Check_StateTransition(_float fTimeDelta)
{
 
    ERunType eRunType = static_cast<ERunType>(m_iCurrentAnimIdx);

    _float3 vNormal = {}; // 벽타기 전환 용도 Normal

    _bool IsLand = m_pAugusta->Is_Land(&vNormal);
    // 뛰고 있는데 땅이 아니라면? => 우선순위 젤 높음. 일정 오프셋은 무시?

    _float fDistanceToGround = m_pAugusta->Get_DistanceToGround();
    if (!IsLand )
    {
        if (fDistanceToGround > 0.2f)
        {
            m_pAugusta->GetStateContextForWrite().m_eFallType = EFallType::FALL_LOOP;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
        }
        return;
    }

    // 어떤 상태이건 Space를 누르면 바로 점프로 전환. => 그다음.
    if (m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE)))
    {
        m_pAugusta->GetStateContextForWrite().m_eJumpType = EJumpType::JUMP_WALK_LF;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP)); // 상위, 하위 상태
        return;
    }
  
    // 뛰다가 Sprint
    if (m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT) | ENUM_CLASS(KEYINPUT::RB)))
    {
        m_pAugusta->GetStateContextForWrite().m_eSprintType = ESprintType::MOVE_F;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SPRINT)); // 상위, 하위 상태
        return;
    }
   

    // 이동 상태를 아래에 몰아둔다. => 우선 순위 낮음
    // 이동 입력 값이 안들어왔다면?
    if (!m_pAugusta->Check_AnyInput(m_iMoveKey))
    {
        // 현재 상태가 STOP_RUN이 아니라면? => STOP RUN
        if (eRunType != ERunType::STOP_RUN_L)
        {
            m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::STOP_RUN_L;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
            return;
        }
        // Stop Run 이면서 애니메이션 재생이 끝났다면?.
        if ((eRunType == ERunType::STOP_RUN_L) && m_IsAnimationEnd)
        {
            m_pAugusta->GetStateContextForWrite().m_eIdleType = EIdleType::STAND1_ACTION01;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
            return;
        }
    }
    if (m_pAugusta->Check_AnyInput(m_iMoveKey))
    {
        // 이동 값이 들어왔는데 Stop Run 상태라면?
        if (eRunType == ERunType::STOP_RUN_L || eRunType == ERunType::RUN_F || eRunType == ERunType::RUN_BASEPOSE)
        {
            m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_F;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
            return;
        }
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
