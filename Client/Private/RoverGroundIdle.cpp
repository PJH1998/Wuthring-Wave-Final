#include "ClientPch.h"
#include "RoverGroundIdle.h"
#include "Rover.h"
#include "StateMachine.h"
#include "Ability.h"
#include "RoverState_Enum.h"


HRESULT CRoverGroundIdle::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pCharacter);
    ASSERT_CRASH(m_pRover);

    // Idle 애니메이션 리스트 셋업
    Setup_Animations();

    // 기본 애니메이션 셋업.
    m_iCurrentAnimIdx = 0;

    // 바꿀 파트타입?
    
    return S_OK;
}

void CRoverGroundIdle::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverIdleType eIdleType = context.m_eIdleType;

    m_iCurrentAnimIdx = ENUM_CLASS(eIdleType);

    // 3. Idle 상태 초기화
    State_Reset();

	m_pRover->Set_Gravity(true);
}

void CRoverGroundIdle::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 입력 확인
    Handle_Input();

    // 1. Idle 업데이트
    Update_IdleAnimations(fTimeDelta);

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    // 3. 상태 전환.
    Check_StateTransition(fTimeDelta);

    // 4. 상태 초기화
    State_Reset();
    
}

void CRoverGroundIdle::OnExit()
{
    CGroundState::OnExit();
	m_pRover->Set_Gravity(true);
	m_fFallTime = 0.f;
}

void CRoverGroundIdle::Handle_Input()
{
	m_States[DASH] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));

	m_States[HIT] = m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT)); // HIT 상태인가?
	//m_States[DODGEABLE] = m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));

	//m_States[DODGE] = m_States[DODGEABLE] && m_States[DASH]; // Dodge 가능하면서 Dash 키 누르면?

	if (m_States[HIT] || m_States[DODGE])
		return;

	m_States[FLY] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::T))
		&& (m_pRover->Get_UtilityType() == UI_TAB_UTILITY::FLIGHT); // 최우선 순위
	m_States[ROPE_HOOK] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::T))
		&& (m_pRover->Get_UtilityType() == UI_TAB_UTILITY::GRAPPLE)
		&& (m_pRover->Is_GrappleHook());

	m_States[ROPE_DRAG] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::T))
		&& (m_pRover->Get_UtilityType() == UI_TAB_UTILITY::GRAPPLE)
		&& (m_pRover->Is_GrappleDrag());

    m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[DASH] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));
    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
    m_States[SPRINT] = m_States[MOVE] && m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));

    // LockOn인 경우에는 W, A, S, D 입력값을 모두 판별.
    m_States[MOVE_U] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
    m_States[MOVE_D] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
    m_States[MOVE_L] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
    m_States[MOVE_R] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));

	// AttackState에서 판별.
	m_States[ATTACK] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
    
    // 기본 Skill E
    m_States[SKILL_E] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));
    m_States[SKILL_Q] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::Q));
    m_States[SKILL_R] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::R));

	// Burst 상태 확인하기.
	m_States[BURST] = m_pRover->Check_AnyConidtion_FromAbility(ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE));

	if (m_States[BURST])
		m_States[BURST_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pRover->Check_Skill("Ex_Skill02"));
	else
		m_States[DEFAULT_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pRover->Check_Skill("Skill02"));

	// 궁 상태 확인하기.
	m_States[ULTI] = m_States[SKILL_R] && (m_pRover->Get_Cost(COST_TYPE::COST2) >= m_pRover->Get_MaxCost());
}



void CRoverGroundIdle::Update_IdleAnimations(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pRover, fTimeDelta);
}

void CRoverGroundIdle::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);

	if (m_States[LAND])
	{
		m_fFallTime = 0.f;
	}
	else if (!m_States[LAND])
	{
		m_fFallTime += fTimeDelta;

		if (m_fFallTime >= 0.2f)
			m_States[FALL] = true;
	}

}

