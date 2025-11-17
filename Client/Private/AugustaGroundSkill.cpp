#include "ClientPch.h"
#include "AugustaGroundSkill.h"
#include "Augusta.h"
#include "StateMachine.h"

HRESULT CAugustaGroundSkill::Initialize(class CGameObject* pOwner)
{
    if (FAILED(__super::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    

    return S_OK;
}

void CAugustaGroundSkill::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaSkillType eSkillType = context.m_eSkillType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eSkillType);

    // 4. 상태 초기화
    State_Reset();

    // 5. 애니메이션 타입에 맞는 파츠 설정.
    switch(eSkillType)
    {
        case EAugustaSkillType::SKILL_STRIKE:
        {
            //_string strBoneName = "WeaponProp06";
            _string strBoneName = "Root";
            m_iPartType = CAugusta::PARTTYPE::PART_GRIFFON;
            m_pAugusta->PartActivate(m_iPartType, true);
            m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
            m_pAugusta->Set_Gravity(false);
            m_pAugusta->Clear_PartAnimation(m_iPartType, m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName));
            
			// 진입할때 한번만 회전 => Griffon
			m_pAugusta->Rotate_Target();

			// 진입했을 때 Condition E_RISE로 변환? => E_GRIFFON도 활성화.
			m_pAugusta->Bind_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_GRIFFON));
			m_pAugusta->Bind_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_RISE));

			// 무적.
			m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
            break;
        }
		case EAugustaSkillType::SKILL_RISE_ZERO:
		{
			m_pAugusta->Set_Gravity(false);
			m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
			break;
		}
        case EAugustaSkillType::SKILL_RISE:
        {
            m_pAugusta->Set_Gravity(false);
			m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
            break;
        }

		case EAugustaSkillType::SKILL_HACK:
		{
			m_iPartType = CAugusta::PARTTYPE::PART_BAYONET; // 추후 애니메이션에 따른. 분기문 필요.

			_string strBoneName = "WeaponProp05";
			m_pAugusta->PartActivate(m_iPartType, true);
			m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
			m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
			m_pAugusta->Set_Gravity(false);
			// 진입할때 한번만.
			m_pAugusta->Rotate_Target();

			break;
		}

		// 스킬. R
		case EAugustaSkillType::ATTACK_SPEEDDRIVE:
		{
			_string strBoneName = "WeaponProp02";
			m_iPartType = CAugusta::PARTTYPE::PART_SKILLWEAPON; // 추후 애니메이션에 따른. 분기문 필요.
			m_pAugusta->PartActivate(m_iPartType, true);
			m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
			m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
			m_pAugusta->Set_Gravity(true);
			// 진입할때 한번만.
			m_pAugusta->Rotate_Target();
			m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
			break;
		}
		case EAugustaSkillType::ATTACK_PULL:
		{
			_string strBoneName = "WeaponProp02";
			m_iPartType = CAugusta::PARTTYPE::PART_SKILLWEAPON; // 추후 애니메이션에 따른. 분기문 필요.
			m_pAugusta->PartActivate(m_iPartType, true);
			m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
			m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
			m_pAugusta->Set_Gravity(true);
			// 진입할때 한번만.
			m_pAugusta->Rotate_Target();
			m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
			break;
		}

		case EAugustaSkillType::ATTACK_SPSKILL:
		{
			_string strBoneName = "WeaponProp05"; 
			m_iPartType = CAugusta::PARTTYPE::PART_SKILLWEAPON; 
			m_pAugusta->PartActivate(m_iPartType, true);
			m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
			m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
			m_pAugusta->Set_Gravity(true);
			// 진입할때 한번만.
			m_pAugusta->Rotate_Target();
			m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
			break;
		}
        
    }


	m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
	m_strSkillName = m_Animations[m_iCurrentAnimIdx].strAnimName;
}

void CAugustaGroundSkill::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 1. 입력키 받기.
    Handle_Input();

    // 2. Skill 업데이트
    Update_SkillAnimations(fTimeDelta);

    // 3.  물리 체크
    Check_Physcis(fTimeDelta);

    // 4. 전환 조건 체크
    Check_StateTransition(fTimeDelta);

    // 5. 상태 리셋.
    State_Reset();
    
}

