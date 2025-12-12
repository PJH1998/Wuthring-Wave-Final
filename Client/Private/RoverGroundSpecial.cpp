#include "ClientPch.h"
#include "RoverGroundSpecial.h"
#include "Rover.h"
#include "StateMachine.h"

// OMNI 상태에서만 탈출 가능.
HRESULT CRoverGroundSpecial::Initialize(CCharacter* pCharacter)
{
    if (FAILED(__super::Initialize(pCharacter)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pCharacter);
    ASSERT_CRASH(m_pRover);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    return S_OK;
}

void CRoverGroundSpecial::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverSpecialType eSpecialType = context.m_eSpecialType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eSpecialType);

    // 4. 상태 초기화
    State_Reset();

    // 5. 애니메이션 타입에 맞는 파츠 설정. => 0, 1 SWORD / 2, 3 DARKSCYTHE

	switch(eSpecialType)
	{ 
	case ERoverSpecialType::EX_ATTACK01:
		m_iPartType = CRover::PARTTYPE::PART_SWORD;
		m_pRover->Clear_PartAnimation(m_iPartType, m_Animations.at(m_iCurrentAnimIdx).strAnimName);
		m_pRover->Rotate_Target();
		break;
	case ERoverSpecialType::EX_ATTACK02:
		m_iPartType = CRover::PARTTYPE::PART_SWORD;
		m_pRover->Clear_PartAnimation(m_iPartType, m_Animations.at(m_iCurrentAnimIdx).strAnimName);
		m_pRover->Rotate_Target();
		break;
	case ERoverSpecialType::EX_ATTACK03:
		m_iPartType = CRover::PARTTYPE::PART_DARKSCYTHE;
		m_pRover->Clear_PartAnimation(m_iPartType, m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName));
		m_pRover->Rotate_Target();
		break;
	case ERoverSpecialType::EX_ATTACK04:
		m_iPartType = CRover::PARTTYPE::PART_DARKSCYTHE;
		m_pRover->Clear_PartAnimation(m_iPartType, m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName));
		m_pRover->Rotate_Target();
		break;
	case ERoverSpecialType::EX_ATTACK05:
		m_iPartType = CRover::PARTTYPE::PART_DARKSCYTHE;
		m_pRover->Clear_PartAnimation(m_iPartType, m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName));
		m_pRover->Rotate_Target();
		break;
	}

    m_pRover->PartActivate(m_iPartType, true);
    

	//if (eSpecialType == ERoverSpecialType::EX_ATTACK05)
	//	m_pRover->PartActivate(m_iPartType, false);
    m_pRover->Set_Gravity(true);
	

	m_strSkillName = m_Animations.at(m_iCurrentAnimIdx).strAnimName; // 진입할때 한번 현재 스킬이름 저장.
}

void CRoverGroundSpecial::OnUpdate(_float fTimeDelta)
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

void CRoverGroundSpecial::OnExit()
{
    CGroundState::OnExit();
	m_iComboCount = 0;
    m_pRover->PartActivate(m_iPartType, false);

	// DarkSythe의 경우 애니메이션 Clear
	if (m_iPartType == CRover::PARTTYPE::PART_DARKSCYTHE)
	{
		_string strPartAnimName = m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName);
		m_pRover->Clear_PartAnimation(m_iPartType, strPartAnimName);
	}
	
    m_pRover->Set_Gravity(true); 
	m_pRover->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::HIT));
	// 공격 콜라이더 비활성화
	m_pRover->Collider_Active(TEXT("Main|X|X"), false);

}

void CRoverGroundSpecial::Handle_Input()
{
    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
    m_States[DASH] = m_States[MOVE] && m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
    m_States[ATTACK] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

	m_States[SKILL_E] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));
	m_States[SKILL_R] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::R));

	m_States[BURST] = m_pRover->Check_AnyConidtion_FromAbility(ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE));

	if (m_States[BURST])
		m_States[BURST_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pRover->Check_Skill("Ex_Skill02"));
	else
		m_States[DEFAULT_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pRover->Check_Skill("Skill02"));
	m_States[ULTI] = m_States[SKILL_R] && (m_pRover->Get_Cost(COST_TYPE::COST5) >= m_pRover->Get_MaxCost());

	m_States[EXIT] = m_pRover->Get_Cost(COST_TYPE::COST1) <= 0.f;
}

