#include "ClientPch.h"
#include "RoverAirFall.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"

HRESULT CRoverAirFall::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pOwner);
    ASSERT_CRASH(m_pRover);

    Setup_Animations();
    return S_OK;
}



void CRoverAirFall::OnEnter(void* pArg)
{
    CAirState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverFallType eFallType = context.m_eFallType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eFallType);

    State_Reset();

    m_pRover->Set_Gravity(true);

}

void CRoverAirFall::OnUpdate(_float fTimeDelta)
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

void CRoverAirFall::OnExit()
{
    CAirState::OnExit();
    m_pRover->Set_Gravity(false);
}

void CRoverAirFall::Handle_Input()
{
    m_eDir = m_pRover->Calculate_Direction(); // 방향 계산

	m_States[HIT] = m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT));
	if (m_States[HIT])
		return;

	m_States[FLY] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::T)) &&
		(m_pRover->Get_UtilityType() == UI_TAB_UTILITY::FLIGHT);

    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
	m_States[AIR_ATTACK] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

	
}

void CRoverAirFall::Update_FallAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 플레이.
    CCharacterState::Play_Animation(m_pRover, fTimeDelta);

    // 조금 더 가속 주기?
    m_pRover->Move_Fall(fTimeDelta, 1.f);
}

void CRoverAirFall::Check_Physics(_float fTimeDelta)
{
    m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
	//m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal, 0.2f);
}

void CRoverAirFall::Check_StateTransition(_float fTimeDelta)
{

	if (m_States[HIT])
	{
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(ERoverHitState::HIT));
		return;
	}

	if (m_States[FLY])
	{
		m_pRover->GetStateContextForWrite().m_eAirFlyType = ERoverAirFlyType::XA_START;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FLY));
		return;
	}


	if (m_States[AIR_ATTACK])
	{
		m_pRover->GetStateContextForWrite().m_eAirAttackType = ERoverAirAttackType::AIRATTACK_START;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::AIR_ATTACK)); // 상위, 하위 상태
		return;
	}


    if (m_States[LAND])
    {
        m_pRover->GetStateContextForWrite().m_eLandType = ERoverLandType::LAND_LIGHT;
        m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::LAND));
        return;
    }


}


void CRoverAirFall::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERoverFallType::FALL_LOOP), "Fall_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverFallType::FALL_LOOP_FAST), "Fall_Loop_Fast", 1.f, 0.f);
}

void CRoverAirFall::State_Reset()
{
    for (_uint i = 0; i < FALLSTATE::END; ++i)
        m_States[i] = false;
}



CRoverAirFall* CRoverAirFall::Create(class CGameObject* pOwner)
{
    CRoverAirFall* pInstance = new CRoverAirFall();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverAirFall");
        return nullptr;
    }

    return pInstance;
}

void CRoverAirFall::Free()
{
    CAirState::Free();
}
