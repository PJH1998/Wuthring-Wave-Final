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

    // 1. 입력에 따라 애니메이션 선택
    Update_RunAnimation();

    // 2. 선택된 애니메이션 재생
    m_IsAnimationEnd = CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // 3. 다른 State로 전환 체크
    Check_StateTransition();
}

void CAugustaGroundRun::OnExit()
{
    CGroundState::OnExit();
}

void CAugustaGroundRun::Setup_Animations()
{
    CState::Add_Animations(static_cast<_uint>(ERunType::RUN_B), "Run_B", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(ERunType::RUN_BASEPOSE), "Run_BasePose", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(ERunType::RUN_F), "Run_F", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(ERunType::RUN_LB), "Run_LB", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(ERunType::RUN_LF), "Run_LF", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(ERunType::RUN_POSE_F), "Run_Pose_F", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(ERunType::RUN_POSE_L), "Run_Pose_L", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(ERunType::RUN_POSE_R), "Run_Pose_R", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(ERunType::RUN_RB), "Run_RB", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(ERunType::RUN_RF), "Run_RF", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(ERunType::RUN_TURNBACK), "Run_Turnback", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(ERunType::STOP_RUN_L), "Stop_Run_L", 1.f, 0.f);
    //CState::Add_Animations(static_cast<_uint>(ERunType::STOP_RUN_R), "Stop_Run_R", 1.f, 0.f);
}

void CAugustaGroundRun::Update_RunAnimation()
{
    // TODO: WASD 입력에 따라 애니메이션 전환
    // W만           → Run_F
    // S만           → Run_B
    // W + A         → Run_LF
    // W + D         → Run_RF
    // S + A         → Run_LB
    // S + D         → Run_RB

    // 현재는 기본 Run_F 유지
   
}

void CAugustaGroundRun::Check_StateTransition()
{

    if (m_pAugusta->Is_LockOn())
    {
        // 1. Lock On일때

    }
    else
    {
        // 2. Lock On 아닐 때

        // 이동 상태를 아래에 몰아둔다. => 우선 순위 낮음
        // 이동 입력 값이 안들어왔다면?
        if (!m_pAugusta->Check_AnyInput(m_iMoveKey))
        {
            // 현재 상태가 STOP_RUN이 아니라면? => STOP RUN
            if (m_iCurrentAnimIdx != static_cast<_uint>(ERunType::STOP_RUN_L))
            {
                m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::STOP_RUN_L;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
                return;
            }
            // Stop Run 이면서 애니메이션 재생이 끝났다면?.
            if (m_iCurrentAnimIdx == static_cast<_uint>(ERunType::STOP_RUN_L) && m_IsAnimationEnd)
            {
                m_pAugusta->GetStateContextForWrite().m_eIdleType = EIdleType::STAND1_ACTION01;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
                return;
            }
        }
        else
        {
            if (m_iCurrentAnimIdx == static_cast<_uint>(ERunType::STOP_RUN_L))
            {
                m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_F;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
            }
        }

        
    }
    //// W키 입력 없으면 Idle로 (HSM: enum 기반)
    //if (!m_pAugusta->Check_AnyInput(1 << ENUM_CLASS(KEYINPUT::W)))
    //{

    //}
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
