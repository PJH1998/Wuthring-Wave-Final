#include "ClientPch.h"
#include "RoverRopeDrag.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"

// 이건 이동 Rope 로만 사용하자.
HRESULT CRoverRopeDrag::Initialize(CCharacter* pCharacter)
{
	if (FAILED(CInteractionState::Initialize(pCharacter)))
		return E_FAIL;

	m_pRover = dynamic_cast<CRover*>(pCharacter);
	ASSERT_CRASH(m_pRover);

	Setup_Animations();
	return S_OK;
}

void CRoverRopeDrag::OnEnter(void* pArg)
{
	CInteractionState::OnEnter(pArg);

	// 1. 애니메이션 결정을 위한 방향 설정.
	m_eRopeDir = m_pRover->Calculate_RopeDirection();

	// 2. 초기 단계 설정.
	m_eRopeStep = ROPESTEP::STEP_START;

	switch (m_eRopeDir)
	{
	case ROPEDIR::U:
		m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeDragType::DRAG_START_U);
		break;
	case ROPEDIR::F:
		m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeDragType::DRAG_START_F);
		break;
	case ROPEDIR::D:
		m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeDragType::DRAG_START_D);
		break;
	}

	// 4. 상태 리셋.
	State_Reset();

	// 5. 나중에 감지된 위치에 있는 방향으로 회전합니다. 
	m_pRover->Rotate_GrappleTarget();

	// 6. 중력 끄기
	m_pRover->Set_Gravity(false);

	// 7. 현재 상태 부여. =>
	m_pRover->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::ROPE_DRAG));
}

void CRoverRopeDrag::OnUpdate(_float fTimeDelta)
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

void CRoverRopeDrag::OnExit()
{
	CInteractionState::OnExit();
	m_pRover->Set_Gravity(false);
	m_eRopeDir = ROPEDIR::END;
	m_eRopeStep = ROPESTEP::STEP_NONE;
	
	m_pRover->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::ROPE_DRAG));
}



void CRoverRopeDrag::Enter_Rope()
{

}

void CRoverRopeDrag::Handle_Input()
{
	m_eDir = m_pRover->Calculate_Direction(); // 방향 계산.
	m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
	m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
	m_States[DRAG] = (m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::T))) 
		&& (m_eRopeStep == ROPESTEP::STEP_LOOP);// Loop 상태일때만 당길 수있다.

}

void CRoverRopeDrag::Update_RopeAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pRover, fTimeDelta);
}

void CRoverRopeDrag::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
	m_States[REACHED] = m_pRover->Is_ReachedGrappleHook();
}

void CRoverRopeDrag::Check_StateTransition(_float fTimeDelta)
{
	ERoverRopeDragType eRopeType = static_cast<ERoverRopeDragType>(m_iCurrentAnimIdx);
	_bool IsEscapePossible = CState::Is_EscapePossible();


	// 1. Step Start 인경우? => Start2로 변경.
	if (m_eRopeStep == ROPESTEP::STEP_START)
	{
		if (IsEscapePossible)
		{

			switch (eRopeType)
			{
			case ERoverRopeDragType::DRAG_START_U:
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeDragType::DRAG_LOOP_U);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			case ERoverRopeDragType::DRAG_START_D:
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeDragType::DRAG_LOOP_D);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			case ERoverRopeDragType::DRAG_START_F:
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeDragType::DRAG_LOOP_F);
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
				m_pRover->Execute_RopeDragTrigger(); // 연결된 객체의 Trigger 호출.
				m_eRopeStep = ROPESTEP::STEP_END;
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverRopeDragType::DRAG_END);
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
			}

			if (!m_States[LAND])
			{
				if (m_States[JUMP])
				{
					m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_WALK_LF;
					m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP));
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
			}

			if (!m_States[LAND]) // 땅이 아니라면?
			{
				if (m_States[JUMP])
				{
					m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_SECOND_F;
					m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP));
					return;
				}
			}
		}

		if (m_IsAnimationEnd) // 애니메이션이 종료되면?
		{
			if (m_States[LAND])
			{
				m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STANDCHANGE;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
				return;
			}

			if (!m_States[LAND])
			{
				m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL));
				return;
			}
		}

		
	}

	
}


void CRoverRopeDrag::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ERoverRopeDragType::DRAG_START_U), "Drag_Start_U", 1.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeDragType::DRAG_START_F), "Drag_Start_F", 1.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeDragType::DRAG_START_D), "Drag_Start_D", 1.5f, 20.f);

	CState::Add_Animations(ENUM_CLASS(ERoverRopeDragType::DRAG_LOOP_U), "Drag_Loop_U", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeDragType::DRAG_LOOP_F), "Drag_Loop_F", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverRopeDragType::DRAG_LOOP_D), "Drag_Loop_D", 1.f, 0.f);

	CState::Add_Animations(ENUM_CLASS(ERoverRopeDragType::DRAG_END), "Drag_End", 1.5f, 100.f);
	

}

void CRoverRopeDrag::State_Reset()
{
	for (_uint i = 0; i < ROPESTATE::END; ++i)
		m_States[i] = false;
}



CRoverRopeDrag* CRoverRopeDrag::Create(CCharacter* pOwner)
{
	CRoverRopeDrag* pInstance = new CRoverRopeDrag();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CRoverRopeDrag");
		return nullptr;
	}

	return pInstance;
}

void CRoverRopeDrag::Free()
{
	CInteractionState::Free();
}
