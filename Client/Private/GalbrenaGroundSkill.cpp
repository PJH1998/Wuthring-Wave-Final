#include "ClientPch.h"
#include "GalbrenaGroundSkill.h"
#include "Galbrena.h"
#include "StateMachine.h"

HRESULT CGalbrenaGroundSkill::Initialize(class CGameObject* pOwner)
{
    if (FAILED(__super::Initialize(pOwner)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pOwner);
    ASSERT_CRASH(m_pGalbrena);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    return S_OK;
}

void CGalbrenaGroundSkill::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaSkillType eSkillType = context.m_eSkillType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eSkillType);

    // 4. 상태 초기화
    State_Reset();

	m_pGalbrena->Set_Gravity(true);

    // 5. 애니메이션 타입에 맞는 파츠 설정.
    switch(eSkillType)
    {
        case EGalbrenaSkillType::ATTACK_JUMP_START: // 기본 E
		{
			m_pGalbrena->Rotate_Target(); // 진입 시 한번만
			m_pGalbrena->Set_Gravity(false);
			break;
		}

		case EGalbrenaSkillType::ATTACK_JUMP_END02: // 기본 E
		{
			m_pGalbrena->Set_Gravity(false);
			break;
		}
    }

	
	m_strSkillName = m_Animations.at(m_iCurrentAnimIdx).strAnimName;

	// 6. 무적 상태 부여
	m_pGalbrena->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
}

void CGalbrenaGroundSkill::OnUpdate(_float fTimeDelta)
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

void CGalbrenaGroundSkill::OnExit()
{
	CGroundState::OnExit();
    m_pGalbrena->Set_Gravity(true);


	if (m_iPartType != CGalbrena::PARTTYPE::TYPE_END)
	{
		m_pGalbrena->PartActivate(m_iPartType, false);
	}
    m_iPartType = CGalbrena::PARTTYPE::TYPE_END;

	// 기본 E 스킬에 적중 시 반동 E 스킬 발동을 위한 Condition
	m_pGalbrena->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::SKILLHIT));

	m_pGalbrena->Collider_Active(TEXT("Galbrena|DEFAULT_E|SKILL"), false);

	m_pGalbrena->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
}

void CGalbrenaGroundSkill::Handle_Input()
{
    m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
    m_States[SKILL_E] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));

}

void CGalbrenaGroundSkill::Update_SkillAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선) // 거리 계산에 따른 Animation Scale 조절.
	m_fRootMotionScale = m_pGalbrena->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; 

	EGalbrenaSkillType eSkillType = static_cast<EGalbrenaSkillType>(m_iCurrentAnimIdx);

	Handle_Animation_SpecialState();

    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta, m_fAnimationScale);

    // Target이 존재한다면? => Auto Target
    
	//if (m_iPartType == CGalbrena::PARTTYPE::PART_DARKSCYTHE || CGalbrena::PART_DARKWING)
    //{
    //    m_pGalbrena->Play_PartAnimation(
    //        m_iPartType,
    //        m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName),
    //        fTimeDelta * m_Animations.at(m_iCurrentAnimIdx).fSpeed, nullptr, 1.f, true, false
    //    );
    //}
}

void CGalbrenaGroundSkill::Check_Physcis(_float fTimeDelta)
{
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
}

void CGalbrenaGroundSkill::Check_StateTransition(_float fTimeDelta)
{
    EGalbrenaSkillType eSkillType = static_cast<EGalbrenaSkillType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

	
	// 1. Default E스킬 사용했을 때?
	if (eSkillType == EGalbrenaSkillType::ATTACK_JUMP_START)
	{
		// 2. Hit 판정이 있었다면?
		if (m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::SKILLHIT)))
		{
			m_pGalbrena->GetStateContextForWrite().m_eSkillType = EGalbrenaSkillType::ATTACK_JUMP_END02;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::SKILL));
			return;
		}
	}


    if (IsEscapePossible)
    {
		if (m_States[JUMP])
		{
			m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
			return;
		}

		if (m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
				return;
			}
		}
    }


    // 가장 우선순위 낮음.
    if (m_IsAnimationEnd)
    {
        if (m_States[LAND])
        {
            m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND1;
            m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
            return;
        }

        if (!m_States[LAND])
        {
            m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
            m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL));
            return;
        }

		
    }
    
}
void CGalbrenaGroundSkill::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaSkillType::SKILL01), "Skill01", 1.f, 20.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaSkillType::SKILL02), "Skill02", 1.f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaSkillType::ATTACK_JUMP_START), "Attack_Jump_Start", 1.f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaSkillType::ATTACK_JUMP), "Attack_Jump", 1.f, 20.f);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaSkillType::ATTACK_JUMP_END02), "Attack_Jump_End02", 1.3f, 20.f);

    //m_PartsAnimations.emplace("Ex_Skill02", "Scythe_Ex_Attack03");
    //m_PartsAnimations.emplace("Skill02", "G_Skill02");


}

void CGalbrenaGroundSkill::State_Reset()
{
    for (_uint i = 0; i < SKILLSTATE::END; ++i)
        m_States[i] = false;
}

void CGalbrenaGroundSkill::Handle_Animation_SpecialState()
{
	EGalbrenaSkillType eSkillType = static_cast<EGalbrenaSkillType>(m_iCurrentAnimIdx);

	// 해당 동작은 온전한 이동량 보장.
	if (eSkillType == EGalbrenaSkillType::ATTACK_JUMP_END02)
		m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate;
}



CGalbrenaGroundSkill* CGalbrenaGroundSkill::Create(class CGameObject* pOwner)
{
    CGalbrenaGroundSkill* pInstance = new CGalbrenaGroundSkill();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaGroundSkill");
    }

    return pInstance;
}

void CGalbrenaGroundSkill::Free()
{
    CGroundState::Free();


}

