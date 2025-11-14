#include "ClientPch.h"
#include "AugustaGroundIdle.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaGroundIdle::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    // Idle 애니메이션 리스트 셋업
    Setup_Animations();

    // 기본 애니메이션 셋업.
    m_iCurrentAnimIdx = 0;

    // 바꿀 파트타입?
    
    return S_OK;
}

void CAugustaGroundIdle::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaIdleType eIdleType = context.m_eIdleType; // 애니메이션 ENUM

    m_iCurrentAnimIdx = ENUM_CLASS(eIdleType);

    m_iPartType = CAugusta::PARTTYPE::PART_BAYONET;

    if (eIdleType == EAugustaIdleType::STAND1_ACTION01 || eIdleType == EAugustaIdleType::STAND1_ACTION02
        || eIdleType == EAugustaIdleType::STAND2)
    {
        _string strBoneName = "Root";
        m_pAugusta->PartActivate(m_iPartType, true);
		m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
        m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
    }

	if (eIdleType == EAugustaIdleType::STANDCHANGE)
	{
		// 한바퀴 돌려준다.
		/*_vector vLook = m_pAugusta->Get_LookVector_NoPitch();
		m_pAugusta->Rotate_Direction(vLook * -1.f);*/

		//_vector vLook = m_pAugusta->Get_LookVector() * -1.f;
		//m_pAugusta->Set_LookVector(vLook);

		_string strBoneName = "WeaponProp05";
		m_pAugusta->PartActivate(m_iPartType, true);
		m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
		m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
	}

    // 3. Idle 상태 초기화
    State_Reset();

	// 4. Attack Volume 끄기


	m_pAugusta->Set_Gravity(true);
}

void CAugustaGroundIdle::OnUpdate(_float fTimeDelta)
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

void CAugustaGroundIdle::OnExit()
{
    CGroundState::OnExit();
    m_pAugusta->PartActivate(m_iPartType, false);
	m_fFallTime = 0.f;
}

void CAugustaGroundIdle::Handle_Input()
{
	// Dash 키입력 체크.
	m_States[DASH] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));

	m_States[HIT] = m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT)); // HIT 상태인가?
	m_States[DODGEABLE] = m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));

	m_States[DODGE] = m_States[DODGEABLE] && m_States[DASH]; // Dodge 가능하면서 Dash 키 누르면?

	if (m_States[HIT] || m_States[DODGE])
		return;
	m_States[FLY] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::T)); // 최우선 순위

    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);

    m_States[SPRINT] = m_States[MOVE] && m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));

    // AttackState에서 판별.
    m_States[ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

    // LockOn인 경우에는 W, A, S, D 입력값을 모두 판별.
    m_States[MOVE_U] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
    m_States[MOVE_D] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
    m_States[MOVE_L] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
    m_States[MOVE_R] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));
    
    // 기본 Skill E
    m_States[SKILL_E] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));
    m_States[SKILL_Q] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::Q));
    m_States[SKILL_R] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::R));

	// Ability System.
	m_States[NORMAL_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pAugusta->Check_Skill("Skill_Hack")); // Skill Hack
	m_States[SWORD_R] = m_States[SKILL_R] && (SKILL_STATE::READY == m_pAugusta->Check_Skill("Burst01")); // BurstR
	m_States[ECHO_R] = m_States[SKILL_R] && (SKILL_STATE::READY == m_pAugusta->Check_Skill("Attack_SpeedDrive")); // 궁극기.
	m_States[POINT_E] = m_States[SKILL_E] && (SKILL_STATE::READY ==  m_pAugusta->Check_Skill("Skill_Strike"));// 그리폰
	
}


// Idle 간의 전환 지정.
void CAugustaGroundIdle::Update_IdleAnimations(_float fTimeDelta)
{
    // 1. 현재 애니메이션 재생
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    EAugustaIdleType eIdleType = static_cast<EAugustaIdleType>(m_iCurrentAnimIdx);

    // 2. 파츠도 재생.
    if (eIdleType == EAugustaIdleType::STAND1_ACTION01 || eIdleType == EAugustaIdleType::STAND1_ACTION02
        || eIdleType == EAugustaIdleType::STAND2 || eIdleType == EAugustaIdleType::STANDCHANGE)
    {
		m_pAugusta->Play_PartAnimation(
			m_iPartType,
			m_Animations.at(m_iCurrentAnimIdx).strAnimName,
			m_Animations.at(m_iCurrentAnimIdx).fSpeed * fTimeDelta, nullptr
		);
    }
    
}

