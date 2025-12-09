#include "ClientPch.h"
#include "AugustaControl.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

// 이건 이동 Rope 로만 사용하자.
HRESULT CAugustaControl::Initialize(CCharacter* pCharacter)
{
	if (FAILED(CInteractionState::Initialize(pCharacter)))
		return E_FAIL;

	m_pAugusta = dynamic_cast<CAugusta*>(pCharacter);
	ASSERT_CRASH(m_pAugusta);

	Setup_Animations();
	return S_OK;
}

void CAugustaControl::OnEnter(void* pArg)
{
	CInteractionState::OnEnter(pArg);

	// 1. 애니메이션 결정을 위한 방향 설정.
	// 2. 초기 단계 설정.
	m_eControlStep = CONTROLSTEP::STEP_ATTACH;

	// 3. 애니메이션 선정 =>
	m_iCurrentAnimIdx = ENUM_CLASS(EAugustaControlType::MANIPULATE_ABSORB_START);

	// 4. 장착 시키기.
	m_pAugusta->Attach_ThrowTarget(true);

	// 5. 상태 리셋.
	State_Reset();

	m_pAugusta->Set_Gravity(true);

}

void CAugustaControl::OnUpdate(_float fTimeDelta)
{
	CInteractionState::OnUpdate(fTimeDelta);

	// 0. 키입력 체크
	Handle_Input();

	// 1. 애니메이션 갱신
	Update_ControlAnimation(fTimeDelta);

	// 2. 물리 체크
	Check_Physics(fTimeDelta);

	// 3. 전환 체크
	Check_StateTransition(fTimeDelta);

	// 상태 리셋;
	State_Reset();

	//m_pAugusta->Rotate_GrappleTarget(); // 회전.
}

void CAugustaControl::OnExit()
{
	CInteractionState::OnExit();
	m_pAugusta->Set_Gravity(false);
	m_eControlStep = CONTROLSTEP::STEP_NONE;
}



void CAugustaControl::Enter_Rope()
{

}

void CAugustaControl::Handle_Input()
{
	m_eDir = m_pAugusta->Calculate_Direction(); // 방향 계산.

	m_States[DETACH] = (m_eControlStep == CONTROLSTEP::STEP_ATTACH_LOOP
		&& m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::T))) ||
		(m_pAugusta->Get_UtilityType() != UI_TAB_UTILITY::LEVITATOR);

	m_States[THROW] = (m_eControlStep == CONTROLSTEP::STEP_ATTACH_LOOP)
		&& m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

	m_States[RUN] = m_pAugusta->Check_AnyInput(m_iMoveKey);

	
	// => 상호 작용 T 타입을 바꾸면?
}

void CAugustaControl::Update_ControlAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

}

void CAugustaControl::Check_Physics(_float fTimeDelta)
{
}

void CAugustaControl::Check_StateTransition(_float fTimeDelta)
{
	EAugustaControlType eRopeType = static_cast<EAugustaControlType>(m_iCurrentAnimIdx);
	_bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. Step Start 인경우? => Start2로 변경.
	if (m_eControlStep == CONTROLSTEP::STEP_ATTACH)
	{
		if (m_IsAnimationEnd)
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaControlType::MANIPULATE_ABSORB_LOOP);
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
				m_pAugusta->Attach_ThrowTarget(false);
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaControlType::MANIPULATE_ABSORB_END);
				m_eControlStep = CONTROLSTEP::STEP_DETACH;
				return;
			}

			if (m_States[THROW])
			{
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaControlType::MANIPULATE_RELEASE_F);
				m_eControlStep = CONTROLSTEP::STEP_THROW;
				return;
			}
		}

		if (m_IsAnimationEnd)
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaControlType::MANIPULATE_ABSORB_LOOP);
			return;
		}
	}

	if (m_eControlStep == CONTROLSTEP::STEP_DETACH)
	{
		if (IsEscapePossible)
		{
			if (m_States[RUN])
			{
				m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
				return;
			}
		}

		if (m_IsAnimationEnd)
		{
			m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
			return;
		}
	}

	if (m_eControlStep == CONTROLSTEP::STEP_THROW)
	{
		if (IsEscapePossible)
		{
			if (m_States[RUN])
			{
				m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
				return;
			}
		}

		if (m_IsAnimationEnd)
		{
			m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
			return;
		}
	}
}


void CAugustaControl::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EAugustaControlType::MANIPULATE_ABSORB_END), "Manipulate_Absorb_End", 1.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaControlType::MANIPULATE_ABSORB_LOOP), "Manipulate_Absorb_Loop", 1.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaControlType::MANIPULATE_ABSORB_START), "Manipulate_Absorb_Start", 1.5f, 0.f);

	CState::Add_Animations(ENUM_CLASS(EAugustaControlType::MANIPULATE_HOLD),		"Manipulate_Hold", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaControlType::MANIPULATE_RELEASE_F), "Manipulate_Release_F", 1.7f, 40.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaControlType::MANIPULATE_RELEASE_F_02), "Manipulate_Release_F_02", 1.7f, 40.f);
}

void CAugustaControl::State_Reset()
{
	for (_uint i = 0; i < CONTROLSTATE::END; ++i)
		m_States[i] = false;
}



CAugustaControl* CAugustaControl::Create(CCharacter* pOwner)
{
	CAugustaControl* pInstance = new CAugustaControl();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CAugustaControl");
		return nullptr;
	}

	return pInstance;
}

void CAugustaControl::Free()
{
	CInteractionState::Free();
}