// Idles 조건이 아닌 것들.
void CRoverGroundIdle::Check_StateTransition(_float fTimeDelta)
{
	// 우선순위 순으로 전환조건 진행.
    ERoverIdleType eIdleType = static_cast<ERoverIdleType>(m_iCurrentAnimIdx);

    _uint iKeyInput = {};

	// 1. 우선순위
	//if (m_States[DODGE])
	//{
	//	m_pRover->GetStateContextForWrite().m_eDodgeType = ERoverDodgeType::MOVE_LIMIT_F;
	//	m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::DODGE)); // 상위, 하위 상태
	//	return;
	//}

	if (m_States[HIT])
	{
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(ERoverHitState::HIT));
		return;
	}

	// DASH
	if (m_States[DASH])
	{
		if (m_States[MOVE_D])
		{
			m_pRover->GetStateContextForWrite().m_eDashType = ERoverDashType::MOVE_B;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::DASH)); // 상위, 하위 상태
			return;
		}
		else
		{
			m_pRover->GetStateContextForWrite().m_eDashType = ERoverDashType::MOVE_F;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::DASH)); // 상위, 하위 상태
			return;
		}
	}

	if (m_States[FALL])
	{
		m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL)); // 상위, 하위 상태
		return;
	}
	
	if (m_States[FLY])
	{
		m_pRover->GetStateContextForWrite().m_eAirFlyType = ERoverAirFlyType::XA_START;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FLY));
		return;
	}

	if (m_States[ROPE_HOOK])
	{
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::INTREACTION), ENUM_CLASS(ERoverInteractionState::ROPEHOOK));
		return;
	}

	if (m_States[ROPE_DRAG])
	{
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::INTREACTION), ENUM_CLASS(ERoverInteractionState::ROPEDRAG));
		return;
	}

    if (m_States[JUMP])
    {
        m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_WALK_LF;
        m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP)); // 상위, 하위 상태
        return;
    }

	if (m_States[ULTI])
	{
		if (SKILL_STATE::READY != m_pRover->Use_Skill("Burst01_Ulti"))
			return;

		m_pRover->GetStateContextForWrite().m_eBurstType = ERoverBurstType::BURST01;
		m_pRover->GetStateContextForWrite().m_strPrevInfo = "ULTI";
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::BURST));
		return;
	}

	if (m_States[BURST_E]) // Burst E
	{
		if (SKILL_STATE::READY != m_pRover->Use_Skill("Ex_Skill02"))
			return;

		m_pRover->GetStateContextForWrite().m_eSkillType = ERoverSkillType::EX_SKILL02;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::SKILL));
		return;
	}

	if (m_States[DEFAULT_E])
	{
		if (SKILL_STATE::READY != m_pRover->Use_Skill("Skill02"))
			return;

		m_pRover->GetStateContextForWrite().m_eSkillType = ERoverSkillType::SKILL02;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::SKILL));
		return;
	}


	// Idle => Attack
	if (m_States[ATTACK])
	{
		// Burst 상태라면 Special 상태로?
		if (m_States[BURST])
		{
			m_pRover->GetStateContextForWrite().m_eSpecialType = ERoverSpecialType::EX_ATTACK01;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::SPECIAL)); // 상위, 하위 상태
			return;
		}
		else
		{
			m_pRover->GetStateContextForWrite().m_eAttackType = ERoverAttackType::ATTACK01;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::ATTACK)); // 상위, 하위 상태
			return;
		}
		
	}

	

	// SPRINT
	if (m_States[SPRINT])
	{
		m_pRover->GetStateContextForWrite().m_eSprintType = ERoverSprintType::SPRINT_F;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::SPRINT)); // 상위, 하위 상태
		return;
	}


    
    // 이동은 Run State에서 조절.
    if (m_States[MOVE])
    {
        // 더 우선순위 높은 것. => Sprint
        // LockOn일때 전환 로직 변경.
        if (m_pRover->Is_LockOn())
        {
            if (m_States[MOVE_U])
            {
                if (m_States[MOVE_L])
                    m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_LF; // 애니메이션 상태 => 블랙보드에 기입.        
                else if (m_States[MOVE_R])
                    m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_RF; // 애니메이션 상태 => 블랙보드에 기입.        
                else
                    m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        
            }
            else if (m_States[MOVE_D])
            {
                if (m_States[MOVE_L])
                    m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_LB; // 애니메이션 상태 => 블랙보드에 기입.        
                else if (m_States[MOVE_R])
                    m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_RB; // 애니메이션 상태 => 블랙보드에 기입.        
                else
                    m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_B; // 애니메이션 상태 => 블랙보드에 기입.        
            }
            else if (m_States[MOVE_L])
                m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_LF; // 애니메이션 상태 => 블랙보드에 기입.        
            else if (m_States[MOVE_R])
                m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_RF; // 애니메이션 상태 => 블랙보드에 기입.        
        }
        else
            m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        

        m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN)); // 상위, 하위 상태
        return;
    }

	if (m_IsAnimationEnd)
	{
		switch (static_cast<ERoverIdleType>(m_iCurrentAnimIdx))
		{
		case ERoverIdleType::STANDCHANGE:
			m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1_ACTION02; // 애니메이션 상태 => 블랙보드에 기입.      
			break;
		case ERoverIdleType::STAND1_ACTION02:
			m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1_ACTION03; // 애니메이션 상태 => 블랙보드에 기입.      
			break;
		case ERoverIdleType::STAND1_ACTION03:
			m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1_ACTION02; // 애니메이션 상태 => 블랙보드에 기입.      
			break;
		default:
			m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1_ACTION02; // 애니메이션 상태 => 블랙보드에 기입.      
			break;
		}

		m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
		return;
	}
}



void CRoverGroundIdle::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERoverIdleType::STAND1_ACTION01), "Stand1_Action01", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverIdleType::STAND1_ACTION02), "Stand1_Action02", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverIdleType::STAND1_ACTION03), "Stand1_Action03", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverIdleType::STAND1), "Stand1", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverIdleType::STAND2), "Stand2", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverIdleType::STAND_CONTROL), "Stand_Control", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverIdleType::STANDCHANGE), "StandChange", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverIdleType::STANDUP), "StandUp", 1.f, 0.f);
}


void CRoverGroundIdle::State_Reset()
{
    for (_uint i = 0; i < IDLESTATE::END; ++i)
        m_States[i] = false;
}

CRoverGroundIdle* CRoverGroundIdle::Create(CCharacter* pOwner)
{
    CRoverGroundIdle* pInstance = new CRoverGroundIdle();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverGroundIdle");
        return nullptr;
    }

    return pInstance;
}

void CRoverGroundIdle::Free()
{
    CGroundState::Free();
}
