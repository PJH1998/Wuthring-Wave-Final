#include "ClientPch.h"
#include "AugustaCapture.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaCapture::Initialize(CCharacter* pCharacter)
{
	if (FAILED(CCaptureState::Initialize(pCharacter)))
		return E_FAIL;

	m_pAugusta = dynamic_cast<CAugusta*>(pCharacter);
	ASSERT_CRASH(m_pAugusta);

	// 1. 애니메이션 설정
	Setup_Animations();
    return S_OK;
}

void CAugustaCapture::OnEnter(void* pArg)
{
	CCaptureState::OnEnter(pArg);

	// 1. 복사본 context 받아오기.
	const auto context = m_pAugusta->TakeStateContext();

	// 2. 복사본에서 필요한 값 읽기
	EAugustaCaptureType eCaptureType = context.m_eCaptureType;

	// 3. 값에 따른 상태 변경.
	m_iCurrentAnimIdx = static_cast<_uint>(context.m_eCaptureType);

	// 4. 현재 상태 초기화
	State_Reset();

	// 5. 중력 켰다.
	m_pAugusta->Set_Gravity(false);

	// 6. 타이머 속도 변경
	m_pAugusta->Change_TimeRate(TEXT("Timer_60"), 0.5f, 0.5f);

	// 7. Capture 스테이트 변경.
	m_eCaptureStep = CAPTURESTEP::STEP_START;
}

void CAugustaCapture::OnUpdate(_float fTimeDelta)
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

void CAugustaCapture::OnExit()
{
	CCaptureState::OnExit();
	m_pAugusta->Set_Gravity(true);
	m_pAugusta->Set_Visible(true);
	m_pAugusta->ClearCaptureState();
	m_eCaptureStep = CAPTURESTEP::STEP_NONE;
	//m_pAugusta->ResetPose();
}

void CAugustaCapture::Handle_Input()
{
	// 애니메이션 실행 도중에 그랩 상태가 제거된다면?
	m_States[GRAB_EXIT] = m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::GRABRELEASE)) && CAPTURESTEP::STEP_END == m_eCaptureStep; // 탈출 조건.

}

void CAugustaCapture::Update_CaptureAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);


}

void CAugustaCapture::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);
}

void CAugustaCapture::Check_StateTransition(_float fTimeDelta)
{
	// => 탈출 조건 일때만 탈출 가능하게?

	EAugustaCaptureType eCaptureType = static_cast<EAugustaCaptureType>(m_iCurrentAnimIdx);

	// 올라갈때는 Fly로
	if (m_IsAnimationEnd && m_eCaptureStep == CAPTURESTEP::STEP_START)
	{
		//m_iCurrentAnimIdx = ENUM_CLASS(EAugustaCaptureType::BEHIT_FLY_LOOP);
		m_pAugusta->Set_Visible(false);
		m_iCurrentAnimIdx = ENUM_CLASS(EAugustaCaptureType::CAPTURED);
		m_eCaptureStep = CAPTURESTEP::STEP_END;
		return;
	}
//	if (m_IsAnimationEnd && eCaptureType == EAugustaCaptureType::BEHIT_FLY_LOOP)
//	{
//		m_iCurrentAnimIdx = ENUM_CLASS(EAugustaCaptureType::CAPTURED);
//		return;
//	}

	// 탈출했다면?
	if (m_States[GRAB_EXIT])
	{
		m_pAugusta->GetStateContextForWrite().m_strPrevInfo = "Capture";
		m_pAugusta->GetStateContextForWrite().m_eHitType = EAugustaHitType::BEHIT_FLY_FALL;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EAugustaHitState::HIT));
		return;
	}
	
		
	


	
}

void CAugustaCapture::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EAugustaCaptureType::CAPTURED), "Captured", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaCaptureType::BEHIT_PUSH_LOOP), "Behit_Push_Loop", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaCaptureType::BEHIT_FLY_START), "Behit_Fly_Start", 0.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaCaptureType::BEHIT_FLY_LOOP), "Behit_Fly_Loop", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaCaptureType::BEHIT_FLY_FALL), "Behit_Fly_Fall", 1.3f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaCaptureType::BEHIT_PUSH_FALL), "Behit_Push_Fall", 1.f, 0.f);
}

void CAugustaCapture::State_Reset()
{
	for (_uint i = 0; i < CAPTURESTATE::END; ++i)
		m_States[i] = false;
}

CAugustaCapture* CAugustaCapture::Create(CCharacter* pOwner)
{
	CAugustaCapture* pInstance = new CAugustaCapture();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CAugustaCapture");
	}

	return pInstance;
}

void CAugustaCapture::Free()
{
	CCaptureState::Free();
}
