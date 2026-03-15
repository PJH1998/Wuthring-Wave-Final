#include "ClientPch.h"
#include "AugustaRopeHook.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

// 이건 이동 Rope 로만 사용하자.
HRESULT CAugustaRopeHook::Initialize(CCharacter* pCharacter)
{
	if (FAILED(CInteractionState::Initialize(pCharacter)))
		return E_FAIL;

	m_pAugusta = dynamic_cast<CAugusta*>(pCharacter);
	ASSERT_CRASH(m_pAugusta);

	Setup_Animations();
	return S_OK;
}

void CAugustaRopeHook::OnEnter(void* pArg)
{
	CInteractionState::OnEnter(pArg);

	m_eRopeDir = m_pAugusta->Calculate_RopeDirection();

	m_eRopeStep = ROPESTEP::STEP_START;

	switch (m_eRopeDir)
	{
	case ROPEDIR::U:
		m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_START01_U);
		break;
	case ROPEDIR::F:
		m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_START01_F);
		break;
	case ROPEDIR::D:
		m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_START01_D);
		break;
	}

	State_Reset();

	m_pAugusta->Rotate_GrappleTarget();
	m_pAugusta->Set_Gravity(false);
	m_pAugusta->Rope_Active(true);
	m_pAugusta->Spwan_RopeEffect(TEXT("Rope"), "WeaponProp01");
}

void CAugustaRopeHook::OnUpdate(_float fTimeDelta)
{
	CInteractionState::OnUpdate(fTimeDelta);

	Handle_Input();
	Update_RopeAnimation(fTimeDelta);
	Check_Physics(fTimeDelta);
	Check_StateTransition(fTimeDelta);
	State_Reset();
}

void CAugustaRopeHook::OnExit()
{
	CInteractionState::OnExit();
	m_pAugusta->Set_Gravity(false);
	m_eRopeDir = ROPEDIR::END;
	m_eRopeStep = ROPESTEP::STEP_NONE;

	m_pAugusta->Rope_Active(false); // Rope Active 종료.
}



void CAugustaRopeHook::Enter_Rope()
{
	// 1. Rope Target 위치로 회전하기 . Look Y도 돌아가야한다.

	// 2. 목표 벡터 위치로 이동하기? 서서히 => 받았을때 한번만 받기.
}

void CAugustaRopeHook::Handle_Input()
{
	m_eDir = m_pAugusta->Calculate_Direction(); // 방향 계산.
	m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
	m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));

}

void CAugustaRopeHook::Update_RopeAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
	EAugustaRopeHookType eRopeType = static_cast<EAugustaRopeHookType>(m_iCurrentAnimIdx);

	// 이 경우에만 이동?
	if (m_eRopeStep == ROPESTEP::STEP_START || m_eRopeStep == ROPESTEP::STEP_START2)
		m_pAugusta->Move_Grapple(fTimeDelta, 0.2f); // 이동.

	if (m_eRopeStep == ROPESTEP::STEP_LOOP)
		m_pAugusta->Move_Grapple(fTimeDelta, 2.f); // 이동.
	
}

void CAugustaRopeHook::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);
	m_States[REACHED] = m_pAugusta->Is_ReachedGrappleHook();
}

void CAugustaRopeHook::Check_StateTransition(_float fTimeDelta)
{
	EAugustaRopeHookType eRopeType = static_cast<EAugustaRopeHookType>(m_iCurrentAnimIdx);
	_bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. Step Start 인경우? => Start2로 변경.
	if (m_eRopeStep == ROPESTEP::STEP_START)
	{
		if (m_IsAnimationEnd)
		{
			switch (eRopeType)
			{
			case EAugustaRopeHookType::FIXHOOK_START01_U:
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_START02_U);
				m_eRopeStep = ROPESTEP::STEP_START2;
				break;
			case EAugustaRopeHookType::FIXHOOK_START01_D:
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_START02_D);
				m_eRopeStep = ROPESTEP::STEP_START2;
				break;
			case EAugustaRopeHookType::FIXHOOK_START01_F:
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_START02_F);
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
			case EAugustaRopeHookType::FIXHOOK_START02_U:
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_LOOP_U);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			case EAugustaRopeHookType::FIXHOOK_START02_F:
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_LOOP_F);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			case EAugustaRopeHookType::FIXHOOK_START02_D:
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_LOOP_D);
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
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_END);
				m_eRopeStep = ROPESTEP::STEP_END;

				m_pAugusta->Rope_Active(false); // Rope Active 종료.
				return;
			}

			if (m_States[LAND])
			{
				if (m_States[MOVE])
				{
					m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
					return;
				}
				

				if (m_States[JUMP])
				{
					m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F;
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
					return;
				}
				else
				{
					m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STANDCHANGE;
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
					return;
				}
			}

			if (!m_States[LAND])
			{
				if (m_States[JUMP])
				{
					m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F;
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
					return;
				}
			}
		}

		if (m_IsAnimationEnd && ROPESTEP::STEP_END == m_eRopeStep)
		{
			if (!m_States[LAND])
			{
				m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL));
				return;
			}
		}
	}

	
}


void CAugustaRopeHook::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_END), "FixHook_End", 2.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_END_FAST), "FixHook_End_Fast", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_LOOP_D), "FixHook_Loop_D", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_LOOP_F), "FixHook_Loop_F", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_LOOP_L), "FixHook_Loop_L", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_LOOP_R), "FixHook_Loop_R", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_LOOP_U), "FixHook_Loop_U", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_START01_D), "FixHook_Start01_D", 3.5f, 16.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_START01_F), "FixHook_Start01_F", 3.5f, 16.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_START01_U), "FixHook_Start01_U", 3.5f, 16.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_START02_D), "FixHook_Start02_D", 3.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_START02_F), "FixHook_Start02_F", 3.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::FIXHOOK_START02_U), "FixHook_Start02_U", 3.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeHookType::HOOK_UP), "Hook_Up", 1.f, 0.f);

}

void CAugustaRopeHook::State_Reset()
{
	for (_uint i = 0; i < ROPESTATE::END; ++i)
		m_States[i] = false;
}



CAugustaRopeHook* CAugustaRopeHook::Create(CCharacter* pOwner)
{
	CAugustaRopeHook* pInstance = new CAugustaRopeHook();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CAugustaRopeHook");
		return nullptr;
	}

	return pInstance;
}

void CAugustaRopeHook::Free()
{
	CInteractionState::Free();
}
