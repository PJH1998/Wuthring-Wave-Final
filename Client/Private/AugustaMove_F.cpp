#include "ClientPch.h"
#include "AugustaMove_F.h"
HRESULT CAugustaMove_F::Initialize(const STATE_DATA& StateData)
{
	// 1. StateData 초기화
	CAugustaBaseState::Initialize(StateData);

	// 2. Transition 등록
	Ready_Transitions();
	
	return S_OK;
}

void CAugustaMove_F::OnEnter()
{
	CState::OnEnter();
}

// 갱신. <- 외부에서 의존성을 주입받는 시기.
void CAugustaMove_F::OnUpdate(_float fTimeDelta)
{
	// 1. 애니메이션 실행.
	m_IsAnimationEnd = m_pPlayer->Play_Animation(m_StateData.strAnimName
		, fTimeDelta, &m_fTrackPosition, m_StateData.fRootMotionRate, m_StateData.IsRootMotion);

	// 2. 수행할 작업.
	m_pPlayer->Check_AnyInput(KEYINPUT::W);

}

// 탈출시 실행.
void CAugustaMove_F::OnExit()
{

}

void CAugustaMove_F::Ready_Transitions()
{
	Add_Transition("Stand1_Action01", [this]()-> bool {
		if (m_IsAnimationEnd)
			return true;
		return false;
	});
}

CAugustaMove_F* CAugustaMove_F::Create(const STATE_DATA& stateData)
{
	CAugustaMove_F* pInstance = new CAugustaMove_F();
	if (FAILED(pInstance->Initialize(stateData)))
	{
		MSG_BOX("Create Failed Augusta Stand");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAugustaMove_F::Free()
{
	CState::Free();
}
