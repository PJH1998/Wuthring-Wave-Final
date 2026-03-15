#include "ClientPch.h"
#include "RoverRopeHook.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"

// 이건 이동 Rope 로만 사용하자.
HRESULT CRoverRopeHook::Initialize(CCharacter* pCharacter)
{
	if (FAILED(CInteractionState::Initialize(pCharacter)))
		return E_FAIL;

	m_pRover = dynamic_cast<CRover*>(pCharacter);
	ASSERT_CRASH(m_pRover);

	Setup_Animations();
	return S_OK;
}

void CRoverRopeHook::OnEnter(void* pArg)
{
	CInteractionState::OnEnter(pArg);

	m_eRopeDir = m_pRover->Calculate_RopeDirection();
	m_eRopeStep = ROPESTEP::STEP_START;

	switch (m_eRopeDir)
	{
	case ROPEDIR::U:
		m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeHookType::FIXHOOK_START01_U);
		break;
	case ROPEDIR::F:
		m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeHookType::FIXHOOK_START01_D);
		break;
	case ROPEDIR::D:
		m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeHookType::FIXHOOK_START01_F);
		break;
	}
	State_Reset();
	m_pRover->Rotate_GrappleTarget();
	m_pRover->Set_Gravity(false);
	m_pRover->Rope_Active(true);
	m_pRover->Spwan_RopeEffect(TEXT("Rope"), "WeaponProp01");
}

void CRoverRopeHook::OnUpdate(_float fTimeDelta)
{
	CInteractionState::OnUpdate(fTimeDelta);

	// 0. 키입력 체크
	Handle_Input();

	// 1. 애니메이션 갱신
	Update_RopeAnimation(fTimeDelta);

	// 2. 물리 체크
	Check_Physics(fTimeDelta);

	// 3. 전환 체크
	Check_StateTransition(fTimeDelta);

	// 상태 리셋;
	State_Reset();

	//m_pRover->Rotate_GrappleTarget(); // 회전.
}

void CRoverRopeHook::OnExit()
{
	CInteractionState::OnExit();
	m_pRover->Set_Gravity(false);
	m_eRopeDir = ROPEDIR::END;
	m_eRopeStep = ROPESTEP::STEP_NONE;

	m_pRover->Rope_Active(false); // Rope Active 종료.
}



void CRoverRopeHook::Enter_Rope()
{
	// 1. Rope Target 위치로 회전하기 . Look Y도 돌아가야한다.

	// 2. 목표 벡터 위치로 이동하기? 서서히 => 받았을때 한번만 받기.
}

void CRoverRopeHook::Handle_Input()
{
	m_eDir = m_pRover->Calculate_Direction(); // 방향 계산.
	m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
	m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));

}

void CRoverRopeHook::Update_RopeAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pRover, fTimeDelta);
	ERoverRopeHookType eRopeType = static_cast<ERoverRopeHookType>(m_iCurrentAnimIdx);

	// 이 경우에만 이동?
	if (m_eRopeStep == ROPESTEP::STEP_START || m_eRopeStep == ROPESTEP::STEP_START2)
		m_pRover->Move_Grapple(fTimeDelta, 0.2f); // 이동.

	if (m_eRopeStep == ROPESTEP::STEP_LOOP)
		m_pRover->Move_Grapple(fTimeDelta, 2.f); // 이동.
	
}

void CRoverRopeHook::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
	m_States[REACHED] = m_pRover->Is_ReachedGrappleHook();
}

void CRoverRopeHook::Check_StateTransition(_float fTimeDelta)
{
	ERoverRopeHookType eRopeType = static_cast<ERoverRopeHookType>(m_iCurrentAnimIdx);
	_bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. Step Start 인경우? => Start2로 변경.
	if (m_eRopeStep == ROPESTEP::STEP_START)
	{
		if (m_IsAnimationEnd)
		{
			switch (eRopeType)
			{
			case ERoverRopeHookType::FIXHOOK_START01_U:
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeHookType::FIXHOOK_START02_U);
				m_eRopeStep = ROPESTEP::STEP_START2;
				break;
			case ERoverRopeHookType::FIXHOOK_START01_D:
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeHookType::FIXHOOK_START02_D);
				m_eRopeStep = ROPESTEP::STEP_START2;
				break;
			case ERoverRopeHookType::FIXHOOK_START01_F:
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeHookType::FIXHOOK_START02_F);
				m_eRopeStep = ROPESTEP::STEP_START2;
				break;
			default:
				break;
			}
		}
		return;
	}

	if (m_eRopeStep == ROPESTEP::STEP_START2)
	{
		if (m_IsAnimationEnd)
		{
			switch (eRopeType)
			{
			case ERoverRopeHookType::FIXHOOK_START02_U:
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeHookType::FIXHOOK_LOOP_U);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			case ERoverRopeHookType::FIXHOOK_START02_F:
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeHookType::FIXHOOK_LOOP_F);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			case ERoverRopeHookType::FIXHOOK_START02_D:
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeHookType::FIXHOOK_LOOP_D);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			default:
				break;
			}
			return;
		}
	}



	// Step이 Loop이거나 End인 경우. 다른 State로 변경 가능.
	if (m_eRopeStep == ROPESTEP::STEP_LOOP || m_eRopeStep == ROPESTEP::STEP_END)
	{
		if (IsEscapePossible)
		{
			// Loop 가 End로 변하는 조건은?
			if ((m_eRopeStep == ROPESTEP::STEP_LOOP) && m_States[REACHED])
			{
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeHookType::FIXHOOK_END);
				m_eRopeStep = ROPESTEP::STEP_END;

				m_pRover->Rope_Active(false); // Rope Active 종료.
				return;
			}

			if (m_States[LAND])
			{
				if (m_States[MOVE])
				{
					m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
					m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
					return;
				}

				if (m_States[JUMP])
				{
					m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_SECOND_F;
					m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP));
					return;
				}
				else
				{
					m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STANDCHANGE;
					m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
					return;
				}
			}

			if (!m_States[LAND])
			{
				if (m_States[JUMP])
				{
					m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_SECOND_F;
					m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP));
					return;
				}
			}
		}

		if (m_IsAnimationEnd && ROPESTEP::STEP_END == m_eRopeStep)
		{
			if (!m_States[LAND])
			{
				m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL));
				return;
			}
		}
	}

	
}


void CRoverRopeHook::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_END), "FixHook_End", 2.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_END_FAST), "FixHook_End_Fast", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_LOOP_D), "FixHook_Loop_D", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_LOOP_F), "FixHook_Loop_F", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_LOOP_L), "FixHook_Loop_L", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_LOOP_R), "FixHook_Loop_R", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_LOOP_U), "FixHook_Loop_U", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_START01_D), "FixHook_Start01_D", 3.5f, 16.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_START01_F), "FixHook_Start01_F", 3.5f, 16.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_START01_U), "FixHook_Start01_U", 3.5f, 16.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_START02_D), "FixHook_Start02_D", 3.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_START02_F), "FixHook_Start02_F", 3.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::FIXHOOK_START02_U), "FixHook_Start02_U", 3.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeHookType::HOOK_UP), "Hook_Up", 1.f, 0.f);

}

void CRoverRopeHook::State_Reset()
{
	for (_uint i = 0; i < ROPESTATE::END; ++i)
		m_States[i] = false;
}



CRoverRopeHook* CRoverRopeHook::Create(CCharacter* pOwner)
{
	CRoverRopeHook* pInstance = new CRoverRopeHook();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CRoverRopeHook");
		return nullptr;
	}

	return pInstance;
}

void CRoverRopeHook::Free()
{
	CInteractionState::Free();
}
