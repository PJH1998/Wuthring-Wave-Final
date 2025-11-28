#include "ClientPch.h"
#include "AugustaGroundSprint.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"
#include "GameInstance.h"

HRESULT CAugustaGroundSprint::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaGroundSprint::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaSprintType eSprintType = context.m_eSprintType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eSprintType);

    // 4. 현재 상태 초기화
    State_Reset();

	// 5. 중력 켰다.
    m_pAugusta->Set_Gravity(true);

	// 6. SFX Motion 시작.
	m_pAugusta->Begin_Toggle_SFX(SFX_TOGGLE::MOTION);
}

void CAugustaGroundSprint::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키입력 감지.
    Handle_Input();

    // 1. 애니메이션 갱신.
	Update_RunAnimation(fTimeDelta); // 애니메이션 갱신 (및 이동/회전).

    // 2. 물리 체크.
    Check_Physics(fTimeDelta);

    // 3. 전환 제어
    Check_StateTransition(fTimeDelta);

    // 4. 현재  상태 초기화
    State_Reset();
    
}

void CAugustaGroundSprint::OnExit()
{
    CGroundState::OnExit();
    m_pAugusta->Set_Gravity(true);

	m_fFallTime = 0.f;

	// 1. SFX 모션 끄기
	m_pAugusta->End_SFX();
}

void CAugustaGroundSprint::Handle_Input()
{
    // 1. 방향 계산
    m_eDir = m_pAugusta->Calculate_Direction();

	// Dash 키입력 체크.
	m_States[DASH] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));

	m_States[HIT] = m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT)); // HIT 상태인가?
	m_States[DODGEABLE] = m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));

	m_States[DODGE] = m_States[DODGEABLE] && m_States[DASH]; // Dodge 가능하면서 Dash 키 누르면?

	if (m_States[DODGE] || m_States[HIT]) // 모든 조건 상위 조건
		return;
	m_States[FLY] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::T)) &&
		(m_pAugusta->Get_UtilityType() == UI_TAB_UTILITY::FLIGHT);

	m_States[ROPE_HOOK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::T))
		&& (m_pAugusta->Get_UtilityType() == UI_TAB_UTILITY::GRAPPLE)
		&& (m_pAugusta->Is_GrappleHook());

	m_States[ROPE_HOOK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::T))
		&& (m_pAugusta->Get_UtilityType() == UI_TAB_UTILITY::GRAPPLE)
		&& (m_pAugusta->Is_GrappleDrag());

    // 키 입력.
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey); // WASD 키입력 체크.
    
    

    m_States[RUN_U] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
    m_States[RUN_D] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
    m_States[RUN_L] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
    m_States[RUN_R] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));
    

    m_States[SKILL_E] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));
    m_States[SKILL_Q] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::Q));
    m_States[SKILL_R] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::R));



	if (m_States[SKILL_E])
	{
		// Ability System
		m_States[NORMAL_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pAugusta->Check_Skill("Skill_Hack")); // 기본 E스킬
		m_States[POINT_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pAugusta->Check_Skill("Skill_Strike"));// 그리폰
	}
	

	if (m_States[SKILL_R])
		m_States[SWORD_R] = m_States[SKILL_R] && (SKILL_STATE::READY == m_pAugusta->Check_Skill("Burst01")); // 궁극기 R스킬(검뽑는거)
	
	m_States[ECHO_R] = m_States[SKILL_R] && (SKILL_STATE::READY == m_pAugusta->Check_Skill("Attack_SpeedDrive")); // Echo 궁극기. (기본 궁극기)
	

    // DASH보다 우선순위 높음.
    m_States[SPRINT] = m_States[MOVE] && m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
    
    // 공격 상태가 아니라 공격 판정 상태로 전달.
    m_States[ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

    // 상태에 따라 속도 다르게.
    m_fSpeed = 1.2f;

	m_States[LOCKON] = m_pAugusta->Is_LockOn();


}

void CAugustaGroundSprint::Update_RunAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
	m_eDir = m_pAugusta->Calculate_Direction();
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
	

    EAugustaSprintType eSprintType = static_cast<EAugustaSprintType>(m_iCurrentAnimIdx);
    
	// 1. 회전 및 이동.
	m_pAugusta->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, m_fSpeed);
}

void CAugustaGroundSprint::Check_Physics(_float fTimeDelta)
{
	
    m_States[WALL] = m_pAugusta->Check_ClimbableWall(&m_vWallNormal); // Wall인지?
    // Land Check

	// 1. Jolt의 IsSupported()를 호출하여 땅의 Normal 벡터(m_vLandNormal)를 갱신합니다.
	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);


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


