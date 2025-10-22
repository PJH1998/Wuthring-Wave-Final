#include "ClientPch.h"
#include "AugustaStop_Run_L.h"
HRESULT CAugustaStop_Run_L::Initialize(const STATE_DATA& StateData)
{
	// 1. StateData 초기화
	CAugustaBaseState::Initialize(StateData);

	// 2. Transition 등록
	Ready_Transitions();
	
	return S_OK;
}

void CAugustaStop_Run_L::OnEnter()
{
	CState::OnEnter();
}

// 갱신. <- 외부에서 의존성을 주입받는 시기.
void CAugustaStop_Run_L::OnUpdate(_float fTimeDelta)
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
void CAugustaStop_Run_L::OnExit()
{

}

void CAugustaStop_Run_L::Ready_Transitions()
{
	// 기본 Stand 상태로 변경.
	Add_Transition("Stand1_Action01", [this]()-> bool {
		if (m_IsAnimationEnd)
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

CAugustaStop_Run_L* CAugustaStop_Run_L::Create(const STATE_DATA& stateData)
{
	CAugustaStop_Run_L* pInstance = new CAugustaStop_Run_L();
	if (FAILED(pInstance->Initialize(stateData)))
	{
		MSG_BOX("Create Failed Augusta Stand");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAugustaStop_Run_L::Free()
{
	CState::Free();
}
