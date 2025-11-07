#include "ClientPch.h"
#include "AugustaGroundRun.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaGroundRun::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaGroundRun::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaRunType eRunType = context.m_eRunType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eRunType);

    // 4. 현재 상태 초기화
    State_Reset();

	// 5. 중력 켰다.
    m_pAugusta->Set_Gravity(true);
}

void CAugustaGroundRun::OnUpdate(_float fTimeDelta)
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

void CAugustaGroundRun::OnExit()
{
    CGroundState::OnExit();
    m_pAugusta->Set_Gravity(true);

	m_iNotLandFrames = 0;
	m_fFallTime = 0.f;
}

void CAugustaGroundRun::Handle_Input()
{
    // 1. 방향 계산
    m_eDir = m_pAugusta->Calculate_Direction();

	m_States[HIT] = m_pAugusta->Is_Hit(); // HIT 상태인가?
	if (m_States[HIT]) // 모든 조건 상위 조건
		return;
	// 우선순위 제일 높음.
	m_States[FLY] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::T));

    // 키 입력.
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey); // WASD 키입력 체크.
    m_States[DASH] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));

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
    m_States[SPRINT_F] = m_States[MOVE] && m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
    
    // 공격 상태가 아니라 공격 판정 상태로 전달.
    m_States[ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

    // 상태에 따라 속도 다르게.
    m_fSpeed = m_States[SPRINT_F] ? 1.2f : 0.7f;
}





void CAugustaGroundRun::Update_RunAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);


    EAugustaRunType eRunType = static_cast<EAugustaRunType>(m_iCurrentAnimIdx);
    // 1. 회전 및 이동.
    if (m_pAugusta->Is_LockOn())
    {
        if (eRunType == EAugustaRunType::SPRINT_F || eRunType == EAugustaRunType::STOP_SPRINT_L)
            m_pAugusta->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, m_fSpeed);
        else 
            // 1. WASD 입력에 따른 8방향 이동
            m_pAugusta->Move_LockOn_8Way(m_eDir, fTimeDelta, m_fSpeed);
    }
    else 
        m_pAugusta->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, m_fSpeed);

}

void CAugustaGroundRun::Check_Physics(_float fTimeDelta)
{
	
    m_States[WALL] = m_pAugusta->Check_ClimbableWall(&m_vWallNormal); // Wall인지?
    // Land Check


	// 1. Jolt의 IsSupported()를 호출하여 땅의 Normal 벡터(m_vLandNormal)를 갱신합니다.
	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);

	_float fLandDistance = 0.5f;


	if (m_States[LAND])
	{
		m_fFallTime = 0.f;
	}
	else if (!m_States[LAND])
	{
		m_fFallTime += fTimeDelta;

		cout << "FallTime : " << m_fFallTime << endl;
		if (m_fFallTime >= 0.2f)
			m_States[FALL] = true;

		//m_States[LAND] = m_pAugusta->Is_Land(0.2f, fLandDistance);
	}
	

	//// 2. 기본 LandDistance 설정
	//_float fLandDistance = 0.5f;

	//// 3. 땅의 경사도(m_vLandNormal.y)를 확인합니다.
	//// m_vLandNormal.y가 1.0(평지)보다 작고 0.3(약 72도)보다 크다면 경사로로 판단.

	//if (m_vLandNormal.y < 0.98f && m_vLandNormal.y > 0.3f)
	//{
	//	// 경사로에서는 Ray 판정 거리를 1.0f (혹은 1.2f) 정도로 늘려서
	//	// 빠르게 내려가도 땅으로 인식되도록 합니다.
	//	fLandDistance = 1.3f;
	//}

	//// 4. 동적으로 조절된 fLandDistance 값으로 RayCast Land 체크를 수행합니다.
	//m_States[LAND] = m_pAugusta->Is_Land(0.2f, fLandDistance);


}


