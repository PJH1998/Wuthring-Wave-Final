#include "ClientPch.h"
#include "GalbrenaRope.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"

HRESULT CGalbrenaRope::Initialize(class CGameObject* pOwner)
{
	if (FAILED(CInteractionState::Initialize(pOwner)))
		return E_FAIL;

	m_pGalbrena = dynamic_cast<CGalbrena*>(pOwner);
	ASSERT_CRASH(m_pGalbrena);

	Setup_Animations();
	return S_OK;
}

void CGalbrenaRope::OnEnter(void* pArg)
{
	CInteractionState::OnEnter(pArg);

	// 1. 복사본 context 받아오기.
	const auto context = m_pGalbrena->TakeStateContext();

	// 2. 복사본에서 필요한 값 읽기
	EGalbrenaHitType eHitType = context.m_eHitType;

	// 3. 값에 따른 상태 변경.
	m_iCurrentAnimIdx = ENUM_CLASS(eHitType);

	// 4. 상태 리셋.
	State_Reset();

	// 5. Hit Description을 이용하여 시작 초기 작업을 정의합니다.
	Enter_Rope();

	// 6. 중력 적용
	m_pGalbrena->Set_Gravity(true);
}

void CGalbrenaRope::OnUpdate(_float fTimeDelta)
{
	CInteractionState::OnUpdate(fTimeDelta);

	// 0. 키입력 체크
	Handle_Input();

	// 1. 애니메이션 갱신
	Update_RopeAnimation(fTimeDelta);

	// 2. 물리 체크
	Check_Physics(fTimeDelta);

	// 3. 전환 체크
	Check_StateTransition(fTimeDelta);

	// 상태 리셋;
	State_Reset();
}

void CGalbrenaRope::OnExit()
{
	CInteractionState::OnExit();
	m_pGalbrena->Set_Gravity(false);

}



void CGalbrenaRope::Enter_Rope()
{
}

void CGalbrenaRope::Handle_Input()
{
	m_eDir = m_pGalbrena->Calculate_Direction(); // 방향 계산.
	m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
	m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));

}

void CGalbrenaRope::Update_RopeAnimation(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);
	EGalbrenaRopeType eRopeType = static_cast<EGalbrenaRopeType>(m_iCurrentAnimIdx);
	
}

void CGalbrenaRope::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
}

void CGalbrenaRope::Check_StateTransition(_float fTimeDelta)
{
	EGalbrenaRopeType eRopeType = static_cast<EGalbrenaRopeType>(m_iCurrentAnimIdx);

	_bool IsEscapePossible = CState::Is_EscapePossible();

	if (IsEscapePossible)
	{
		if (m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
				return;
			}

			if (m_States[JUMP])
			{
				m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
				return;
			}
		}

		if (!m_States[LAND])
		{
			if (m_States[JUMP])
			{
				m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
				return;
			}
		}
	}

	if (m_IsAnimationEnd)
	{
		
	}


}


void CGalbrenaRope::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeType::FIXHOOK_END), "FixHook_End", 1.f, 40.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaRopeType::FIXHOOK_END_FAST), "FixHook_End_Fast", 1.f, 40.f);

}

void CGalbrenaRope::State_Reset()
{
	for (_uint i = 0; i < ROPESTATE::END; ++i)
		m_States[i] = false;
}



CGalbrenaRope* CGalbrenaRope::Create(class CGameObject* pOwner)
{
	CGalbrenaRope* pInstance = new CGalbrenaRope();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CGalbrenaRope");
		return nullptr;
	}

	return pInstance;
}

void CGalbrenaRope::Free()
{
	CInteractionState::Free();
}
