#include "ClientPch.h"
#include "AugustaAirJump.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaAirJump::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaAirJump::OnEnter(void* pArg)
{
    CAirState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaJumpType eJumpType = context.m_eJumpType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eJumpType);

    m_pAugusta->Set_Gravity(false);
}

void CAugustaAirJump::OnUpdate(_float fTimeDelta)
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

void CAugustaAirJump::OnExit()
{
    CAirState::OnExit();
}



void CAugustaAirJump::Handle_Input()
{
    EAugustaJumpType eJumpType = static_cast<EAugustaJumpType>(m_iCurrentAnimIdx);

    m_eDir = m_pAugusta->Calculate_Direction(); 
	m_States[HIT] = m_pAugusta->Is_Hit(); // HIT 상태인가?

	// Hit면 모든 상태 제거
	if (m_States[HIT])
		return;

	m_States[FLY] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::T));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
 
    // Double Jump
    m_States[DOUBLE_JUMP] = eJumpType == EAugustaJumpType::JUMP_WALK_LF 
        && m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT)) && CState::Is_EscapePossible();
    
    // Jump Attack
    m_States[AIR_ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
}

void CAugustaAirJump::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);

}

// 점프에 관련된 Update
void CAugustaAirJump::Update_JumpAnimation(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // 1. 조작키에 따른 이동?
    if (m_States[MOVE])
        m_pAugusta->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, 0.25f);

}



void CAugustaAirJump::Check_StateTransition(_float fTimeDelta)
{
    EAugustaJumpType eJumpType = static_cast<EAugustaJumpType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

    // 1. 우선순위 제일 높음.
	if (m_States[HIT])
	{
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EAugustaHitState::HIT));
		return;
	}

	if (m_States[FLY])
	{
		m_pAugusta->GetStateContextForWrite().m_eAirFlyType = EAugustaAirFlyType::XA_START;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FLY));
		return;
	}

    if (m_States[DOUBLE_JUMP])
    {
        m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP)); // 상위, 하위 상태
        return;
    }

    if (m_States[AIR_ATTACK] && IsEscapePossible)
    {
        m_pAugusta->GetStateContextForWrite().m_eAirAttackType = EAugustaAirAttackType::AIRATTACK_START;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::AIR_ATTACK)); // 상위, 하위 상태
        return;
    }

    // 2. 점프 애니메이션이 끝났는데도 안닿았을경우?
    if (m_IsAnimationEnd && !m_States[LAND])
    {
        m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
        return;
    }

    // 점프 도중 땅에 닿으면?
    if (m_States[LAND] && (IsEscapePossible)/* 최소 조건*/)
    {
        m_pAugusta->GetStateContextForWrite().m_eLandType = EAugustaLandType::LAND_LIGHT;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
        return;
    }
}


void CAugustaAirJump::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaJumpType::JUMP_LOOP), "Jump_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaJumpType::JUMP_RUN_LF), "Jump_Run_LF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaJumpType::JUMP_RUN_RF), "Jump_Run_RF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaJumpType::JUMP_SECOND_B), "Jump_Second_B", 1.f, 5.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaJumpType::JUMP_SECOND_F), "Jump_Second_F", 1.f, 5.f, 1.f); // 더블 점프
    CState::Add_Animations(ENUM_CLASS(EAugustaJumpType::JUMP_WALK_LF), "Jump_Walk_LF", 1.f, 10.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaJumpType::JUMP_WALK_RF), "Jump_Walk_RF", 1.f, 10.f, 1.f); // 제자리 점프
}



CAugustaAirJump* CAugustaAirJump::Create(class CGameObject* pOwner)
{
    CAugustaAirJump* pInstance = new CAugustaAirJump();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaAirJump");
        return nullptr;
    }

    return pInstance;
}

void CAugustaAirJump::Free()
{
    CAirState::Free();
}