void CRoverGroundSpecial::Update_SkillAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pRover->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; // 거리 계산에 따른 Animation Scale 조절.
	
	// 1. 애니메이션 실행
    CCharacterState::Play_Animation(m_pRover, fTimeDelta, m_fAnimationScale);

    // 2. 파츠 실행.

	if (m_iPartType == CRover::PARTTYPE::PART_DARKSCYTHE)
	{
		// 3. 자동으로 Animation 종료시 Activate 종료.
		m_pRover->Play_PartAnimation(
			m_iPartType,
			m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName),
			(m_Animations.at(m_iCurrentAnimIdx).fSpeed)* fTimeDelta, nullptr
		);
	}
	else
	{
		m_pRover->Play_PartAnimation(
			m_iPartType,
			m_Animations.at(m_iCurrentAnimIdx).strAnimName,
			m_Animations.at(m_iCurrentAnimIdx).fSpeed * fTimeDelta, nullptr
		);
	}
	
}

void CRoverGroundSpecial::Check_Physcis(_float fTimeDelta)
{
    m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
}

void CRoverGroundSpecial::Check_StateTransition(_float fTimeDelta)
{
    ERoverSpecialType eSpType = static_cast<ERoverSpecialType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

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

	// 1. 탈출 가능한 시점에서
	if (IsEscapePossible)
	{
		// 가장 우선순위 높은 상황.
		if (m_States[ATTACK] && !m_States[EXIT])
		{
			switch (eSpType)
			{
				case ERoverSpecialType::EX_ATTACK01:
				{
					m_pRover->GetStateContextForWrite().m_eSpecialType = ERoverSpecialType::EX_ATTACK02;
					m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::SPECIAL));
					return;
				}
				case ERoverSpecialType::EX_ATTACK02:
				{
					m_pRover->GetStateContextForWrite().m_eSpecialType = ERoverSpecialType::EX_ATTACK03;
					m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::SPECIAL));
					return;
				}
				case ERoverSpecialType::EX_ATTACK03:
				{
					m_pRover->GetStateContextForWrite().m_eSpecialType = ERoverSpecialType::EX_ATTACK04;
					m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::SPECIAL));
					return;
				}
				case ERoverSpecialType::EX_ATTACK04:
				{
					m_pRover->GetStateContextForWrite().m_eSpecialType = ERoverSpecialType::EX_ATTACK05;
					m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::SPECIAL));
					return;
				}
			}
		}
		
		if (m_States[MOVE])
		{
			m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN)); // 상위, 하위 상태
			return;
		}

		if (m_States[JUMP])
		{
			m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_WALK_LF;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP)); // 상위, 하위 상태
			return;
		}
		
	}

	if (m_IsAnimationEnd)
	{
		if (m_States[LAND])
		{
			m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
			return;
		}
		else if (!m_States[LAND])
		{
			m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL));
			return;
		}
	}
    
}

void CRoverGroundSpecial::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERoverSpecialType::EX_ATTACK01), "Ex_Attack01", 1.4f, 20.f);
    CState::Add_Animations(ENUM_CLASS(ERoverSpecialType::EX_ATTACK02), "Ex_Attack02", 1.4f, 20.f);
    CState::Add_Animations(ENUM_CLASS(ERoverSpecialType::EX_ATTACK03), "Ex_Attack03", 1.4f, 26.f);
    CState::Add_Animations(ENUM_CLASS(ERoverSpecialType::EX_ATTACK04), "Ex_Attack04", 1.4f, 35.f);
	CState::Add_Animations(ENUM_CLASS(ERoverSpecialType::EX_ATTACK05), "Ex_Attack05", 1.4f, 80.f);
    


	m_PartsAnimations.emplace("Ex_Attack03", "Scythe_Ex_Attack03");
	m_PartsAnimations.emplace("Ex_Attack04", "Scythe_Ex_Attack04");
	m_PartsAnimations.emplace("Ex_Attack05", "Scythe_Ex_Attack04");
}

void CRoverGroundSpecial::State_Reset()
{
    for (_uint i = 0; i < SPEICALSTATE::END; ++i)
        m_States[i] = false;
}


CRoverGroundSpecial* CRoverGroundSpecial::Create(CCharacter* pOwner)
{
    CRoverGroundSpecial* pInstance = new CRoverGroundSpecial();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverGroundSpecial");
    }

    return pInstance;
}

void CRoverGroundSpecial::Free()
{
    CGroundState::Free();
}
