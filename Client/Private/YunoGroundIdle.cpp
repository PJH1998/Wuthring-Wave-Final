#include "ClientPch.h"
#include "YunoGroundIdle.h"
#include "Yuno.h"
#include "StateMachine.h"

HRESULT CYunoGroundIdle::Initialize(CCharacter* pCharacter)
{
	if (FAILED(CGroundState::Initialize(pCharacter)))
		return E_FAIL;

	m_pYuno = dynamic_cast<CYuno*>(pCharacter);
	ASSERT_CRASH(m_pYuno);

	// Idle 애니메이션 리스트 셋업
	Setup_Animations();

	// 기본 애니메이션 셋업.
	m_iCurrentAnimIdx = 0;

	// 바꿀 파트타입?

	return S_OK;
}

void CYunoGroundIdle::OnEnter(void* pArg)
{
	CGroundState::OnEnter(pArg);

	// 1. 복사본 Context 받아오기
	const auto context = m_pYuno->TakeStateContext();

	// 2. 복사본에서 필요한 값 읽기
	EYunoIdleType eIdleType = context.m_eIdleType;

	m_iCurrentAnimIdx = ENUM_CLASS(eIdleType);

	// 3. Idle 상태 초기화
	State_Reset();

	m_pYuno->Set_Gravity(true);
}

void CYunoGroundIdle::OnUpdate(_float fTimeDelta)
{
	CGroundState::OnUpdate(fTimeDelta);

	// 0. 입력 확인
	Handle_Input();

	// 1. Idle 업데이트
	Update_IdleAnimations(fTimeDelta);

	// 2. 물리 체크
	Check_Physics(fTimeDelta);

	// 3. 상태 전환.
	Check_StateTransition(fTimeDelta);

	// 4. 상태 초기화
	State_Reset();
}

void CYunoGroundIdle::OnExit()
{
	CGroundState::OnExit();
	m_pYuno->Set_Gravity(true);
}

void CYunoGroundIdle::Handle_Input()
{
}

void CYunoGroundIdle::Update_IdleAnimations(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pYuno, fTimeDelta);
}

void CYunoGroundIdle::Check_Physics(_float fTimeDelta)
{
}

void CYunoGroundIdle::Check_StateTransition(_float fTimeDelta)
{
}

void CYunoGroundIdle::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EYunoIdleType::STAND1_ACTION01), "Stand1_Action01", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EYunoIdleType::STAND1_ACTION02), "Stand1_Action02", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EYunoIdleType::STAND1_ACTION03), "Stand1_Action03", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EYunoIdleType::STAND2), "Stand2", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EYunoIdleType::STANDCHANGE), "StandChange", 1.f, 0.f);
}

void CYunoGroundIdle::State_Reset()
{
	for (_uint i = 0; i < IDLESTATE::END; ++i)
		m_States[i] = false;
}

CYunoGroundIdle* CYunoGroundIdle::Create(CCharacter* pOwner)
{
	CYunoGroundIdle* pInstance = new CYunoGroundIdle();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CYunoGroundIdle");
		return nullptr;
	}

	return pInstance;
}

void CYunoGroundIdle::Free()
{
	CGroundState::Free();
}
