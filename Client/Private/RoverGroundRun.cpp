#include "ClientPch.h"
#include "RoverGroundRun.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"

HRESULT CRoverGroundRun::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pOwner);
    ASSERT_CRASH(m_pRover);

    Setup_Animations();
    return S_OK;
}



void CRoverGroundRun::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverRunType eRunType = context.m_eRunType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eRunType);

    // 4. 현재 상태 초기화
    State_Reset();

    m_pRover->Set_Gravity(true);
}

void CRoverGroundRun::OnUpdate(_float fTimeDelta)
{
    
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키입력 감지.
    Handle_Input();

    // 1. 애니메이션 갱신.
    Update_RunAnimation(fTimeDelta);

    // 2. 물리 체크.
    Check_Physics(fTimeDelta);

    // 3. 전환 제어
    Check_StateTransition(fTimeDelta);

    // 4. 현재  상태 초기화
    State_Reset();
    
}

void CRoverGroundRun::OnExit()
{
    CGroundState::OnExit();
    m_pRover->Set_Gravity(true);
	m_fFallTime = 0.f;
}

void CRoverGroundRun::Handle_Input()
{
    // 1. 방향 계산
    m_eDir = m_pRover->Calculate_Direction();

	m_States[HIT] = m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT));
	if (m_States[HIT]) // 모든 조건 상위 조건
		return;
	m_States[FLY] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::T));

    // 키 입력.
    m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey); // WASD 키입력 체크.
    m_States[DASH] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));

    m_States[RUN_U] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
    m_States[RUN_D] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
    m_States[RUN_L] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
    m_States[RUN_R] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));
    

    m_States[SKILL_E] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));
    m_States[SKILL_Q] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::Q));
    m_States[SKILL_R] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::R));


    // DASH보다 우선순위 높음.
    m_States[SPRINT_F] = m_States[MOVE] && m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));

    // 공격 상태가 아니라 공격 판정 상태로 전달.
    m_States[ATTACK] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

    // 상태에 따라 속도 다르게.
    m_fSpeed = m_States[SPRINT_F] ? 1.2f : 0.7f;

	// Burst인지 체크
	m_States[BURST] = m_pRover->Check_AnyConidtion_FromAbility(ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE));

	if (m_States[BURST])
		m_States[BURST_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pRover->Check_Skill("Ex_Skill02"));
	else
		m_States[DEFAULT_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pRover->Check_Skill("Skill02"));

	// 궁 상태 확인하기.
	m_States[ULTI] = m_States[SKILL_R] && (m_pRover->Get_Cost(COST_TYPE::COST2) >= m_pRover->Get_MaxCost());
}


void CRoverGroundRun::Update_RunAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pRover, fTimeDelta);


    ERoverRunType eRunType = static_cast<ERoverRunType>(m_iCurrentAnimIdx);
    // 1. 회전 및 이동.
    if (m_pRover->Is_LockOn())
    {
        if (eRunType == ERoverRunType::SPRINT_F || eRunType == ERoverRunType::STOP_SPRINT_L)
            m_pRover->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, m_fSpeed);
        else 
            // 1. WASD 입력에 따른 8방향 이동
            m_pRover->Move_LockOn_8Way(m_eDir, fTimeDelta, m_fSpeed);
    }
    else 
        m_pRover->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, m_fSpeed);


}

