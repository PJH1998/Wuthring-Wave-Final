#include "ClientPch.h"
#include "RoverGroundSkill.h"
#include "Rover.h"
#include "StateMachine.h"

HRESULT CRoverGroundSkill::Initialize(CCharacter* pCharacter)
{
    if (FAILED(__super::Initialize(pCharacter)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pCharacter);
    ASSERT_CRASH(m_pRover);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    

    return S_OK;
}

void CRoverGroundSkill::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverSkillType eSkillType = context.m_eSkillType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eSkillType);

    // 4. 상태 초기화
    State_Reset();

    // 5. 애니메이션 타입에 맞는 파츠 설정.
    switch(eSkillType)
    {
        case ERoverSkillType::SKILL02:
		{
			_string strBoneName = "WingCase";
			m_iPartType = CRover::PARTTYPE::PART_DARKWING;
			m_pRover->PartActivate(m_iPartType, true);
			m_pRover->Set_SocketMatrixToParts(m_iPartType, strBoneName);
			m_pRover->Clear_PartAnimation(m_iPartType, m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName));
			m_pRover->Rotate_Target(); // 진입 시 한번만
			break;
		}
			
		case ERoverSkillType::EX_SKILL02:
		{
			_string strBoneName = "WeaponProp02";
			m_iPartType = CRover::PARTTYPE::PART_DARKSCYTHE;
			m_pRover->PartActivate(m_iPartType, true);
			m_pRover->Set_SocketMatrixToParts(m_iPartType, strBoneName);
			m_pRover->Clear_PartAnimation(m_iPartType, m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName));
			m_pRover->Rotate_Target(); // 진입 시 한번만
			break;
		}
    }

	m_pRover->Set_Gravity(true);
	m_strSkillName = m_Animations.at(m_iCurrentAnimIdx).strAnimName;

	m_pRover->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
}

void CRoverGroundSkill::OnUpdate(_float fTimeDelta)
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

void CRoverGroundSkill::OnExit()
{
	CGroundState::OnExit();
    m_pRover->Set_Gravity(true);


	if (m_iPartType != CRover::PARTTYPE::TYPE_END)
	{
		m_pRover->PartActivate(m_iPartType, false);
	}
    m_iPartType = CRover::PARTTYPE::TYPE_END;

	// 공격 콜라이더 비활성화
	m_pRover->Collider_Active(TEXT("Main|X|X"), false);

	m_pRover->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
}

void CRoverGroundSkill::Handle_Input()
{
    m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
    m_States[SKILL_E] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));

}

void CRoverGroundSkill::Update_SkillAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선) // 거리 계산에 따른 Animation Scale 조절.
	m_fRootMotionScale = m_pRover->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; 

	ERoverSkillType eSkillType = static_cast<ERoverSkillType>(m_iCurrentAnimIdx);

    CCharacterState::Play_Animation(m_pRover, fTimeDelta, m_fAnimationScale);

    // Target이 존재한다면? => Auto Target
    if (m_iPartType == CRover::PARTTYPE::PART_DARKSCYTHE || CRover::PART_DARKWING)
    {
        m_pRover->Play_PartAnimation(
            m_iPartType,
            m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName),
            fTimeDelta * m_Animations.at(m_iCurrentAnimIdx).fSpeed, nullptr, 1.f, true, false
        );
    }

     
}

void CRoverGroundSkill::Check_Physcis(_float fTimeDelta)
{
	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
}

void CRoverGroundSkill::Check_StateTransition(_float fTimeDelta)
{
    ERoverSkillType eSkillType = static_cast<ERoverSkillType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

    if (IsEscapePossible)
    {
		if (m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
				return;
			}
		}
    }


    // 가장 우선순위 낮음.
    if (m_IsAnimationEnd)
    {
        if (m_States[LAND])
        {
            m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STANDCHANGE;
            m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
            return;
        }

        if (!m_States[LAND])
        {
            m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
            m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL));
            return;
        }

		
    }
    
}
void CRoverGroundSkill::SetUp_Animations()
{
    //CState::Add_Animations(ENUM_CLASS(ERoverSkillType::EX_SKILL02), "Ex_Skill02", 1.f, 120.f);
    CState::Add_Animations(ENUM_CLASS(ERoverSkillType::EX_SKILL02), "Ex_Skill02", 1.f, 40.f);
	CState::Add_Animations(ENUM_CLASS(ERoverSkillType::SKILL02), "Skill02", 1.f, 40.f);

    m_PartsAnimations.emplace("Ex_Skill02", "Scythe_Ex_Attack03");
    m_PartsAnimations.emplace("Skill02", "G_Skill02");

}

void CRoverGroundSkill::State_Reset()
{
    for (_uint i = 0; i < SKILLSTATE::END; ++i)
        m_States[i] = false;
}



CRoverGroundSkill* CRoverGroundSkill::Create(CCharacter* pOwner)
{
    CRoverGroundSkill* pInstance = new CRoverGroundSkill();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverGroundSkill");
    }

    return pInstance;
}

void CRoverGroundSkill::Free()
{
    CGroundState::Free();


}