void CAugustaGroundSprint::Check_StateTransition(_float fTimeDelta)
{
 
    EAugustaSprintType eSprintType = static_cast<EAugustaSprintType>(m_iCurrentAnimIdx);

	// 1. 우선순위
	if (m_States[DODGE])
	{
		m_pAugusta->GetStateContextForWrite().m_eDodgeType = EAugustaDodgeType::MOVE_LIMIT_F;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::DODGE)); // 상위, 하위 상태
		return;
	}

	// 2.
	if (m_States[HIT])
	{
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EAugustaHitState::HIT));
		return;
	}


	if (m_States[FALL])
    {
		m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
		return;
    }

	if (m_States[ROPE_HOOK])
	{
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::INTREACTION), ENUM_CLASS(EAugustaInteractionState::ROPEHOOK));
		return;
	}

	if (m_States[ROPE_DRAG])
	{
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::INTREACTION), ENUM_CLASS(EAugustaInteractionState::ROPEHOOK));
		return;
	}

	if (m_States[FLY])
	{
		m_pAugusta->GetStateContextForWrite().m_eAirFlyType = EAugustaAirFlyType::XA_START;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FLY));
		return;
	}

    // SPACE 누르면 바로 점프로 전환.
    if (m_States[JUMP])
    {
        m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_WALK_LF;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP)); // 상위, 하위 상태
        return;
    }

	// 구현. => Burst 게이지 모두 찼을때 궁 누르면 공격기 모션.
	if (m_States[SWORD_R])
	{
		// 위에 서체크하긴 했지만? 다시 체크.
		if (SKILL_STATE::READY != m_pAugusta->Use_Skill("Burst01"))
			return;

		m_pAugusta->Bind_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
		m_pAugusta->GetStateContextForWrite().m_eBurstType = EAugustaBurstType::BURST01;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::BURST)); // 상위, 하위 상태
		return;
	}
	if (m_States[ECHO_R])
	{
		if (SKILL_STATE::READY != m_pAugusta->Use_Skill("Attack_SpeedDrive"))
			return;

		m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::ATTACK_SPEEDDRIVE;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL)); // 상위, 하위 상태
		return;
	}
	if (m_States[POINT_E])
	{
		// 위에 서체크하긴 했지만? 다시 체크.
		if (SKILL_STATE::READY != m_pAugusta->Use_Skill("Skill_Strike"))
			return;

		//m_pAugusta->Bind_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_GRIFFON));

		m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::SKILL_STRIKE;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL));
		return;
	}    // SKILL_E 누르면 => 

	if (m_States[NORMAL_E])
	{
		if (SKILL_STATE::READY != m_pAugusta->Use_Skill("Skill_Hack"))
			return;


		m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::SKILL_HACK;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL));
		return;
	}

	// Run => Attack
	if (m_States[ATTACK])
	{
		m_pAugusta->GetStateContextForWrite().m_eAttackType = EAugustaAttackType::ATTACK01;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::ATTACK)); // 상위, 하위 상태
		return;
	}


	// 뛰다가 Dash
	if (m_States[DASH])
	{
		m_pAugusta->GetStateContextForWrite().m_eDashType = EAugustaDashType::MOVE_F;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::DASH)); // 상위, 하위 상태
		return;
	}
    
	// Sprint 상태면?
	if (m_States[SPRINT])
	{
		m_iCurrentAnimIdx = ENUM_CLASS(EAugustaSprintType::SPRINT_F);
		return;
	}

    if (m_States[MOVE])
    {
        // 만약에 현재 상태가 Sprint 였으면? => 애니메이션 변경을 하지 않음.
        if (m_pAugusta->Is_LockOn())
        {
            if (m_States[RUN_U])
            {
				if (m_States[RUN_L])
					m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_LF;
                else if (m_States[RUN_R])
					m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_RF;
                else
					m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
            }
            else if (m_States[RUN_D])
            {
                if (m_States[RUN_L])
					m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_LB;
                else if (m_States[RUN_R])
					m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_RB;
                else
					m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_B;
            }
            else if (m_States[RUN_L])
				m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_LF;
            else if (m_States[RUN_R])
				m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_RF;

			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN)); // 상위, 하위 상태
            return;
        }
		else
		{
			m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN)); // 상위, 하위 상태
			return;
		}
    }


		// 이동 입력 값이 안들어왔다면?
	if (!m_States[MOVE])
	{
		// STOP SPRINT 이면서 애니메이션이 재생이 끝났다면?
		if (m_IsAnimationEnd && (eSprintType == EAugustaSprintType::STOP_SPRINT_L))
		{
			m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STANDCHANGE;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE)); // 상위, 하위 상태
			return;
		}

		if (eSprintType == EAugustaSprintType::SPRINT_F)
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaSprintType::STOP_SPRINT_L);
			return;
		}
	}
}



void CAugustaGroundSprint::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaSprintType::SPRINT_F), "Sprint_F", 1.35f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSprintType::STOP_RUN_L), "Stop_Run_L", 1.f, 0.f); // 왼발로 멈추기.
    CState::Add_Animations(ENUM_CLASS(EAugustaSprintType::STOP_SPRINT_L), "Stop_Sprint_L", 1.f, 0.f); // 왼발로 멈추기
}

void CAugustaGroundSprint::State_Reset()
{
    for (_uint i = 0; i < RUNSTATE::END; ++i)
    {
        m_States[i] = false;
    }
}



CAugustaGroundSprint* CAugustaGroundSprint::Create(class CGameObject* pOwner)
{
    CAugustaGroundSprint* pInstance = new CAugustaGroundSprint();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundSprint");
        return nullptr;
    }

    return pInstance;
}

void CAugustaGroundSprint::Free()
{
    CGroundState::Free();
}
