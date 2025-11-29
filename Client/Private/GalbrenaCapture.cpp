#include "ClientPch.h"
#include "GalbrenaCapture.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"

HRESULT CGalbrenaCapture::Initialize(CGameObject* pOwner)
{
	if (FAILED(CCaptureState::Initialize(pOwner)))
		return E_FAIL;

	m_pGalbrena = dynamic_cast<CGalbrena*>(pOwner);
	ASSERT_CRASH(m_pGalbrena);

	// 1. 애니메이션 설정
	Setup_Animations();
    return S_OK;
}

void CGalbrenaCapture::OnEnter(void* pArg)
{
	CCaptureState::OnEnter(pArg);

	// 1. 복사본 context 받아오기.
	const auto context = m_pGalbrena->TakeStateContext();

	// 2. 복사본에서 필요한 값 읽기
	EGalbrenaCaptureType eCaptureType = context.m_eCaptureType;

	// 3. 값에 따른 상태 변경.
	m_iCurrentAnimIdx = static_cast<_uint>(context.m_eCaptureType);

	// 4. 현재 상태 초기화
	State_Reset();

	// 5. 중력 켰다.
	m_pGalbrena->Set_Gravity(false);

	// 6. 타이머 속도 변경
	m_pGalbrena->Change_TimeRate(TEXT("Timer_60"), 0.5f, 0.5f);

	m_eCaptureStep = CAPTURESTEP::STEP_START;
}

void CGalbrenaCapture::OnUpdate(_float fTimeDelta)
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

void CGalbrenaCapture::OnExit()
{
	CCaptureState::OnExit();
	m_pGalbrena->Set_Gravity(true);
	m_pGalbrena->Set_Visible(true);
	m_pGalbrena->ClearCaptureState();
	m_eCaptureStep = CAPTURESTEP::STEP_END;
	//m_pGalbrena->ResetPose();
}

void CGalbrenaCapture::Handle_Input()
{
	// 애니메이션 실행 도중에 그랩 상태가 제거된다면?
	m_States[GRAB_EXIT] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::GRABRELEASE)) && CAPTURESTEP::STEP_END == m_eCaptureStep; // 탈출 조건.

}

void CGalbrenaCapture::Update_CaptureAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);


}

void CGalbrenaCapture::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
}

void CGalbrenaCapture::Check_StateTransition(_float fTimeDelta)
{
	// => 탈출 조건 일때만 탈출 가능하게?

	EGalbrenaCaptureType eCaptureType = static_cast<EGalbrenaCaptureType>(m_iCurrentAnimIdx);

	// 올라갈때는 Fly로
	if (m_IsAnimationEnd && m_eCaptureStep == CAPTURESTEP::STEP_START)
	{
		m_pGalbrena->Set_Visible(false);
		m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaCaptureType::CAPTURED);
		m_eCaptureStep = CAPTURESTEP::STEP_END;
		return;
	}

	// 탈출했다면?
	if (m_States[GRAB_EXIT])
	{
		m_pGalbrena->GetStateContextForWrite().m_strPrevInfo = "Capture";
		m_pGalbrena->GetStateContextForWrite().m_eHitType = EGalbrenaHitType::BEHIT_FLY_FALL;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EGalbrenaHitState::HIT));
		return;
	}
	
		
	


	
}

void CGalbrenaCapture::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EGalbrenaCaptureType::CAPTURED), "Captured", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaCaptureType::BEHIT_PUSH_LOOP), "Behit_Push_Loop", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaCaptureType::BEHIT_FLY_START), "Behit_Fly_Start", 0.5f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaCaptureType::BEHIT_FLY_LOOP), "Behit_Fly_Loop", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaCaptureType::BEHIT_FLY_FALL), "Behit_Fly_Fall", 1.3f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaCaptureType::BEHIT_PUSH_FALL), "Behit_Push_Fall", 1.f, 0.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaCaptureType::BEHIT_PUSH_START), "Behit_Push_Start", 1.f, 0.f);
}

void CGalbrenaCapture::State_Reset()
{
	for (_uint i = 0; i < CAPTURESTATE::END; ++i)
		m_States[i] = false;
}

CGalbrenaCapture* CGalbrenaCapture::Create(CGameObject* pOwner)
{
	CGalbrenaCapture* pInstance = new CGalbrenaCapture();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CGalbrenaCapture");
	}

	return pInstance;
}

void CGalbrenaCapture::Free()
{
	CCaptureState::Free();
}
