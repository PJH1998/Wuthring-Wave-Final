#include "ClientPch.h"
#include "AugustaAirFall.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaAirFall::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaAirFall::OnEnter(void* pArg)
{
    CAirState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaFallType eFallType = context.m_eFallType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eFallType);

    State_Reset();

    m_pAugusta->Set_Gravity(true);

}

void CAugustaAirFall::OnUpdate(_float fTimeDelta)
{
    CAirState::OnUpdate(fTimeDelta);

    // 0. 키입력 체크
    Handle_Input();

    // 1. 애니메이션 갱신
    Update_FallAnimation(fTimeDelta);

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    // 3. 전환 체크
    Check_StateTransition(fTimeDelta);

    // 상태 리셋;
    State_Reset();
}

void CAugustaAirFall::OnExit()
{
    CAirState::OnExit();
    m_pAugusta->Set_Gravity(false);
}

void CAugustaAirFall::Handle_Input()
{
    m_eDir = m_pAugusta->Calculate_Direction(); // 방향 계산.
	m_States[HIT] = m_pAugusta->Check_AnyCondition(CHARACTER_CONDITION::HIT); // HIT 상태인가?
	if (m_States[HIT])
		return;

    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
	m_States[FLY] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::T));
	m_States[ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
}

void CAugustaAirFall::Update_FallAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 플레이.
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // 조금 더 가속 주기?
    m_pAugusta->Move_Fall(fTimeDelta, 1.f);

    // 1. 조작키에 따른 이동?
    /*if (m_States[MOVE])
    {
        m_pAugusta->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, 1.f);
        return;
    }*/
}

void CAugustaAirFall::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);
}

void CAugustaAirFall::Check_StateTransition(_float fTimeDelta)
{
	if (m_States[HIT])
	{
 		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EAugustaHitState::HIT));
		return;
	}

	if (m_States[!LAND])
	{
		if (m_States[ATTACK])
		{
			m_pAugusta->GetStateContextForWrite().m_eAirAttackType = EAugustaAirAttackType::AIRATTACK_START;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::AIR_ATTACK)); // 상위, 하위 상태
			return;
		}
	}

    if (m_States[LAND])
    {
        m_pAugusta->GetStateContextForWrite().m_eLandType = EAugustaLandType::LAND_LIGHT;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
        return;
    }

	if (m_States[FLY])
	{
		m_pAugusta->GetStateContextForWrite().m_eAirFlyType = EAugustaAirFlyType::XA_START;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FLY));
		return;
	}

    /*if (m_States[LAND])
    {
        m_pAugusta->GetStateContextForWrite().m_eLandType = EAugustaLandType::LAND_LIGHT;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
        return;
    }*/

}


void CAugustaAirFall::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaFallType::FALL_LOOP), "Fall_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaFallType::FALL_LOOP_FAST), "Fall_Loop_Fast", 1.f, 0.f);
}

void CAugustaAirFall::State_Reset()
{
    for (_uint i = 0; i < FALLSTATE::END; ++i)
        m_States[i] = false;
}



CAugustaAirFall* CAugustaAirFall::Create(class CGameObject* pOwner)
{
    CAugustaAirFall* pInstance = new CAugustaAirFall();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaAirFall");
        return nullptr;
    }

    return pInstance;
}

void CAugustaAirFall::Free()
{
    CAirState::Free();
}
