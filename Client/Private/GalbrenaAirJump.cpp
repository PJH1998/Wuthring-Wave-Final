#include "ClientPch.h"
#include "GalbrenaAirJump.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"

HRESULT CGalbrenaAirJump::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pOwner);
    ASSERT_CRASH(m_pGalbrena);

    Setup_Animations();
    return S_OK;
}



void CGalbrenaAirJump::OnEnter(void* pArg)
{
    CAirState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaJumpType eJumpType = context.m_eJumpType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eJumpType);

    m_pGalbrena->Set_Gravity(false);

}

void CGalbrenaAirJump::OnUpdate(_float fTimeDelta)
{
    CAirState::OnUpdate(fTimeDelta);

    // 0. 키입력 체크
    Handle_Input();

    // 1. 애니메이션 플레이.
    Update_JumpAnimation(fTimeDelta);

    // 2. 물리 체크.
    Check_Physics(fTimeDelta);

    // 3. 전환 조건 체크.
    Check_StateTransition(fTimeDelta);
}

void CGalbrenaAirJump::OnExit()
{
    CAirState::OnExit();
}



void CGalbrenaAirJump::Handle_Input()
{
    EGalbrenaJumpType eJumpType = static_cast<EGalbrenaJumpType>(m_iCurrentAnimIdx);

    m_eDir = m_pGalbrena->Calculate_Direction(); 
	m_States[HIT] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT));

	// Hit면 모든 상태 제거
	if (m_States[HIT])
		return;

	m_States[FLY] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::T)) &&
		(m_pGalbrena->Get_UtilityType() == UI_TAB_UTILITY::FLIGHT);

    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
 
    // Double Jump
    m_States[DOUBLE_JUMP] = eJumpType == EGalbrenaJumpType::JUMP_WALK_LF 
        && m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT)) && CState::Is_EscapePossible();
    
    // Jump Attack
    m_States[AIR_ATTACK] =  m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
}

void CGalbrenaAirJump::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
}

// 점프에 관련된 Update
void CGalbrenaAirJump::Update_JumpAnimation(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);

    // 1. 조작키에 따른 이동?
    if (m_States[MOVE])
        m_pGalbrena->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, 0.25f);

}



void CGalbrenaAirJump::Check_StateTransition(_float fTimeDelta)
{
    EGalbrenaJumpType eJumpType = static_cast<EGalbrenaJumpType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. 우선순위 제일 높음.
	if (m_States[HIT])
	{
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EGalbrenaHitState::HIT));
		return;
	}

	// 2. 날 수 있다면?
	if (m_States[FLY])
	{
		m_pGalbrena->GetStateContextForWrite().m_eAirFlyType = EGalbrenaAirFlyType::XA_START;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FLY));
		return;
	}

    if (m_States[DOUBLE_JUMP])
    {
        m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F;
        m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP)); // 상위, 하위 상태
        return;
    }

	if (m_States[AIR_ATTACK] && IsEscapePossible)
	{
		m_pGalbrena->GetStateContextForWrite().m_eAirAttackType = EGalbrenaAirAttackType::AIRATTACK_START;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::AIR_ATTACK)); // 상위, 하위 상태
		return;
	}

    // 2. 점프 애니메이션이 끝났는데도 안닿았을경우?
    if (m_IsAnimationEnd && !m_States[LAND])
    {
        m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
        m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL)); // 상위, 하위 상태
        return;
    }

    // 점프 도중 땅에 닿으면?
    if (m_States[LAND] && (IsEscapePossible)/* 최소 조건*/)
    {
        m_pGalbrena->GetStateContextForWrite().m_eLandType = EGalbrenaLandType::LAND_HEAVY;
        m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::LAND));
        return;
    }
}


void CGalbrenaAirJump::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaJumpType::JUMP_LOOP), "Jump_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaJumpType::JUMP_RUN_LF), "Jump_Run_LF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaJumpType::JUMP_RUN_RF), "Jump_Run_RF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaJumpType::JUMP_SECOND_B), "Jump_Second_B", 1.f, 5.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaJumpType::JUMP_SECOND_F), "Jump_Second_F", 1.f, 5.f, 1.f); // 더블 점프
    CState::Add_Animations(ENUM_CLASS(EGalbrenaJumpType::JUMP_WALK_LF), "Jump_Walk_LF", 1.f, 10.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaJumpType::JUMP_WALK_RF), "Jump_Walk_RF", 1.f, 10.f, 1.f); // 제자리 점프
}



CGalbrenaAirJump* CGalbrenaAirJump::Create(class CGameObject* pOwner)
{
    CGalbrenaAirJump* pInstance = new CGalbrenaAirJump();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaAirJump");
        return nullptr;
    }

    return pInstance;
}

void CGalbrenaAirJump::Free()
{
    CAirState::Free();
}
