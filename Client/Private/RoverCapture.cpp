#include "ClientPch.h"
#include "RoverCapture.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"

HRESULT CRoverCapture::Initialize(CGameObject* pOwner)
{
	if (FAILED(CCaptureState::Initialize(pOwner)))
		return E_FAIL;

	m_pRover = dynamic_cast<CRover*>(pOwner);
	ASSERT_CRASH(m_pRover);

	// 1. 애니메이션 설정
	Setup_Animations();
    return S_OK;
}

void CRoverCapture::OnEnter(void* pArg)
{
	CCaptureState::OnEnter(pArg);

	// 1. 복사본 context 받아오기.
	const auto context = m_pRover->TakeStateContext();

	// 2. 복사본에서 필요한 값 읽기
	ERoverCaptureType eCaptureType = context.m_eCaptureType;

	// 3. 값에 따른 상태 변경.
	m_iCurrentAnimIdx = static_cast<_uint>(context.m_eCaptureType);

	// 4. 현재 상태 초기화
	State_Reset();

	// 5. 중력 켰다.
	m_pRover->Set_Gravity(false);

	// 6. 타이머 속도 변경
	m_pRover->Change_TimeRate(TEXT("Timer_60"), 0.5f, 0.5f);

	m_eCaptureStep = CAPTURESTEP::STEP_START;
}

void CRoverCapture::OnUpdate(_float fTimeDelta)
{
	CCaptureState::OnUpdate(fTimeDelta);

	// 0. 키입력 감지.
	Handle_Input();

	// 1. 애니메이션 갱신.
	Update_CaptureAnimation(fTimeDelta); // 애니메이션 갱신 (및 이동/회전).

	// 2. 물리 체크.
	Check_Physics(fTimeDelta);

	// 3. 전환 제어
	Check_StateTransition(fTimeDelta);

	// 4. 현재  상태 초기화
	State_Reset();
}

void CRoverCapture::OnExit()
{
	CCaptureState::OnExit();
	m_pRover->Set_Gravity(true);
	m_pRover->Set_Visible(true);
	m_pRover->ClearCaptureState();
	m_eCaptureStep = CAPTURESTEP::STEP_END;
	//m_pRover->ResetPose();
}

void CRoverCapture::Handle_Input()
{
	// 애니메이션 실행 도중에 그랩 상태가 제거된다면?
	m_States[GRAB_EXIT] = m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::GRABRELEASE)) && CAPTURESTEP::STEP_END == m_eCaptureStep; // 탈출 조건.

}

void CRoverCapture::Update_CaptureAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pRover, fTimeDelta);


}

void CRoverCapture::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
}

void CRoverCapture::Check_StateTransition(_float fTimeDelta)
{
	// => 탈출 조건 일때만 탈출 가능하게?

	ERoverCaptureType eCaptureType = static_cast<ERoverCaptureType>(m_iCurrentAnimIdx);

	// 올라갈때는 Fly로
	if (m_IsAnimationEnd && m_eCaptureStep == CAPTURESTEP::STEP_START)
	{
		m_pRover->Set_Visible(false);
		m_iCurrentAnimIdx = ENUM_CLASS(ERoverCaptureType::CAPTURED);
		m_eCaptureStep = CAPTURESTEP::STEP_END;
		return;
	}

	// 탈출했다면?
	if (m_States[GRAB_EXIT])
	{
		m_pRover->GetStateContextForWrite().m_strPrevInfo = "Capture";
		m_pRover->GetStateContextForWrite().m_eHitType = ERoverHitType::BEHIT_FLY_FALL;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(ERoverHitState::HIT));
		return;
	}
	
		
	


	
}

void CRoverCapture::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ERoverCaptureType::CAPTURED), "Captured", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverCaptureType::BEHIT_PUSH_LOOP), "Behit_Push_Loop", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverCaptureType::BEHIT_FLY_START), "Behit_Fly_Start", 0.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverCaptureType::BEHIT_FLY_LOOP), "Behit_Fly_Loop", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverCaptureType::BEHIT_FLY_FALL), "Behit_Fly_Fall", 1.3f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverCaptureType::BEHIT_PUSH_FALL), "Behit_Push_Fall", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(ERoverCaptureType::BEHIT_PUSH_START), "Behit_Push_Start", 1.f, 0.f);
}

void CRoverCapture::State_Reset()
{
	for (_uint i = 0; i < CAPTURESTATE::END; ++i)
		m_States[i] = false;
}

CRoverCapture* CRoverCapture::Create(CGameObject* pOwner)
{
	CRoverCapture* pInstance = new CRoverCapture();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CRoverCapture");
	}

	return pInstance;
}

void CRoverCapture::Free()
{
	CCaptureState::Free();
}
