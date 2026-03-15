#include "ClientPch.h"
#include "AugustaRopeDrag.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaRopeDrag::Initialize(CCharacter* pCharacter)
{
	if (FAILED(CInteractionState::Initialize(pCharacter)))
		return E_FAIL;

	m_pAugusta = dynamic_cast<CAugusta*>(pCharacter);
	ASSERT_CRASH(m_pAugusta);

	Setup_Animations();
	return S_OK;
}

void CAugustaRopeDrag::OnEnter(void* pArg)
{
	CInteractionState::OnEnter(pArg);

	m_eRopeDir = m_pAugusta->Calculate_RopeDirection();
	m_eRopeStep = ROPESTEP::STEP_START;

	switch (m_eRopeDir)
	{
	case ROPEDIR::U:
		m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeDragType::DRAG_START_U);
		break;
	case ROPEDIR::F:
		m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeDragType::DRAG_START_F);
		break;
	case ROPEDIR::D:
		m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeDragType::DRAG_START_D);
		break;
	}

	State_Reset();
	m_pAugusta->Rotate_GrappleTarget();
	m_pAugusta->Set_Gravity(false);
	m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::ROPE_DRAG));
	m_pAugusta->Rope_Active(true);
	m_pAugusta->Spawn_RopeEffect(TEXT("Rope"), "WeaponProp02");
}

void CAugustaRopeDrag::OnUpdate(_float fTimeDelta)
{
	CInteractionState::OnUpdate(fTimeDelta);
	Handle_Input();
	Update_RopeAnimation(fTimeDelta);
	Check_Physics(fTimeDelta);
	Check_StateTransition(fTimeDelta);
	State_Reset();
}

void CAugustaRopeDrag::OnExit()
{
	CInteractionState::OnExit();
	m_pAugusta->Set_Gravity(false);
	m_eRopeDir = ROPEDIR::END;
	m_eRopeStep = ROPESTEP::STEP_NONE;
	m_pAugusta->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::ROPE_DRAG));
	m_pAugusta->Rope_Active(false);
}



void CAugustaRopeDrag::Enter_Rope()
{

}

void CAugustaRopeDrag::Handle_Input()
{
	m_eDir = m_pAugusta->Calculate_Direction(); // 방향 계산.
	m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
	m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
	m_States[DRAG] = (m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::T))) 
		&& (m_eRopeStep == ROPESTEP::STEP_LOOP);// Loop 상태일때만 당길 수있다.

}

void CAugustaRopeDrag::Update_RopeAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
}

void CAugustaRopeDrag::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);
	m_States[REACHED] = m_pAugusta->Is_ReachedGrappleHook();
}

void CAugustaRopeDrag::Check_StateTransition(_float fTimeDelta)
{
	EAugustaRopeDragType eRopeType = static_cast<EAugustaRopeDragType>(m_iCurrentAnimIdx);
	_bool IsEscapePossible = CState::Is_EscapePossible();


	// 1. Step Start 인경우? => Start2로 변경.
	if (m_eRopeStep == ROPESTEP::STEP_START)
	{
		if (IsEscapePossible)
		{

			switch (eRopeType)
			{
			case EAugustaRopeDragType::DRAG_START_U:
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeDragType::DRAG_LOOP_U);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			case EAugustaRopeDragType::DRAG_START_D:
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeDragType::DRAG_LOOP_D);
				m_eRopeStep = ROPESTEP::STEP_LOOP;
				break;
			case EAugustaRopeDragType::DRAG_START_F:
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeDragType::DRAG_LOOP_F);
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
				m_pAugusta->Execute_RopeDragTrigger(); // 연결된 객체의 Trigger 호출.
				m_eRopeStep = ROPESTEP::STEP_END;
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRopeDragType::DRAG_END);
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
			}

			if (!m_States[LAND])
			{
				if (m_States[JUMP])
				{
					m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_WALK_LF;
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
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

				
			}

			if (!m_States[LAND]) // 땅이 아니라면?
			{
				if (m_States[JUMP])
				{
					m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F;
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
					return;
				}
			}
		}

		if (m_IsAnimationEnd) // 애니메이션이 종료되면?
		{
			if (m_States[LAND])
			{
				m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STANDCHANGE;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
				return;
			}

			if (!m_States[LAND])
			{
				m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL));
				return;
			}
		}

		
	}

	
}


void CAugustaRopeDrag::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeDragType::DRAG_START_U), "Drag_Start_U", 1.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeDragType::DRAG_START_F), "Drag_Start_F", 1.5f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeDragType::DRAG_START_D), "Drag_Start_D", 1.5f, 20.f);

	CState::Add_Animations(ENUM_CLASS(EAugustaRopeDragType::DRAG_LOOP_U), "Drag_Loop_U", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeDragType::DRAG_LOOP_F), "Drag_Loop_F", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaRopeDragType::DRAG_LOOP_D), "Drag_Loop_D", 1.f, 0.f);

	CState::Add_Animations(ENUM_CLASS(EAugustaRopeDragType::DRAG_END), "Drag_End", 1.5f, 100.f);
	

}

void CAugustaRopeDrag::State_Reset()
{
	for (_uint i = 0; i < ROPESTATE::END; ++i)
		m_States[i] = false;
}



CAugustaRopeDrag* CAugustaRopeDrag::Create(CCharacter* pOwner)
{
	CAugustaRopeDrag* pInstance = new CAugustaRopeDrag();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CAugustaRopeDrag");
		return nullptr;
	}

	return pInstance;
}

void CAugustaRopeDrag::Free()
{
	CInteractionState::Free();
}
