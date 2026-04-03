#include "ClientPch.h"
#include "RoverControl.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"

// 이건 이동 Rope 로만 사용하자.
HRESULT CRoverControl::Initialize(CCharacter* pCharacter)
{
	if (FAILED(CInteractionState::Initialize(pCharacter)))
		return E_FAIL;

	m_pRover = dynamic_cast<CRover*>(pCharacter);
	ASSERT_CRASH(m_pRover);

	Setup_Animations();
	return S_OK;
}

void CRoverControl::OnEnter(void* pArg)
{
	CInteractionState::OnEnter(pArg);

	// 1. 애니메이션 결정을 위한 방향 설정.
	// 2. 초기 단계 설정.
	m_eControlStep = CONTROLSTEP::STEP_ATTACH;

	// 3. 애니메이션 선정 =>
	m_iCurrentAnimIdx = ENUM_CLASS(ERoverControlType::MANIPULATE_ABSORB_START);

	// 4. 장착 시키기.
	m_pRover->Attach_ThrowTarget(true);

	// 5. 상태 리셋.
	State_Reset();

	m_pRover->Set_Gravity(true);

}

void CRoverControl::OnUpdate(_float fTimeDelta)
{
	CInteractionState::OnUpdate(fTimeDelta);

	Handle_Input();
	Update_ControlAnimation(fTimeDelta);
	Check_Physics(fTimeDelta);
	Check_StateTransition(fTimeDelta);
	State_Reset();
}

void CRoverControl::OnExit()
{
	CInteractionState::OnExit();
	m_pRover->Set_Gravity(false);
	m_eControlStep = CONTROLSTEP::STEP_NONE;
}



void CRoverControl::Enter_Rope()
{

}

void CRoverControl::Handle_Input()
{
	m_eDir = m_pRover->Calculate_Direction(); // 방향 계산.

	m_States[DETACH] = (m_eControlStep == CONTROLSTEP::STEP_ATTACH_LOOP
		&& m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::T))) ||
		(m_pRover->Get_UtilityType() != UI_TAB_UTILITY::LEVITATOR);

	m_States[THROW] = (m_eControlStep == CONTROLSTEP::STEP_ATTACH_LOOP)
		&& m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

	m_States[RUN] = m_pRover->Check_AnyInput(m_iMoveKey);

	
	// => 상호 작용 T 타입을 바꾸면?
}

void CRoverControl::Update_ControlAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pRover, fTimeDelta);

}

void CRoverControl::Check_Physics(_float fTimeDelta)
{
}

void CRoverControl::Check_StateTransition(_float fTimeDelta)
{
	ERoverControlType eRopeType = static_cast<ERoverControlType>(m_iCurrentAnimIdx);
	_bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. Step Start 인경우? => Start2로 변경.
	if (m_eControlStep == CONTROLSTEP::STEP_ATTACH)
	{
		if (m_IsAnimationEnd)
		{
			m_iCurrentAnimIdx = ENUM_CLASS(ERoverControlType::MANIPULATE_ABSORB_LOOP);
			m_eControlStep = CONTROLSTEP::STEP_ATTACH_LOOP;
			return;
		}
	}

	if (m_eControlStep == CONTROLSTEP::STEP_ATTACH_LOOP)
	{
		if (IsEscapePossible) // 탈출 가능 지점에서 분기 나누기.
		{
			if (m_States[DETACH]) // 해제.
			{
				m_pRover->Attach_ThrowTarget(false);
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverControlType::MANIPULATE_ABSORB_END);
				m_eControlStep = CONTROLSTEP::STEP_DETACH;
				return;
			}

			if (m_States[THROW])
			{
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverControlType::MANIPULATE_RELEASE_F);
				m_eControlStep = CONTROLSTEP::STEP_THROW;
				return;
			}
		}

		if (m_IsAnimationEnd)
		{
			m_iCurrentAnimIdx = ENUM_CLASS(ERoverControlType::MANIPULATE_ABSORB_LOOP);
			return;
		}
	}

	if (m_eControlStep == CONTROLSTEP::STEP_DETACH)
	{
		if (IsEscapePossible)
		{
			if (m_States[RUN])
			{
				m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
				return;
			}
		}

		if (m_IsAnimationEnd)
		{
			m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1_ACTION01;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
			return;
		}
	}

	if (m_eControlStep == CONTROLSTEP::STEP_THROW)
	{
		if (IsEscapePossible)
		{
			if (m_States[RUN])
			{
				m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
				return;
			}
		}

		if (m_IsAnimationEnd)
		{
			m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1_ACTION01;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
			return;
		}
	}
}


void CRoverControl::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ERoverControlType::MANIPULATE_ABSORB_END), "Manipulate_Absorb_End", 1.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverControlType::MANIPULATE_ABSORB_LOOP), "Manipulate_Absorb_Loop", 1.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverControlType::MANIPULATE_ABSORB_START), "Manipulate_Absorb_Start", 1.5f, 0.f);

	CState::Add_Animations(ENUM_CLASS(ERoverControlType::MANIPULATE_HOLD),		"Manipulate_Hold", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverControlType::MANIPULATE_RELEASE_F), "Manipulate_Release_F", 1.7f, 40.f);
	CState::Add_Animations(ENUM_CLASS(ERoverControlType::MANIPULATE_RELEASE_F_02), "Manipulate_Release_F_02", 1.7f, 40.f);
}

void CRoverControl::State_Reset()
{
	for (_uint i = 0; i < CONTROLSTATE::END; ++i)
		m_States[i] = false;
}



CRoverControl* CRoverControl::Create(CCharacter* pOwner)
{
	CRoverControl* pInstance = new CRoverControl();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CRoverControl");
		return nullptr;
	}

	return pInstance;
}

void CRoverControl::Free()
{
	CInteractionState::Free();
}
