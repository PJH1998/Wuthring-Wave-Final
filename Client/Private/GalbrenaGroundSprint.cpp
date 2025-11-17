#include "ClientPch.h"
#include "GalbrenaGroundSprint.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"
#include "GameInstance.h"

HRESULT CGalbrenaGroundSprint::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pOwner);
    ASSERT_CRASH(m_pGalbrena);

    Setup_Animations();
    return S_OK;
}



void CGalbrenaGroundSprint::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaSprintType eSprintType = context.m_eSprintType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eSprintType);

    // 4. 현재 상태 초기화
    State_Reset();

	// 5. 중력 켰다.
    m_pGalbrena->Set_Gravity(true);

	// 6. SFX Motion 시작.
	m_pGalbrena->Begin_Toggle_SFX(SFX_TOGGLE::MOTION);
}

void CGalbrenaGroundSprint::OnUpdate(_float fTimeDelta)
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

void CGalbrenaGroundSprint::OnExit()
{
    CGroundState::OnExit();
    m_pGalbrena->Set_Gravity(true);

	m_fFallTime = 0.f;
	m_pGalbrena->End_SFX();
}

void CGalbrenaGroundSprint::Handle_Input()
{
	// 1. 방향 계산
	m_eDir = m_pGalbrena->Calculate_Direction();

	// Dash 키입력 체크.
	m_States[DASH] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));

	m_States[HIT] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT)); // HIT 상태인가?
	m_States[DODGEABLE] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));

	m_States[DODGE] = m_States[DODGEABLE] && m_States[DASH]; // Dodge 가능하면서 Dash 키 누르면?

	if (m_States[DODGE] || m_States[HIT]) // 모든 조건 상위 조건
		return;

	m_States[FLY] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::T));

	// 키 입력.
	m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
	m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey); // WASD 키입력 체크.
	m_States[DASH] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));

	m_States[RUN_U] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
	m_States[RUN_D] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
	m_States[RUN_L] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
	m_States[RUN_R] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));


	m_States[SKILL_E] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));
	m_States[SKILL_Q] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::Q));
	m_States[SKILL_R] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::R));


	// DASH보다 우선순위 높음.
	m_States[SPRINT] = m_States[MOVE] && m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));

	// 공격 상태가 아니라 공격 판정 상태로 전달.
	m_States[ATTACK] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

	// 상태에 따라 속도 다르게.
	m_fSpeed = 1.2f;

	// Burst인지 체크
	//m_States[BURST] = m_pGalbrena->Check_AnyConidtion_FromAbility(ENUM_CLASS(UI_GALBRENA_CONDITION::BURST_ACTIVE));

	//if (m_States[BURST])
	//	m_States[BURST_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pGalbrena->Check_Skill("Ex_Skill02"));
	//else
	//	m_States[DEFAULT_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pGalbrena->Check_Skill("Skill02"));

	m_States[DEFAULT_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pGalbrena->Check_Skill("Attack_Jump_Start"));

	// 궁 상태 확인하기.
	//m_States[ULTI] = m_States[SKILL_R] && (m_pGalbrena->Get_Cost(COST_TYPE::COST2) >= m_pGalbrena->Get_MaxCost());
}





void CGalbrenaGroundSprint::Update_RunAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
	m_eDir = m_pGalbrena->Calculate_Direction();
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);
	

    EGalbrenaSprintType eSprintType = static_cast<EGalbrenaSprintType>(m_iCurrentAnimIdx);
    // 1. 회전 및 이동.

	m_pGalbrena->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, m_fSpeed);
}

void CGalbrenaGroundSprint::Check_Physics(_float fTimeDelta)
{
	
    m_States[WALL] = m_pGalbrena->Check_ClimbableWall(&m_vWallNormal); // Wall인지?
    // Land Check

	// 1. Jolt의 IsSupported()를 호출하여 땅의 Normal 벡터(m_vLandNormal)를 갱신합니다.
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);


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