void CAugustaGroundSkill::OnExit()
{
	CGroundState::OnExit();


	if (m_iPartType != CAugusta::PARTTYPE::TYPE_END)
	{
		m_pAugusta->PartActivate(m_iPartType, false);
	}
	
	if (m_iSubPartType != CAugusta::PARTTYPE::TYPE_END)
	{
		m_pAugusta->PartActivate(m_iSubPartType, false);
	}
    
    m_pAugusta->Set_Gravity(true);
    

    m_iPartType = CAugusta::PARTTYPE::TYPE_END;

	/* 컨디션 제거*/
	m_pAugusta->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));

	// Collider 제거.
	m_pAugusta->Collider_Active(TEXT("Main|X|X"), false);
}

void CAugustaGroundSkill::Handle_Input()
{
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[SKILL_E] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));
    m_States[SKILL_R] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::R));

    //m_States[POINT_E] = m_States[SKILL_E] && 
	//	(SKILL_STATE::READY == m_pAugusta->Check_Skill(m_strSkilName));
	//
	//
	//EAugustaSkillType eSkillType = static_cast<EAugustaSkillType>(m_iCurrentAnimIdx);
    //m_States[SWORD_R] = m_States[SKILL_R] && (SKILL_STATE::READY == m_pAugusta->Check_Skill(m_strSkilName));
}

void CAugustaGroundSkill::Update_SkillAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선) // 거리 계산에 따른 Animation Scale 조절.
	m_fRootMotionScale = m_pAugusta->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations[m_iCurrentAnimIdx].fRootMotionRate * m_fRootMotionScale; 

	EAugustaSkillType eSkillType = static_cast<EAugustaSkillType>(m_iCurrentAnimIdx);

	Handle_Animation_SpecialState();


    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta, m_fAnimationScale);

    // Target이 존재한다면? => Auto Target
    if (m_iPartType == CAugusta::PARTTYPE::PART_GRIFFON)
    {
        m_pAugusta->Play_PartAnimation(
            m_iPartType,
            m_PartsAnimations[m_Animations[m_iCurrentAnimIdx].strAnimName],
            fTimeDelta * m_Animations[m_iCurrentAnimIdx].fSpeed, nullptr, 1.f, true, false
        );
    }

	if (m_iPartType == CAugusta::PARTTYPE::PART_BAYONET)
	{
		m_pAugusta->Play_PartAnimation(
			m_iPartType,
			m_Animations[m_iCurrentAnimIdx].strAnimName,
			fTimeDelta * m_Animations[m_iCurrentAnimIdx].fSpeed, nullptr, 1.f, true, false
		);
	}
     
}

void CAugustaGroundSkill::Check_Physcis(_float fTimeDelta)
{
	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);
}

