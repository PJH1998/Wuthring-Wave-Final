#include "ClientPch.h"
#include "GalbrenaControl.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"

// 이건 이동 Rope 로만 사용하자.
HRESULT CGalbrenaControl::Initialize(CCharacter* pCharacter)
{
	if (FAILED(CInteractionState::Initialize(pCharacter)))
		return E_FAIL;

	m_pGalbrena = dynamic_cast<CGalbrena*>(pCharacter);
	ASSERT_CRASH(m_pGalbrena);

	Setup_Animations();
	return S_OK;
}

void CGalbrenaControl::OnEnter(void* pArg)
{
	CInteractionState::OnEnter(pArg);

	// 1. 초기 단계 설정.
	m_eControlStep = CONTROLSTEP::STEP_ATTACH;

	// 3. 애니메이션 선정 =>
	m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaControlType::MANIPULATE_ABSORB_START);

	// 4. 장착 시키기.
	m_pGalbrena->Attach_ThrowTarget(true);

	// 5. 상태 리셋.
	State_Reset();

	m_pGalbrena->Set_Gravity(true);

}

void CGalbrenaControl::OnUpdate(_float fTimeDelta)
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

	//m_pGalbrena->Rotate_GrappleTarget(); // 회전.
}

void CGalbrenaControl::OnExit()
{
	CInteractionState::OnExit();
	m_pGalbrena->Set_Gravity(false);
	m_eControlStep = CONTROLSTEP::STEP_NONE;
}



void CGalbrenaControl::Enter_Rope()
{

}

void CGalbrenaControl::Handle_Input()
{
	m_eDir = m_pGalbrena->Calculate_Direction(); // 방향 계산.
	//m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);

	m_States[DETACH] = (m_eControlStep == CONTROLSTEP::STEP_ATTACH_LOOP
		&& m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::T))) ||
		(m_pGalbrena->Get_UtilityType() != UI_TAB_UTILITY::LEVITATOR);

	m_States[THROW] = (m_eControlStep == CONTROLSTEP::STEP_ATTACH_LOOP)
		&& m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

	m_States[RUN] = m_pGalbrena->Check_AnyInput(m_iMoveKey);

	
	// => 상호 작용 T 타입을 바꾸면?
}

void CGalbrenaControl::Update_ControlAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);

}

void CGalbrenaControl::Check_Physics(_float fTimeDelta)
{
}

void CGalbrenaControl::Check_StateTransition(_float fTimeDelta)
{
	EGalbrenaControlType eRopeType = static_cast<EGalbrenaControlType>(m_iCurrentAnimIdx);
	_bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. Step Start 인경우? => Start2로 변경.
	if (m_eControlStep == CONTROLSTEP::STEP_ATTACH)
	{
		if (m_IsAnimationEnd)
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaControlType::MANIPULATE_ABSORB_LOOP);
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
				m_pGalbrena->Attach_ThrowTarget(false);
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaControlType::MANIPULATE_ABSORB_END);
				m_eControlStep = CONTROLSTEP::STEP_DETACH;
				return;
			}

			if (m_States[THROW])
			{
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaControlType::MANIPULATE_RELEASE_F);
				m_eControlStep = CONTROLSTEP::STEP_THROW;
				return;
			}

		}

		if (m_IsAnimationEnd)
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaControlType::MANIPULATE_ABSORB_LOOP);
			return;
		}
	}

	if (m_eControlStep == CONTROLSTEP::STEP_DETACH)
	{
		if (IsEscapePossible)
		{
			if (m_States[RUN])
			{
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
				return;
			}
		}

		if (m_IsAnimationEnd)
		{
			m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND1_ACTION01;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
			return;
		}
	}

	if (m_eControlStep == CONTROLSTEP::STEP_THROW)
	{
		if (IsEscapePossible)
		{
			if (m_States[RUN])
			{
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
				return;
			}
		}

		if (m_IsAnimationEnd)
		{
			m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND1_ACTION01;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
			return;
		}
	}
}


void CGalbrenaControl::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EGalbrenaControlType::MANIPULATE_ABSORB_END), "Manipulate_Absorb_End", 1.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaControlType::MANIPULATE_ABSORB_LOOP), "Manipulate_Absorb_Loop", 1.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaControlType::MANIPULATE_ABSORB_START), "Manipulate_Absorb_Start", 1.5f, 0.f);

	CState::Add_Animations(ENUM_CLASS(EGalbrenaControlType::MANIPULATE_HOLD),		"Manipulate_Hold", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaControlType::MANIPULATE_RELEASE_F), "Manipulate_Release_F", 1.7f, 40.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaControlType::MANIPULATE_RELEASE_F_02), "Manipulate_Release_F_02", 1.7f, 40.f);
}

void CGalbrenaControl::State_Reset()
{
	for (_uint i = 0; i < CONTROLSTATE::END; ++i)
		m_States[i] = false;
}



CGalbrenaControl* CGalbrenaControl::Create(CCharacter* pOwner)
{
	CGalbrenaControl* pInstance = new CGalbrenaControl();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CGalbrenaControl");
		return nullptr;
	}

	return pInstance;
}

void CGalbrenaControl::Free()
{
	CInteractionState::Free();
}