void CGalbrenaGroundSprint::Check_StateTransition(_float fTimeDelta)
{
 
    EGalbrenaSprintType eSprintType = static_cast<EGalbrenaSprintType>(m_iCurrentAnimIdx);

	// 1. 우선순위
	if (m_States[DODGE])
	{
		m_pGalbrena->GetStateContextForWrite().m_eDodgeType = EGalbrenaDodgeType::MOVE_LIMIT_F;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::DODGE)); // 상위, 하위 상태
		return;
	}

	// 2.Hit 상태.
	if (m_States[HIT])
	{
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EGalbrenaHitState::HIT));
		return;
	}


	if (m_States[FALL])
    {
		m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL)); // 상위, 하위 상태
		return;
    }

	if (m_States[FLY])
	{
		m_pGalbrena->GetStateContextForWrite().m_eAirFlyType = EGalbrenaAirFlyType::XA_START;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FLY));
		return;
	}

    // SPACE 누르면 바로 점프로 전환.
    if (m_States[JUMP])
    {
        m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_WALK_LF;
        m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP)); // 상위, 하위 상태
        return;
    }

	if (m_States[DEFAULT_E])
	{
		if (SKILL_STATE::READY != m_pGalbrena->Use_Skill("Attack_Jump_Start"))
			return;

		m_pGalbrena->GetStateContextForWrite().m_eSkillType = EGalbrenaSkillType::ATTACK_JUMP_START;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::SKILL)); // 상위, 하위 상태
		return;
	}

	if (m_States[ATTACK])
	{
		// Burst 상태라면 Special 상태로?
		m_pGalbrena->GetStateContextForWrite().m_eAttackType = EGalbrenaAttackType::ATTACK01;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::ATTACK)); // 상위, 하위 상태
		return;
	}

	// 뛰다가 Dash
	if (m_States[DASH])
	{
		m_pGalbrena->GetStateContextForWrite().m_eDashType = EGalbrenaDashType::MOVE_F;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::DASH)); // 상위, 하위 상태
		return;
	}
    
	// Sprint 상태면?
	if (m_States[SPRINT])
	{
		m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaSprintType::SPRINT_F);
		return;
	}

    if (m_States[MOVE])
    {
        // 만약에 현재 상태가 Sprint 였으면? => 애니메이션 변경을 하지 않음.
        if (m_pGalbrena->Is_LockOn())
        {
            if (m_States[RUN_U])
            {
				if (m_States[RUN_L])
					m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_LF;
                else if (m_States[RUN_R])
					m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_RF;
                else
					m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
            }
            else if (m_States[RUN_D])
            {
                if (m_States[RUN_L])
					m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_LB;
                else if (m_States[RUN_R])
					m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_RB;
                else
					m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_B;
            }
            else if (m_States[RUN_L])
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_LF;
            else if (m_States[RUN_R])
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_RF;

			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN)); // 상위, 하위 상태
            return;
        }
		else
		{
			m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN)); // 상위, 하위 상태
			return;
		}
    }


		// 이동 입력 값이 안들어왔다면?
	if (!m_States[MOVE])
	{
		// STOP SPRINT 이면서 애니메이션이 재생이 끝났다면?
		if (m_IsAnimationEnd && (eSprintType == EGalbrenaSprintType::STOP_SPRINT_L))
		{
			m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND1;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE)); // 상위, 하위 상태
			return;
		}

		if (eSprintType == EGalbrenaSprintType::SPRINT_F)
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaSprintType::STOP_SPRINT_L);
			return;
		}
	}
}



void CGalbrenaGroundSprint::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaSprintType::SPRINT_F), "Sprint_F", 1.35f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaSprintType::STOP_RUN_L), "Stop_Run_L", 1.f, 0.f); // 왼발로 멈추기.
    CState::Add_Animations(ENUM_CLASS(EGalbrenaSprintType::STOP_SPRINT_L), "Stop_Sprint_L", 1.f, 0.f); // 왼발로 멈추기
}

void CGalbrenaGroundSprint::State_Reset()
{
    for (_uint i = 0; i < RUNSTATE::END; ++i)
    {
        m_States[i] = false;
    }
}



CGalbrenaGroundSprint* CGalbrenaGroundSprint::Create(class CGameObject* pOwner)
{
    CGalbrenaGroundSprint* pInstance = new CGalbrenaGroundSprint();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaGroundSprint");
        return nullptr;
    }

    return pInstance;
}

void CGalbrenaGroundSprint::Free()
{
    CGroundState::Free();
}
