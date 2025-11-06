#include "ClientPch.h"
#include "AugustaGroundDash.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"


HRESULT CAugustaGroundDash::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();

    return S_OK;
}

void CAugustaGroundDash::OnEnter(void* pArg)
{
    // 상위 객체 수행 작업.
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaDashType EDashType = context.m_eDashType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eDashType);

    State_Reset();

	m_pAugusta->Set_Gravity(true);
}

void CAugustaGroundDash::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키 입력
    Handle_Input();

    // 1. 애니메이션 실행
    Update_SprintAnimation(fTimeDelta);
    
    // 2. 상태 제어.
    /*if (m_pAugusta->Is_LockOn())
        LockOnCheck_StateTransition(fTimeDelta);
    else       
        Check_StateTransition(fTimeDelta);*/
    Check_StateTransition(fTimeDelta);

    // 3. 상태 초기화.
    State_Reset();
}

void CAugustaGroundDash::OnExit()
{
    CGroundState::OnExit();
}

void CAugustaGroundDash::Handle_Input()
{
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
	m_States[LAND] = m_pAugusta->Is_Land();
}



void CAugustaGroundDash::Update_SprintAnimation(_float fTimeDelta)
{
    // 1. 누른키에 따른 방향 계산
    m_eDir = m_pAugusta->Calculate_Direction();

    // 2. LockOn 상태일때는 현재 방향에서 누른 방향을 바라보게 수정.
    if (m_pAugusta->Is_LockOn())
    {
        _vector vMoveDir = m_pAugusta->Calculate_Move_Direction(m_eDir);
        //m_pAugusta->Rotate_Direction(vMoveDir);
    }
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
    
}

void CAugustaGroundDash::Check_StateTransition(_float fTimeDelta)
{
    EAugustaDashType eDashType = static_cast<EAugustaDashType>(m_iCurrentAnimIdx);

	if (!m_States[LAND])
	{
		m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
		return;
	}

 
  

    switch (eDashType)
    {
    case EAugustaDashType::MOVE_F:
    {
        if (CState::Is_EscapePossible())
        {
            if (m_States[JUMP])
            {
                m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_WALK_LF;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP)); // 상위, 하위 상태
                return;
            }

            if (m_States[MOVE])
            {
                m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN)); // 상위, 하위 상태
                return;
            }
        }
    }
        break;
    }

	// 애니메이션 끝나면?
	if (m_IsAnimationEnd)
	{
		m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE)); // 상위, 하위 상태
		return;
	}

}

void CAugustaGroundDash::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaDashType::MOVE_F), "Move_F", 1.f, 20.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaDashType::MOVE_B), "Move_B", 1.f, 20.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaDashType::MOVE_LIMIT_B), "Move_Limit_B", 30.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaDashType::MOVE_LIMIT_F), "Move_Limit_F", 30.f, 0.f);
}

void CAugustaGroundDash::State_Reset()
{
    for (_uint i = 0; i < DASHSTATE::END; ++i)
        m_States[i] = false;
}

CAugustaGroundDash* CAugustaGroundDash::Create(class CGameObject* pOwner)
{
    CAugustaGroundDash* pInstance = new CAugustaGroundDash();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundDash");
    }

    return pInstance;
}

void CAugustaGroundDash::Free()
{
    CGroundState::Free();
}
