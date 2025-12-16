#include "ClientPch.h"
#include "GalbrenaRopeHook.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"

// 이건 이동 Rope 로만 사용하자.
HRESULT CGalbrenaRopeHook::Initialize(CCharacter* pCharacter)
{
	if (FAILED(CInteractionState::Initialize(pCharacter)))
		return E_FAIL;

	m_pGalbrena = dynamic_cast<CGalbrena*>(pCharacter);
	ASSERT_CRASH(m_pGalbrena);

	Setup_Animations();
	return S_OK;
}

void CGalbrenaRopeHook::OnEnter(void* pArg)
{
	CInteractionState::OnEnter(pArg);

	// 1. 애니메이션 결정을 위한 방향 설정.
	m_eRopeDir = m_pGalbrena->Calculate_RopeDirection();

	// 2. 초기 단계 설정.
	/*m_eRopeStep = ROPESTEP::STEP_START;

	switch (m_eRopeDir)
	{
	case ROPEDIR::U:
		m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START01_U);
		break;
	case ROPEDIR::F:
		m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START01_F);
		break;
	case ROPEDIR::D:
		m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START01_D);
		break;
	}*/

	m_eRopeStep = ROPESTEP::STEP_START2;

	switch (m_eRopeDir)
	{
	case ROPEDIR::U:
		m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START02_U);
		break;
	case ROPEDIR::F:
		m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START02_F);
		break;
	case ROPEDIR::D:
		m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START02_D);
		break;
	}

	// 4. 상태 리셋.
	State_Reset();

	// 5. Description을 이용하여 시작 초기 작업을 정의합니다.
	//Enter_Rope();

	// 6. 나중에 감지된 위치에 있는 방향으로 회전합니다. 
	// 추후에는 => Look이 y도 돌아가야함.
	m_pGalbrena->Rotate_GrappleTarget();

	// 6. 중력 끄기
	m_pGalbrena->Set_Gravity(false);

	// 7. Condition 추가. => Condition 체크할때 ROPE_HOOK DRAG에 따라서 판별합니다.
	m_pGalbrena->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::ROPE_HOOK));

	// 8. Rope Efeect 생성
	m_pGalbrena->Rope_Active(true);
	m_pGalbrena->Spwan_RopeEffect(TEXT("Rope"), "WeaponProp01");
}

void CGalbrenaRopeHook::OnUpdate(_float fTimeDelta)
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

	//m_pGalbrena->Rotate_GrappleTarget(); // 회전.
}

void CGalbrenaRopeHook::OnExit()
{
	CInteractionState::OnExit();
	m_pGalbrena->Set_Gravity(false);
	m_eRopeDir = ROPEDIR::END;
	m_eRopeStep = ROPESTEP::STEP_NONE;

	m_pGalbrena->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::ROPE_HOOK));
	m_pGalbrena->Rope_Active(false); // Rope Active 종료.
}



void CGalbrenaRopeHook::Enter_Rope()
{
	// 1. Rope Target 위치로 회전하기 . Look Y도 돌아가야한다.

	// 2. 목표 벡터 위치로 이동하기? 서서히 => 받았을때 한번만 받기.
}

void CGalbrenaRopeHook::Handle_Input()
{
	m_eDir = m_pGalbrena->Calculate_Direction(); // 방향 계산.
	m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
	m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));

}

void CGalbrenaRopeHook::Update_RopeAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);
	EGalbrenaRopeHookType eRopeType = static_cast<EGalbrenaRopeHookType>(m_iCurrentAnimIdx);

	// 이 경우에만 이동?
	if (m_eRopeStep == ROPESTEP::STEP_START || m_eRopeStep == ROPESTEP::STEP_START2)
		m_pGalbrena->Move_Grapple(fTimeDelta, 0.2f); // 이동.

	if (m_eRopeStep == ROPESTEP::STEP_LOOP)
		m_pGalbrena->Move_Grapple(fTimeDelta, 2.f); // 이동.
	
}

void CGalbrenaRopeHook::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
	m_States[REACHED] = m_pGalbrena->Is_ReachedGrappleHook();
}

void CGalbrenaRopeHook::Check_StateTransition(_float fTimeDelta)
{
	EGalbrenaRopeHookType eRopeType = static_cast<EGalbrenaRopeHookType>(m_iCurrentAnimIdx);
	_bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. Step Start 인경우? => Start2로 변경.
	if (m_eRopeStep == ROPESTEP::STEP_START)
	{
		if (m_IsAnimationEnd)
		{
			switch (eRopeType)
			{
			case EGalbrenaRopeHookType::FIXHOOK_START01_U:
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START02_U);
				m_eRopeStep = ROPESTEP::STEP_START2;
				break;
			case EGalbrenaRopeHookType::FIXHOOK_START01_D:
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START02_D);
				m_eRopeStep = ROPESTEP::STEP_START2;
				break;
			case EGalbrenaRopeHookType::FIXHOOK_START01_F:
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START02_F);
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
			case EGalbrenaRopeHookType::FIXHOOK_START02_U:
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_LOOP_U);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			case EGalbrenaRopeHookType::FIXHOOK_START02_F:
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_LOOP_F);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			case EGalbrenaRopeHookType::FIXHOOK_START02_D:
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_LOOP_D);
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
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_END);
				m_eRopeStep = ROPESTEP::STEP_END;
				m_pGalbrena->Rope_Active(false);
				return;
			}

			if (m_States[LAND])
			{
				if (m_States[MOVE])
				{
					m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
					return;
				}
				

				if (m_States[JUMP])
				{
					m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F;
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
					return;
				}
				else
				{
					m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STANDCHANGE;
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
					return;
				}
			}

			if (!m_States[LAND])
			{
				if (m_States[JUMP])
				{
					m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F;
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
					return;
				}
			}
		}

		if (m_IsAnimationEnd && ROPESTEP::STEP_END == m_eRopeStep)
		{
			if (!m_States[LAND])
			{
				m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL));
				return;
			}
		}
	}

	
}


void CGalbrenaRopeHook::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_END), "FixHook_End", 2.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_END_FAST), "FixHook_End_Fast", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_LOOP_D), "FixHook_Loop_D", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_LOOP_F), "FixHook_Loop_F", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_LOOP_L), "FixHook_Loop_L", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_LOOP_R), "FixHook_Loop_R", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_LOOP_U), "FixHook_Loop_U", 3.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START01_D), "FixHook_Start01_D", 3.5f, 16.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START01_F), "FixHook_Start01_F", 3.5f, 16.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START01_U), "FixHook_Start01_U", 3.5f, 16.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START02_D), "FixHook_Start02_D", 3.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START02_F), "FixHook_Start02_F", 3.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::FIXHOOK_START02_U), "FixHook_Start02_U", 3.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeHookType::HOOK_UP), "Hook_Up", 1.f, 0.f);

}

void CGalbrenaRopeHook::State_Reset()
{
	for (_uint i = 0; i < ROPESTATE::END; ++i)
		m_States[i] = false;
}



CGalbrenaRopeHook* CGalbrenaRopeHook::Create(CCharacter* pOwner)
{
	CGalbrenaRopeHook* pInstance = new CGalbrenaRopeHook();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CGalbrenaRopeHook");
		return nullptr;
	}

	return pInstance;
}

void CGalbrenaRopeHook::Free()
{
	CInteractionState::Free();
}
