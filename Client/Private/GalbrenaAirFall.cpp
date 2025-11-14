#include "ClientPch.h"
#include "GalbrenaAirFall.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"

HRESULT CGalbrenaAirFall::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pOwner);
    ASSERT_CRASH(m_pGalbrena);

    Setup_Animations();
    return S_OK;
}



void CGalbrenaAirFall::OnEnter(void* pArg)
{
    CAirState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaFallType eFallType = context.m_eFallType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eFallType);

    State_Reset();

    m_pGalbrena->Set_Gravity(true);

}

void CGalbrenaAirFall::OnUpdate(_float fTimeDelta)
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

void CGalbrenaAirFall::OnExit()
{
    CAirState::OnExit();
    m_pGalbrena->Set_Gravity(false);
}

void CGalbrenaAirFall::Handle_Input()
{
    m_eDir = m_pGalbrena->Calculate_Direction(); // 방향 계산

	m_States[HIT] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT));
	if (m_States[HIT])
		return;

	m_States[FLY] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::T)); // 최우선 순위

    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
	m_States[AIR_ATTACK] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

	
}

void CGalbrenaAirFall::Update_FallAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 플레이.
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);

    // 조금 더 가속 주기?
    m_pGalbrena->Move_Fall(fTimeDelta, 1.f);
}

void CGalbrenaAirFall::Check_Physics(_float fTimeDelta)
{
    m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
	//m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal, 0.2f);
}

void CGalbrenaAirFall::Check_StateTransition(_float fTimeDelta)
{

	if (m_States[HIT])
	{
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EGalbrenaHitState::HIT));
		return;
	}

	if (m_States[FLY])
	{
		m_pGalbrena->GetStateContextForWrite().m_eAirFlyType = EGalbrenaAirFlyType::XA_START;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FLY));
		return;
	}


	//if (m_States[AIR_ATTACK])
	//{
	//	m_pGalbrena->GetStateContextForWrite().m_eAirAttackType = EGalbrenaAirAttackType::AIRATTACK_START;
	//	m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::AIR_ATTACK)); // 상위, 하위 상태
	//	return;
	//}


    if (m_States[LAND])
    {
        m_pGalbrena->GetStateContextForWrite().m_eLandType = EGalbrenaLandType::LAND_LIGHT;
        m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::LAND));
        return;
    }


}


void CGalbrenaAirFall::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaFallType::FALL_LOOP), "Fall_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaFallType::FALL_LOOP_FAST), "Fall_Loop_Fast", 1.f, 0.f);
}

void CGalbrenaAirFall::State_Reset()
{
    for (_uint i = 0; i < FALLSTATE::END; ++i)
        m_States[i] = false;
}



CGalbrenaAirFall* CGalbrenaAirFall::Create(class CGameObject* pOwner)
{
    CGalbrenaAirFall* pInstance = new CGalbrenaAirFall();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaAirFall");
        return nullptr;
    }

    return pInstance;
}

void CGalbrenaAirFall::Free()
{
    CAirState::Free();
}
