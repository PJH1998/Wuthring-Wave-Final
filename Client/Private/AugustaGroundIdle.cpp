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

    //m_pAugusta->m_StateContext.Clear();
    
    // 1. 복사본 Context 받아오기
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EIdleType eIdleType = context.m_eIdleType;


}

void CAugustaGroundIdle::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 1. 현재 애니메이션 재생
    m_IsAnimationEnd = CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // 2. Idle 내부 애니메이션 전환 ( 랜덤하게 Action 재생)
    if (m_IsAnimationEnd)
        m_iCurrentAnimIdx = (m_iCurrentAnimIdx + 1) % static_cast<_uint>(EIdleType::END);

    // 3. 다른 State로 전환 체크
    Check_StateTransition();
}

void CAugustaGroundIdle::OnExit()
{
    CGroundState::OnExit();
}

void CAugustaGroundIdle::Setup_Animations()
{
    CState::Add_Animations(static_cast<_uint>(EIdleType::STAND1_ACTION01), "Stand1_Action01", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(EIdleType::STAND1_ACTION02), "Stand1_Action02", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(EIdleType::STAND1_ACTION03), "Stand1_Action03", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(EIdleType::STAND1_TURN_L90D), "Stand1_Turn_L90D", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(EIdleType::STAND1_TURN_R90D), "Stand1_Turn_R90D", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(EIdleType::STAND2), "Stand2", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(EIdleType::STAND_CONTROL), "Stand_Control", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(EIdleType::STANDCHANGE), "StandChange", 1.f, 0.f);
    CState::Add_Animations(static_cast<_uint>(EIdleType::STANDUP), "StandUp", 1.f, 0.f);
}

void CAugustaGroundIdle::Check_StateTransition()
{
    
    // 1. Lock On일때
    if (m_pAugusta->Is_LockOn())
    {

    }
    // 2. 아닐 때
    else
    { 
        if (m_pAugusta->Check_AnyInput(m_iMoveKey))
        {
            m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN)); // 상위, 하위 상태
            return;
        }
    }
   

    
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
    __super::Free();
}
