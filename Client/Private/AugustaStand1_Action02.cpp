#include "ClientPch.h"
#include "PlayerAugusta.h"
#include "AugustaStand1_Action02.h"
#include "InputController.h"

HRESULT CAugustaStand1_Action02::Initialize(const STATE_DATA& StateData)
{
	// 1. 초기화
	CAugustaBaseState::Initialize(StateData);

	// 2. 전환조건 등록
	Add_Transition("Stand1_Action01", [this]()-> bool {
		if (m_IsAnimationEnd)
			return true;
		return false;
		});

	return S_OK;
}

void CAugustaStand1_Action02::OnEnter()
{
	CState::OnEnter();
}

void CAugustaStand1_Action02::OnUpdate(_float fTimeDelta)
{
	// 1. 애니메이션 실행.
	m_IsAnimationEnd = m_pPlayer->Play_Animation(m_StateData.strAnimName
		, fTimeDelta, &m_fTrackPosition, m_StateData.fRootMotionRate, m_StateData.IsRootMotion);
}


// 탈출시 수행.
void CAugustaStand1_Action02::OnExit()
{
	CState::OnExit();
}



CAugustaStand1_Action02* CAugustaStand1_Action02::Create(const STATE_DATA& stateData)
{
	CAugustaStand1_Action02* pInstance = new CAugustaStand1_Action02();
	if (FAILED(pInstance->Initialize(stateData)))
	{
		MSG_BOX("Create Failed Augusta Stand");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAugustaStand1_Action02::Free()
{
	CState::Free();
}
