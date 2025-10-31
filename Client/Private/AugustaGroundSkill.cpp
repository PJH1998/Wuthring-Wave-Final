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

void CAugustaGroundSkill::OnEnter()
{
    CGroundState::OnEnter();

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

            const _string& strAnimName = m_Animations[m_iCurrentAnimIdx].strAnimName;
            auto iter = m_PartsAnimations.find(strAnimName);
            if (iter != m_PartsAnimations.end())
                m_pAugusta->Clear_PartAnimation(m_iPartType, iter->second);
            
            break;
        }
        case EAugustaSkillType::SKILL_RISE:
        {
            m_pAugusta->Set_Gravity(false);
            break;
        }
        
    }
    // 진입할때 한번만.
	m_pAugusta->Rotate_Target();
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
    
    m_pAugusta->PartActivate(m_iPartType, false);
    m_pAugusta->Set_Gravity(true);
    

    m_iPartType = CAugusta::PARTTYPE::TYPE_END;
}

void CAugustaGroundSkill::Handle_Input()
{
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[SKILL_E] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));

}

void CAugustaGroundSkill::Update_SkillAnimations(_float fTimeDelta)
{
	EAugustaSkillType eSkillType = static_cast<EAugustaSkillType>(m_iCurrentAnimIdx);

    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // Target이 존재한다면? => Auto Target
	
	
    //m_pAugusta->Rotate_Target();

    //m_pAugusta->Move_Direction(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta, 0.1f);

    if (m_iPartType != CAugusta::PARTTYPE::TYPE_END)
    {
        m_pAugusta->Play_PartAnimation(
            m_iPartType,
            m_PartsAnimations[m_Animations[m_iCurrentAnimIdx].strAnimName],
            fTimeDelta * m_Animations[m_iCurrentAnimIdx].fSpeed, nullptr, 1.f, true, false
        );
    }
     
}

void CAugustaGroundSkill::Check_Physcis(_float fTimeDelta)
{
    m_States[LAND] = m_pAugusta->Get_DistanceToGround(0.1f) <= 0.2f;
}

void CAugustaGroundSkill::Check_StateTransition(_float fTimeDelta)
{
    EAugustaSkillType eSkillType = static_cast<EAugustaSkillType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

    if (IsEscapePossible)
    {
        if (eSkillType == EAugustaSkillType::SKILL_STRIKE && m_States[SKILL_E])
        {
            m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::SKILL_RISE;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL));
            return;
        }

        // 더블 점프 형태로만 변경 가능.
        if (m_States[JUMP])
        {
            m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
            return;
        }

        // 땅에 닿으면. 우선 순위
        if (m_States[LAND])
        {
            if (m_States[MOVE])
            {
                m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
                return;
            }

            if (!m_States[MOVE])
            {
                m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
                return;
            }
        }
  
    }


    // 가장 우선순위 낮음.
    if (m_IsAnimationEnd)
    {
        if (m_States[LAND])
        {
            m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
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
    CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::SKILL_HACK), "Skill_Hack", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::SKILL_RISE), "Skill_Rise", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::SKILL_RISE_ZERO), "Skill_Rise_Zero", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::SKILL_STRIKE), "Skill_Strike", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSkillType::SKILLQTE), "SkillQTE", 1.f, 0.f);

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
