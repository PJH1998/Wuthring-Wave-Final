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

	// 1. 애니메이션 결정을 위한 방향 설정.
	m_eRopeDir = m_pGalbrena->Calculate_RopeDirection();

	// 2. 초기 단계 설정.
	m_eControlStep = CONTROLSTEP::STEP_START;

	// 3. 상태 리셋.
	State_Reset();

	// 4. 나중에 감지된 위치에 있는 방향으로 회전합니다. 
	m_pGalbrena->Rotate_GrappleTarget();

	// 5. 중력 끄기
	m_pGalbrena->Set_Gravity(false);

	// 6. 현재 상태 부여.

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
	m_eRopeDir = ROPEDIR::END;
	m_eControlStep = CONTROLSTEP::STEP_NONE;
}



void CGalbrenaControl::Enter_Rope()
{

}

void CGalbrenaControl::Handle_Input()
{
	m_eDir = m_pGalbrena->Calculate_Direction(); // 방향 계산.
	m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);

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
}


void CGalbrenaControl::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EGalbrenaControlType::MANIPULATE_ABSORB_END), "Manipulate_Absorb_End", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaControlType::MANIPULATE_ABSORB_LOOP), "Manipulate_Absorb_Loop", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaControlType::MANIPULATE_ABSORB_START), "Manipulate_Absorb_Start", 1.f, 0.f);

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
