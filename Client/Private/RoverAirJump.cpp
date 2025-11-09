#include "ClientPch.h"
#include "RoverAirJump.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"

HRESULT CRoverAirJump::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pOwner);
    ASSERT_CRASH(m_pRover);

    Setup_Animations();
    return S_OK;
}



void CRoverAirJump::OnEnter(void* pArg)
{
    CAirState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverJumpType eJumpType = context.m_eJumpType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eJumpType);

    m_pRover->Set_Gravity(true);

}

void CRoverAirJump::OnUpdate(_float fTimeDelta)
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

void CRoverAirJump::OnExit()
{
    CAirState::OnExit();
}



void CRoverAirJump::Handle_Input()
{
    ERoverJumpType eJumpType = static_cast<ERoverJumpType>(m_iCurrentAnimIdx);

    m_eDir = m_pRover->Calculate_Direction(); 
    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
 
    // Double Jump
    m_States[DOUBLE_JUMP] = eJumpType == ERoverJumpType::JUMP_WALK_LF 
        && m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT)) && CState::Is_EscapePossible();
    
    // Jump Attack
    m_States[AIR_ATTACK] =  m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
}

void CRoverAirJump::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
	//m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal, 0.2f);
}

// 점프에 관련된 Update
void CRoverAirJump::Update_JumpAnimation(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pRover, fTimeDelta);

    // 1. 조작키에 따른 이동?
    if (m_States[MOVE])
        m_pRover->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, 0.25f);

}



void CRoverAirJump::Check_StateTransition(_float fTimeDelta)
{
    ERoverJumpType eJumpType = static_cast<ERoverJumpType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

    // 1. 우선순위 제일 높음.
    if (m_States[DOUBLE_JUMP])
    {
        m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_SECOND_F;
        m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP)); // 상위, 하위 상태
        return;
    }

	if (m_States[AIR_ATTACK] && IsEscapePossible)
	{
		m_pRover->GetStateContextForWrite().m_eAirAttackType = ERoverAirAttackType::AIRATTACK_START;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::AIR_ATTACK)); // 상위, 하위 상태
		return;
	}

    // 2. 점프 애니메이션이 끝났는데도 안닿았을경우?
    if (m_IsAnimationEnd && !m_States[LAND])
    {
        m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
        m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL)); // 상위, 하위 상태
        return;
    }

    // 점프 도중 땅에 닿으면?
    if (m_States[LAND] && (IsEscapePossible)/* 최소 조건*/)
    {
        m_pRover->GetStateContextForWrite().m_eLandType = ERoverLandType::LAND_LIGHT;
        m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::LAND));
        return;
    }
}


void CRoverAirJump::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERoverJumpType::JUMP_LOOP), "Jump_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverJumpType::JUMP_RUN_LF), "Jump_Run_LF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverJumpType::JUMP_RUN_RF), "Jump_Run_RF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverJumpType::JUMP_SECOND_B), "Jump_Second_B", 1.f, 5.f, 3.f);
    CState::Add_Animations(ENUM_CLASS(ERoverJumpType::JUMP_SECOND_F), "Jump_Second_F", 1.f, 5.f, 3.f); // 더블 점프
    CState::Add_Animations(ENUM_CLASS(ERoverJumpType::JUMP_WALK_LF), "Jump_Walk_LF", 1.f, 10.f, 3.f);
    CState::Add_Animations(ENUM_CLASS(ERoverJumpType::JUMP_WALK_RF), "Jump_Walk_RF", 1.f, 10.f, 3.f); // 제자리 점프
}



CRoverAirJump* CRoverAirJump::Create(class CGameObject* pOwner)
{
    CRoverAirJump* pInstance = new CRoverAirJump();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverAirJump");
        return nullptr;
    }

    return pInstance;
}

void CRoverAirJump::Free()
{
    CAirState::Free();
}
