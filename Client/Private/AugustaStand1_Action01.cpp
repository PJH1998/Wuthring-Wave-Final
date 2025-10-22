#include "ClientPch.h"
#include "AugustaStand1_Action01.h"
#include "InputController.h"
HRESULT CAugustaStand1_Action01::Initialize(const STATE_DATA& StateData)
{
	// 1. StateData 초기화
	CAugustaBaseState::Initialize(StateData);

	// 2. Transition 등록
	Add_Transition("Stand1_Action02", [this]()-> bool {
			if (m_IsAnimationEnd)
				return true;
			return false;
		});

	//Add_Transition("Run_F", [this]()-> bool {
	//		// 하나라도 이동이 있었다면?
	//		if (m_pPlayer->Check_AnyInput(m_iMoveKey))
	//			return true;
	//		return false;
	//	});

	//Add_Transition("Attack01", [this]()-> bool {
	//		// 왼쪽 클릭을 했다면?
	//		_uint iMouseLB =static_cast<_uint>(MOUSEKEYSTATE::LB);
	//		if (m_pPlayer->Check_AnyInput(iMouseLB))
	//			return true;
	//		return false;
	//	});


	
	
	return S_OK;
}

void CAugustaStand1_Action01::OnEnter()
{
	CState::OnEnter();
}

// 갱신. <- 외부에서 의존성을 주입받는 시기.
void CAugustaStand1_Action01::OnUpdate(_float fTimeDelta)
{
	// 1. 애니메이션 실행.
	m_IsAnimationEnd = m_pPlayer->Play_Animation(m_StateData.strAnimName
		, fTimeDelta, &m_fTrackPosition, m_StateData.fRootMotionRate, m_StateData.IsRootMotion);


}

// 탈출 조건
void CAugustaStand1_Action01::OnExit()
{

}

void CAugustaStand1_Action01::Ready_Transitions()
{

}

CAugustaStand1_Action01* CAugustaStand1_Action01::Create(const STATE_DATA& stateData)
{
	CAugustaStand1_Action01* pInstance = new CAugustaStand1_Action01();
	if (FAILED(pInstance->Initialize(stateData)))
	{
		MSG_BOX("Create Failed Augusta Stand");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAugustaStand1_Action01::Free()
{
	CState::Free();
}