void CAugustaGroundSkill::Check_StateTransition(_float fTimeDelta)
{
    EAugustaSkillType eSkillType = static_cast<EAugustaSkillType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

    if (IsEscapePossible)
    {
		// 1. Griffon => Rise Zero => Rise 
		// => AIRATTACK_HACKDOWN_START => AIRATTACK_HACKDOWN_SP_END

		if (m_States[SKILL_E])
		{
			m_States[SKILL_RISE_ZERO] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pAugusta->Check_Skill("Skill_Rise_Zero", m_strSkillName));
			m_States[SKILL_RISE] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pAugusta->Check_Skill("Skill_Rise", m_strSkillName));
			m_States[AIRATTACK_HACKDOWN_START] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pAugusta->Check_Skill("AirAttack_HackDown_Start", m_strSkillName));

			//if (eSkillType == EAugustaSkillType::SKILL_STRIKE)
			if (m_States[SKILL_RISE_ZERO])
			{
				if (SKILL_STATE::READY != m_pAugusta->Use_Skill("Skill_Rise_Zero"))
					return;

				m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::SKILL_RISE_ZERO;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL));
				return;
			}

			// 1. Skill Rise Zero
			if (eSkillType == EAugustaSkillType::SKILL_RISE_ZERO)
			{
				if (SKILL_STATE::READY != m_pAugusta->Use_Skill("Skill_Rise"))
					return;

				m_pAugusta->Remove_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_RISE));
				m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::SKILL_RISE;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL));
				return;
			}

			// 2. Skill Rise
			if (m_States[AIRATTACK_HACKDOWN_START])
			{
				if (SKILL_STATE::READY != m_pAugusta->Use_Skill("AirAttack_HackDown_Start"))
					return;

				m_pAugusta->Remove_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_GRIFFON));
				m_pAugusta->GetStateContextForWrite().m_eAirAttackType = EAugustaAirAttackType::AIRATTACK_HACKDOWN_START;
				m_pAugusta->GetStateContextForWrite().m_strPrevInfo = "Griffon";
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::AIR_ATTACK));
				return;
			}
		}
		

		if (m_States[SKILL_R])
		{
			// 이전 이름에 현재 Skill 이름이 들어감.
			m_States[ATTACK_PULL] = eSkillType == EAugustaSkillType::ATTACK_SPEEDDRIVE;
			m_States[ATTACK_SP_SKILL] = eSkillType == EAugustaSkillType::ATTACK_PULL;

			if (m_States[ATTACK_PULL])
			{
				m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::ATTACK_PULL;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL));
				return;
			}

			if (m_States[ATTACK_SP_SKILL])
			{
				m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::ATTACK_SPSKILL;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL));
				return;
			}
			
		}

	
		// UNIQUE_R이 마지막 스킬일때?
		if (eSkillType == EAugustaSkillType::ATTACK_SPSKILL)
		{
			if (m_States[MOVE])
			{
				m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
				return;
			}
		}

        // 더블 점프 형태로만 변경 가능. => 도중에 바꿨을때는 E스킬 사용을 초기화
        if (m_States[JUMP])
        {
			m_pAugusta->Remove_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_RISE));
			m_pAugusta->Remove_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_GRIFFON));

            m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
            return;
        }

        // 땅에 닿으면. 우선 순위 => 도중에 바꿨을때는 E스킬 사용을 초기화
		if (m_States[LAND])
		{
			m_pAugusta->Remove_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_RISE));
			m_pAugusta->Remove_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_GRIFFON));
			if (m_States[MOVE])
			{
				m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
				return;
			}
		}
    }


    // 가장 우선순위 낮음.
    if (m_IsAnimationEnd)
    {
		// 아무것도 안했다면? => 컨디션 제거.
		m_pAugusta->Remove_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_RISE));
		m_pAugusta->Remove_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_GRIFFON));

		if (eSkillType == EAugustaSkillType::ATTACK_SPSKILL)
		{
			if (m_States[MOVE])
			{
				m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
				return;
			}
			else
			{
				m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STANDCHANGE;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
				return;
			}
		}

        if (m_States[LAND])
        {
            m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STANDCHANGE;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
            return;
        }

        if (!m_States[LAND])
        {
            m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL));
            return;
        }

		

		
    }
    
}
void CAugustaGroundSkill::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::SKILL_HACK), "Skill_Hack", 1.f, 45.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::SKILL_STRIKE), "Skill_Strike", 1.f, 30.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::SKILL_RISE_ZERO), "Skill_Rise_Zero", 1.2f, 15.f, 1.2f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::SKILL_RISE), "Skill_Rise", 1.2f, 25.f, 1.2f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::SKILLQTE), "SkillQTE", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::ATTACK_SPEEDDRIVE), "Attack_SpeedDrive", 1.f, 30.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::ATTACK_PULL), "Attack_Pull", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::ATTACK_SPSKILL), "Attack_SpSkill", 1.f, 60.f);

    m_PartsAnimations.emplace("Skill_Strike", "SA1Shouwangjiu_Skill_Strike");
    //SA1Shouwangjiu_AirAttack_End
    //SA1Shouwangjiu_AirAttack_Loop
    //SA1Shouwangjiu_AirAttack_Start
    //SA1Shouwangjiu_Fly_Loop
    //SA1Shouwangjiu_Skill_Strike

}

void CAugustaGroundSkill::State_Reset()
{
    for (_uint i = 0; i < SKILLSTATE::END; ++i)
        m_States[i] = false;
}

void CAugustaGroundSkill::Handle_Animation_SpecialState()
{
	EAugustaSkillType eSkillType = static_cast<EAugustaSkillType>(m_iCurrentAnimIdx);


	if (eSkillType == EAugustaSkillType::ATTACK_PULL || 
		eSkillType == EAugustaSkillType::SKILL_RISE_ZERO || 
		eSkillType == EAugustaSkillType::SKILL_RISE) // 1. 뒤로 이동은 온전하게 이동거리받기.
		m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate;

	
}


CAugustaGroundSkill* CAugustaGroundSkill::Create(class CGameObject* pOwner)
{
    CAugustaGroundSkill* pInstance = new CAugustaGroundSkill();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundSkill");
    }

    return pInstance;
}

void CAugustaGroundSkill::Free()
{
    CGroundState::Free();


}