void CRoverGroundRun::Check_Physics(_float fTimeDelta)
{
    // Wall인지?
    m_States[WALL] = m_pRover->Check_ClimbableWall(&m_vWallNormal);

	// 1. Jolt의 IsSupported()를 호출하여 땅의 Normal 벡터(m_vLandNormal)를 갱신합니다.
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


void CRoverGroundRun::Check_StateTransition(_float fTimeDelta)
{
 
    ERoverRunType eRunType = static_cast<ERoverRunType>(m_iCurrentAnimIdx);
    _float3 vNormal = {}; // 벽타기 전환 용도 Normal
    // 이 조건은 추후 디테일 잡아보기.

		// 상위, 하위 상태
	if (m_States[HIT])
	{
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(ERoverHitState::HIT));
		return;
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

    if (m_States[JUMP]) // SPACE 누르면 바로 점프로 전환.
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

	// Run => Attack
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


	// 뛰다가 Dash
	if (m_States[DASH])
	{
		if (m_States[RUN_D])
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

    // Dash 보다 우선순위 높음.
    if (m_States[SPRINT_F])
    {
        m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::SPRINT_F);
        return;
    }

   

    if (m_States[MOVE])
    {
        // 만약에 현재 상태가 Sprint 였으면? => 애니메이션 변경을 하지 않음.
        if (m_pRover->Is_LockOn())
        {
            if (m_States[RUN_U])
            {
                if (m_States[RUN_L])
                    m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::RUN_LF);
                else if (m_States[RUN_R])
                    m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::RUN_RF);
                else
                    m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::RUN_F);
            }
            else if (m_States[RUN_D])
            {
                if (m_States[RUN_L])
                    m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::RUN_LB);
                else if (m_States[RUN_R])
                    m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::RUN_RB);
                else
                    m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::RUN_B);
            }
            else if (m_States[RUN_L])
                m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::RUN_LF);
            else if (m_States[RUN_R])
                m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::RUN_RF);

            return;
        }
        // 이동 값이 들어왔는데 Stop Run 상태라면?
        if (eRunType == ERoverRunType::STOP_RUN_L || eRunType == ERoverRunType::SPRINT_F)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::RUN_F);
            return;
        }
		else
		{
			m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::RUN_F);
			return;
		}
    }

    // 이동 입력 값이 안들어왔다면?
    if (!m_States[MOVE])
    {
        // 현재 상태가 Sprint 였다면?
        if (eRunType == ERoverRunType::SPRINT_F)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::STOP_SPRINT_L);
            return;
        }

        // 현재 상태가 STOP_RUN이 아니라면? => STOP RUN
        if (eRunType != ERoverRunType::STOP_RUN_L)
        {
			m_fTrackPosition = 0.f;
            m_iCurrentAnimIdx = ENUM_CLASS(ERoverRunType::STOP_RUN_L);
            return;
        }
        // Stop Run 이면서 애니메이션 재생이 끝났다면?.
        if ((eRunType == ERoverRunType::STOP_RUN_L || eRunType == ERoverRunType::STOP_SPRINT_L) && m_IsAnimationEnd)
        {
			
            m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1;
            m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
            return;
        }
    }

    
}



void CRoverGroundRun::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::RUN_B), "Run_B", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::RUN_F), "Run_F", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::RUN_LB), "Run_LB", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::RUN_LF), "Run_LF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::RUN_RB), "Run_RB", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::RUN_RF), "Run_RF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::RUN_BASEPOSE), "Run_BasePose", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::RUN_POSE_F), "Run_Pose_F", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::RUN_POSE_L), "Run_Pose_L", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::RUN_POSE_R), "Run_Pose_R", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::RUN_TURNBACK), "Run_Turnback", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::SPRINT_F), "Sprint_F", 1.35f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::STOP_RUN_L), "Stop_Run_L", 1.f, 0.f); // 왼발로 멈추기.
    CState::Add_Animations(ENUM_CLASS(ERoverRunType::STOP_SPRINT_L), "Stop_Sprint_L", 1.f, 0.f); // 왼발로 멈추기
}

void CRoverGroundRun::State_Reset()
{
    for (_uint i = 0; i < RUNSTATE::END; ++i)
    {
        m_States[i] = false;
    }
}



CRoverGroundRun* CRoverGroundRun::Create(class CGameObject* pOwner)
{
    CRoverGroundRun* pInstance = new CRoverGroundRun();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverGroundRun");
        return nullptr;
    }

    return pInstance;
}

void CRoverGroundRun::Free()
{
    CGroundState::Free();
}
