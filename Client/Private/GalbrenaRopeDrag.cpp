#include "ClientPch.h"
#include "GalbrenaRopeDrag.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"

// 이건 이동 Rope 로만 사용하자.
HRESULT CGalbrenaRopeDrag::Initialize(CCharacter* pCharacter)
{
	if (FAILED(CInteractionState::Initialize(pCharacter)))
		return E_FAIL;

	m_pGalbrena = dynamic_cast<CGalbrena*>(pCharacter);
	ASSERT_CRASH(m_pGalbrena);

	Setup_Animations();
	return S_OK;
}

void CGalbrenaRopeDrag::OnEnter(void* pArg)
{
	CInteractionState::OnEnter(pArg);

	// 1. 애니메이션 결정을 위한 방향 설정.
	m_eRopeDir = m_pGalbrena->Calculate_RopeDirection();

	// 2. 초기 단계 설정.
	m_eRopeStep = ROPESTEP::STEP_START;

	switch (m_eRopeDir)
	{
	case ROPEDIR::U:
		m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeDragType::DRAG_START_U);
		break;
	case ROPEDIR::F:
		m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeDragType::DRAG_START_F);
		break;
	case ROPEDIR::D:
		m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeDragType::DRAG_START_D);
		break;
	}

	// 4. 상태 리셋.
	State_Reset();

	// 5. 나중에 감지된 위치에 있는 방향으로 회전합니다. 
	m_pGalbrena->Rotate_GrappleTarget();

	// 6. 중력 끄기
	m_pGalbrena->Set_Gravity(false);

	// 7. 현재 상태 부여. =>
	m_pGalbrena->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::ROPE_DRAG));

	// 8. Rope Efeect 생성
	m_pGalbrena->Rope_Active(true);
	m_pGalbrena->Spwan_RopeEffect(TEXT("Rope"), "WeaponProp02");
}

void CGalbrenaRopeDrag::OnUpdate(_float fTimeDelta)
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

void CGalbrenaRopeDrag::OnExit()
{
	CInteractionState::OnExit();
	m_pGalbrena->Set_Gravity(false);
	m_eRopeDir = ROPEDIR::END;
	m_eRopeStep = ROPESTEP::STEP_NONE;
	
	m_pGalbrena->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::ROPE_DRAG));
	m_pGalbrena->Rope_Active(false);
}



void CGalbrenaRopeDrag::Enter_Rope()
{

}

void CGalbrenaRopeDrag::Handle_Input()
{
	m_eDir = m_pGalbrena->Calculate_Direction(); // 방향 계산.
	m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
	m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
	m_States[DRAG] = (m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::T))) 
		&& (m_eRopeStep == ROPESTEP::STEP_LOOP);// Loop 상태일때만 당길 수있다.

}

void CGalbrenaRopeDrag::Update_RopeAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);
}

void CGalbrenaRopeDrag::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
	m_States[REACHED] = m_pGalbrena->Is_ReachedGrappleHook();
}

void CGalbrenaRopeDrag::Check_StateTransition(_float fTimeDelta)
{
	EGalbrenaRopeDragType eRopeType = static_cast<EGalbrenaRopeDragType>(m_iCurrentAnimIdx);
	_bool IsEscapePossible = CState::Is_EscapePossible();


	// 1. Step Start 인경우? => Start2로 변경.
	if (m_eRopeStep == ROPESTEP::STEP_START)
	{
		if (IsEscapePossible)
		{

			switch (eRopeType)
			{
			case EGalbrenaRopeDragType::DRAG_START_U:
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeDragType::DRAG_LOOP_U);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			case EGalbrenaRopeDragType::DRAG_START_D:
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeDragType::DRAG_LOOP_D);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			case EGalbrenaRopeDragType::DRAG_START_F:
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeDragType::DRAG_LOOP_F);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			default:
				break;
			}
		}
		return;
	}

	
	if (m_eRopeStep == ROPESTEP::STEP_LOOP) 
	{
		// 탈출이 가능한 경우?
		if (IsEscapePossible)
		{
			// Drag 상태.
			if (m_States[DRAG])
			{
				m_pGalbrena->Execute_RopeDragTrigger(); // 연결된 객체의 Trigger 호출.
				m_eRopeStep = ROPESTEP::STEP_END;
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaRopeDragType::DRAG_END);
				m_pGalbrena->Rope_Active(false); // Rope Active 종료.
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
			}

			if (!m_States[LAND])
			{
				if (m_States[JUMP])
				{
					m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_WALK_LF;
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
					return;
				}
			}
		}
	}


	// Step이 Loop이거나 End인 경우. 다른 State로 변경 가능.
	if (m_eRopeStep == ROPESTEP::STEP_END)
	{
		if (IsEscapePossible)
		{
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
			}

			if (!m_States[LAND]) // 땅이 아니라면?
			{
				if (m_States[JUMP])
				{
					m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F;
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
					return;
				}
			}
		}

		if (m_IsAnimationEnd) // 애니메이션이 종료되면?
		{
			if (m_States[LAND])
			{
				m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STANDCHANGE;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
				return;
			}

			if (!m_States[LAND])
			{
				m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL));
				return;
			}
		}

		
	}

	
}


void CGalbrenaRopeDrag::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeDragType::DRAG_START_U), "Drag_Start_U", 1.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeDragType::DRAG_START_F), "Drag_Start_F", 1.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeDragType::DRAG_START_D), "Drag_Start_D", 1.5f, 20.f);

	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeDragType::DRAG_LOOP_U), "Drag_Loop_U", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeDragType::DRAG_LOOP_F), "Drag_Loop_F", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeDragType::DRAG_LOOP_D), "Drag_Loop_D", 1.f, 0.f);

	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeDragType::DRAG_END), "Drag_End", 1.5f, 100.f);
	

}

void CGalbrenaRopeDrag::State_Reset()
{
	for (_uint i = 0; i < ROPESTATE::END; ++i)
		m_States[i] = false;
}



CGalbrenaRopeDrag* CGalbrenaRopeDrag::Create(CCharacter* pOwner)
{
	CGalbrenaRopeDrag* pInstance = new CGalbrenaRopeDrag();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CGalbrenaRopeDrag");
		return nullptr;
	}

	return pInstance;
}

void CGalbrenaRopeDrag::Free()
{
	CInteractionState::Free();
}
