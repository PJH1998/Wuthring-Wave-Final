#include "ClientPch.h"
#include "AugustaGroundDash.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"


HRESULT CAugustaGroundDash::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pCharacter);
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

	// 4. State 초기화.
    State_Reset();
	m_pAugusta->Set_Gravity(true);

	// 5. 무적
	//m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
}

void CAugustaGroundDash::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키 입력
    Handle_Input();

    // 1. 애니메이션 실행
	Update_DashAnimation(fTimeDelta);
    
    // 2. 상태 제어.
    Check_StateTransition(fTimeDelta);

    // 3. 상태 초기화.
    State_Reset();
}

void CAugustaGroundDash::OnExit()
{
    CGroundState::OnExit();
	// 무적 제거.
	//m_pAugusta->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
}

void CAugustaGroundDash::Handle_Input()
{

	// Dash 키입력 체크.
	m_States[DASH] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));
	m_States[HIT] = m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT)); // HIT 상태인가?
	m_States[DODGE] = m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));
	//m_States[DODGE] = m_States[DODGEABLE] // Dodge 가능하면서 Dash 키 누르면?

	if (m_States[DODGE] || m_States[HIT]) // 모든 조건 상위 조건
		return;

    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
	m_States[SPRINT] = m_States[MOVE] && m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);

	
}



void CAugustaGroundDash::Update_DashAnimation(_float fTimeDelta)
{
    

  //  // 2. LockOn 상태일때는 현재 방향에서 누른 방향을 바라보게 수정.
  //  if (m_pAugusta->Is_LockOn())
  //  {
  //      _vector vMoveDir = m_pAugusta->Calculate_Move_Direction(m_eDir);
		//m_pAugusta->Rotate_Direction(vMoveDir);
  //  }

	// 1. 누른키에 따른 방향 계산
	m_eDir = m_pAugusta->Calculate_Direction();
	// 2. Animation 실행
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

	
    
}

void CAugustaGroundDash::Check_StateTransition(_float fTimeDelta)
{
    EAugustaDashType eDashType = static_cast<EAugustaDashType>(m_iCurrentAnimIdx);

	// 1. 우선순위
	if (m_States[DODGE])
	{
		// 맞은 방향에 따라서 애니메이션 선택. => 후방에서 맞으면 MOVE_LIMIT_F
		// 맞은 방향에 따라서 애니메이션 선택. => 전방에서 맞으면 MOVE_LIMIT_B

		const CCharacter::HIT_DESC* pDesc = m_pAugusta->GetPendingHitDesc();
		if (nullptr == pDesc)
			return;

		//if (pDesc->IsBack)
		//	m_pAugusta->GetStateContextForWrite().m_eDodgeType = EAugustaDodgeType::MOVE_LIMIT_F;
		//else 
		//	m_pAugusta->GetStateContextForWrite().m_eDodgeType = EAugustaDodgeType::MOVE_LIMIT_B;

		m_pAugusta->GetStateContextForWrite().m_eDodgeType = EAugustaDodgeType::MOVE_LIMIT_F;
		
		// 맞은 방향으로 한번 회전.
		//m_pAugusta->Rotate_Target(); 

		// Dodge 이전에 누른 방향으로 회전.
		_vector vMoveDir = m_pAugusta->Calculate_Move_Direction(m_eDir);
		m_pAugusta->Rotate_Direction(vMoveDir);

		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::DODGE)); // 상위, 하위 상태
		
		return;
	}

	// 2.
	if (m_States[HIT])
	{
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EAugustaHitState::HIT));
		return;
	}

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

			if (m_States[SPRINT])
			{
				m_pAugusta->GetStateContextForWrite().m_eSprintType = EAugustaSprintType::SPRINT_F;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SPRINT)); // 상위, 하위 상태
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
		m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION02;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE)); // 상위, 하위 상태
		return;
	}

}

void CAugustaGroundDash::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaDashType::MOVE_F), "Move_F", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaDashType::MOVE_B), "Move_B", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaDashType::MOVE_LIMIT_B), "Move_Limit_B", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaDashType::MOVE_LIMIT_F), "Move_Limit_F", 1.f, 0.f);
}

void CAugustaGroundDash::State_Reset()
{
    for (_uint i = 0; i < DASHSTATE::END; ++i)
        m_States[i] = false;
}

CAugustaGroundDash* CAugustaGroundDash::Create(CCharacter* pOwner)
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
