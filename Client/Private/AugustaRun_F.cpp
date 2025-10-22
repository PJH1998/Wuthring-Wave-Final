#include "ClientPch.h"
#include "AugustaRun_F.h"
HRESULT CAugustaRun_F::Initialize(const STATE_DATA& StateData)
{
	// 1. StateData 초기화
	CAugustaBaseState::Initialize(StateData);

	// 2. Transition 등록
	Ready_Transitions();
	
	return S_OK;
}

void CAugustaRun_F::OnEnter()
{
	CState::OnEnter();
}

// 갱신. <- 외부에서 의존성을 주입받는 시기.
void CAugustaRun_F::OnUpdate(_float fTimeDelta)
{
	// 1. 애니메이션 실행.
	m_IsAnimationEnd = m_pPlayer->Play_Animation(m_StateData.strAnimName
		, fTimeDelta, &m_fTrackPosition, m_StateData.fRootMotionRate, m_StateData.IsRootMotion);

	// 2. 수행할 작업.
	if (m_pPlayer->Check_AnyInput(ENUM_CLASS(KEYINPUT::W)))
	{
		if (m_pPlayer->Check_AnyInput(ENUM_CLASS(KEYINPUT::A)))
		{

		}
		if (m_pPlayer->Check_AnyInput(ENUM_CLASS(KEYINPUT::D)))
		{

		}
		return;
	}


	if (m_pPlayer->Check_AnyInput(ENUM_CLASS(KEYINPUT::S)))
	{
		if (m_pPlayer->Check_AnyInput(ENUM_CLASS(KEYINPUT::A)))
		{

		}
		if (m_pPlayer->Check_AnyInput(ENUM_CLASS(KEYINPUT::D)))
		{

		}
		return;
	}

	if (m_pPlayer->Check_AnyInput(ENUM_CLASS(KEYINPUT::A)))
	{
		return;
	}

	if (m_pPlayer->Check_AnyInput(ENUM_CLASS(KEYINPUT::D)))
	{
		return;
	}

	//m_pPlayer->Check_AnyInput();

}

// 탈출시 실행.
void CAugustaRun_F::OnExit()
{

}

void CAugustaRun_F::Ready_Transitions()
{
	// Stop 상태로 변경.
	Add_Transition("Stop_Run_L", [this]()-> bool {
		if (!m_pPlayer->Check_AnyInput(m_iMoveKey))
			return true;
		return false;
	});

	//// 키입력이 없으면 바로 변경.
	//Add_Transition("Stand1_Action01", [this]()-> bool {
	//	if (m_IsAnimationEnd)
	//		return true;
	//	return false;
	//	});
}

CAugustaRun_F* CAugustaRun_F::Create(const STATE_DATA& stateData)
{
	CAugustaRun_F* pInstance = new CAugustaRun_F();
	if (FAILED(pInstance->Initialize(stateData)))
	{
		MSG_BOX("Create Failed Augusta Stand");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAugustaRun_F::Free()
{
	CState::Free();
}