void CAugustaGroundIdle::Check_Physics(_float fTimeDelta)
{
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

// Idles 조건이 아닌 것들.
void CAugustaGroundIdle::Check_StateTransition(_float fTimeDelta)
{

    EAugustaIdleType eIdleType = static_cast<EAugustaIdleType>(m_iCurrentAnimIdx);

    _uint iKeyInput = {};

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


    // 우선순위 순으로 전환조건 진행.
	if (m_States[FALL])
	{
		m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
		return;
	}

	if (m_States[FLY])
	{
		m_pAugusta->GetStateContextForWrite().m_eAirFlyType = EAugustaAirFlyType::XA_START;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FLY));
		return;
	}

    // 점프
    if (m_States[JUMP])
    {
        m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_WALK_LF;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP)); // 상위, 하위 상태
        return;
    }

    // 아직 미구현. => Burst 게이지 모두 찼을때 궁 누르면 공격기 모션.
    if (m_States[SWORD_R])
    {
		// 위에 서체크하긴 했지만? 다시 체크.
		if (SKILL_STATE::READY != m_pAugusta->Use_Skill("Burst01"))
			return;

		// Bind Condition SP_ATTACK
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

		m_pAugusta->Bind_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_GRIFFON)); // 상태 바인딩.

		m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::SKILL_STRIKE;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL));
		return;
	}
    
	// SKILL _E
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

    // 에코 => 소환 
    if (m_States[SKILL_Q])
    {
       /* m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::SKILL_RISE;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL));*/
        return;
    }


    // 기본 공격
    if (m_States[ATTACK])
    {
        m_pAugusta->GetStateContextForWrite().m_eAttackType = EAugustaAttackType::ATTACK01;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::ATTACK)); // 상위, 하위 상태
        return;
    }

	// 뛰다가 Dash
	if (m_States[DASH])
	{
		if (m_States[MOVE_D])
		{
			m_pAugusta->GetStateContextForWrite().m_eDashType = EAugustaDashType::MOVE_B;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::DASH)); // 상위, 하위 상태
			return;
		}
		else
		{
			m_pAugusta->GetStateContextForWrite().m_eDashType = EAugustaDashType::MOVE_F;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::DASH)); // 상위, 하위 상태
			return;
		}
	}

    // Sprint => 빠르게 달리기.
    if (m_States[SPRINT])
    {
        m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::SPRINT_F;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN)); // 상위, 하위 상태
        return;
    }



    
    // 이동은 Run State에서 조절.
    if (m_States[MOVE])
    {
        // 더 우선순위 높은 것. => Sprint
        // LockOn일때 전환 로직 변경.
        if (m_pAugusta->Is_LockOn())
        {
            if (m_States[MOVE_U])
            {
                if (m_States[MOVE_L])
                    m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_LF; // 애니메이션 상태 => 블랙보드에 기입.        
                else if (m_States[MOVE_R])
                    m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_RF; // 애니메이션 상태 => 블랙보드에 기입.        
                else
                    m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        
            }
            else if (m_States[MOVE_D])
            {
                if (m_States[MOVE_L])
                    m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_LB; // 애니메이션 상태 => 블랙보드에 기입.        
                else if (m_States[MOVE_R])
                    m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_RB; // 애니메이션 상태 => 블랙보드에 기입.        
                else
                    m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_B; // 애니메이션 상태 => 블랙보드에 기입.        
            }
            else if (m_States[MOVE_L])
                m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_LF; // 애니메이션 상태 => 블랙보드에 기입.        
            else if (m_States[MOVE_R])
                m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_RF; // 애니메이션 상태 => 블랙보드에 기입.        
        }
        else
            m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        

        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN)); // 상위, 하위 상태
        return;
    }

    // 자동 변환.
    if (m_IsAnimationEnd)
    {
        EAugustaIdleType nextIdle = EAugustaIdleType::STAND1;

        switch (static_cast<EAugustaIdleType>(m_iCurrentAnimIdx))
        {
        case EAugustaIdleType::STAND1:
            nextIdle = EAugustaIdleType::STAND1_ACTION01;
            break;
        case EAugustaIdleType::STAND1_ACTION01:
            nextIdle = EAugustaIdleType::STAND1_ACTION02;
            break;
        case EAugustaIdleType::STAND1_ACTION02:
            nextIdle = EAugustaIdleType::STAND1_ACTION03;
            break;
        case EAugustaIdleType::STAND1_ACTION03:
            nextIdle = EAugustaIdleType::STAND1_ACTION01;  // 다시 처음으로
            break;
        default:
            nextIdle = EAugustaIdleType::STAND1_ACTION01;
            break;
        }

        m_iCurrentAnimIdx = ENUM_CLASS(nextIdle);
        return;
    }

}



void CAugustaGroundIdle::LockOn_StateTransition(_float fTimeDelta)
{

}

void CAugustaGroundIdle::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaIdleType::STAND1_ACTION01), "Stand1_Action01", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaIdleType::STAND1_ACTION02), "Stand1_Action02", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaIdleType::STAND1_ACTION03), "Stand1_Action03", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaIdleType::STAND1), "Stand1", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaIdleType::STAND2), "Stand2", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaIdleType::STAND_CONTROL), "Stand_Control", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaIdleType::STANDCHANGE), "StandChange", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaIdleType::STANDUP), "StandUp", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaIdleType::STAND1_TURN_L90D), "Stand1_Turn_L90D", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaIdleType::STAND1_TURN_L90D), "Stand1_Turn_R90D", 1.f, 0.f);
}


void CAugustaGroundIdle::State_Reset()
{
    for (_uint i = 0; i < IDLESTATE::END; ++i)
        m_States[i] = false;
}

CAugustaGroundIdle* CAugustaGroundIdle::Create(class CGameObject* pOwner)
{
    CAugustaGroundIdle* pInstance = new CAugustaGroundIdle();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundIdle");
        return nullptr;
    }

    return pInstance;
}

void CAugustaGroundIdle::Free()
{
    CGroundState::Free();
}