void CAugustaGroundRun::Check_StateTransition(_float fTimeDelta)
{
 
    EAugustaRunType eRunType = static_cast<EAugustaRunType>(m_iCurrentAnimIdx);
    _float3 vNormal = {}; // 벽타기 전환 용도 Normal
    // 이 조건은 추후 디테일 잡아보기.

    //// 전방 벽감지.
    //if (m_States[RUN_U] && m_States[WALL])
    //{
    //    m_pAugusta->GetStateContextForWrite().m_eClimbMoveType = EAugustaClimbMoveType::CLIMB_U_1;
    //    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_MOVE)); // 상위, 하위 상태
    //    return;
    //}
    
    // Land 판정이 아니면서 Ray 반사 길이가 0.2f 이상이면?
    //if (!m_States[LAND] && fDistanceToGround > 0.3f)

    if (!m_States[LAND])
    {
		m_iNotLandFrames++;
		if (m_iNotLandFrames >= MAX_NOT_LAND_FRAMES)
		{
			m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
			return;
		}
    }

	// 상위, 하위 상태
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

	// Land가 아닌 판정이면 0 초기화.
	m_iNotLandFrames = 0;
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

#ifdef _DEBUG
		m_pAugusta->Print_CoolTime();
#endif // _DEBUG

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

    // Dash 보다 우선순위 높음.
    if (m_States[SPRINT_F])
    {
        m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRunType::SPRINT_F);
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
                    m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRunType::RUN_LF);
                else if (m_States[RUN_R])
                    m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRunType::RUN_RF);
                else
                    m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRunType::RUN_F);
            }
            else if (m_States[RUN_D])
            {
                if (m_States[RUN_L])
                    m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRunType::RUN_LB);
                else if (m_States[RUN_R])
                    m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRunType::RUN_RB);
                else
                    m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRunType::RUN_B);
            }
            else if (m_States[RUN_L])
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRunType::RUN_LF);
            else if (m_States[RUN_R])
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRunType::RUN_RF);

            return;
        }
        // 이동 값이 들어왔는데 Stop Run 상태라면?
        if (eRunType == EAugustaRunType::STOP_RUN_L || eRunType == EAugustaRunType::SPRINT_F)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRunType::RUN_F);
            return;
        }
    }

    // 이동 입력 값이 안들어왔다면?
    if (!m_States[MOVE])
    {
        // 현재 상태가 Sprint 였다면?
        if (eRunType == EAugustaRunType::SPRINT_F)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRunType::STOP_SPRINT_L);
            return;
        }

        // 현재 상태가 STOP_RUN이 아니라면? => STOP RUN
        if (eRunType != EAugustaRunType::STOP_RUN_L)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(EAugustaRunType::STOP_RUN_L);
            return;
        }
        // Stop Run 이면서 애니메이션 재생이 끝났다면?.
        if ((eRunType == EAugustaRunType::STOP_RUN_L || eRunType == EAugustaRunType::STOP_SPRINT_L) && m_IsAnimationEnd)
        {
            m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
            return;
        }
    }

    
}



void CAugustaGroundRun::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::RUN_B), "Run_B", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::RUN_F), "Run_F", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::RUN_LB), "Run_LB", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::RUN_LF), "Run_LF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::RUN_RB), "Run_RB", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::RUN_RF), "Run_RF", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::RUN_BASEPOSE), "Run_BasePose", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::RUN_POSE_F), "Run_Pose_F", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::RUN_POSE_L), "Run_Pose_L", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::RUN_POSE_R), "Run_Pose_R", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::RUN_TURNBACK), "Run_Turnback", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::SPRINT_F), "Sprint_F", 1.35f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::STOP_RUN_L), "Stop_Run_L", 1.f, 0.f); // 왼발로 멈추기.
    CState::Add_Animations(ENUM_CLASS(EAugustaRunType::STOP_SPRINT_L), "Stop_Sprint_L", 1.f, 0.f); // 왼발로 멈추기
}

void CAugustaGroundRun::State_Reset()
{
    for (_uint i = 0; i < RUNSTATE::END; ++i)
    {
        m_States[i] = false;
    }
}



CAugustaGroundRun* CAugustaGroundRun::Create(class CGameObject* pOwner)
{
    CAugustaGroundRun* pInstance = new CAugustaGroundRun();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundRun");
        return nullptr;
    }

    return pInstance;
}

void CAugustaGroundRun::Free()
{
    CGroundState::Free();
}
