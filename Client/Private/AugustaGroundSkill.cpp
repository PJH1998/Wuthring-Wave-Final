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
    ESkillType eSkillType = context.m_eSkillType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eSkillType);

    // 4. 상태 초기화
    State_Reset();

    // 5. 애니메이션 타입에 맞는 파츠 설정.

     
    switch(eSkillType)
    {
    case ESkillType::SKILL_STRIKE:
    {
        //_string strBoneName = "WeaponProp06";
        _string strBoneName = "Root";
        m_iPartType = CAugusta::PARTTYPE::PART_GRIFFON;
        m_pAugusta->PartAcitvate(m_iPartType, true);
        m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
        break;
    }
    case ESkillType::SKILL_RISE:
    {

        break;
    }
        
    }
    
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
    m_pAugusta->PartAcitvate(m_iPartType, false);
}

void CAugustaGroundSkill::Handle_Input()
{
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);

}

void CAugustaGroundSkill::Update_SkillAnimations(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // Target이 존재한다면? => Auto Target
    m_pAugusta->Rotate_Target();

    //m_pAugusta->Move_Direction(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta, 0.1f);

    m_pAugusta->Play_PartAnimation(
        m_iPartType,
        m_PartsAnimations[m_Animations[m_iCurrentAnimIdx].strAnimName],
        fTimeDelta * m_Animations[m_iCurrentAnimIdx].fSpeed, nullptr
    );
}

void CAugustaGroundSkill::Check_Physcis(_float fTimeDelta)
{
    m_States[LAND] = m_pAugusta->Get_DistanceToGround(0.1f) <= 0.2f;
}

void CAugustaGroundSkill::Check_StateTransition(_float fTimeDelta)
{
    ESkillType eSkillType = static_cast<ESkillType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

    if (IsEscapePossible)
    {
        // 땅에 닿으면. 우선 순위
        if (m_States[LAND])
        {
            if (m_States[JUMP])
            {
                m_pAugusta->GetStateContextForWrite().m_eJumpType = EJumpType::JUMP_WALK_LF;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
                return;
            }
            if (m_States[MOVE])
            {
                m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_F;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
                return;
            }
            if (!m_States[MOVE])
            {
                m_pAugusta->GetStateContextForWrite().m_eIdleType = EIdleType::STAND1_ACTION01;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
                return;
            }
        }

        if (!m_States[LAND])
        {
            m_pAugusta->GetStateContextForWrite().m_eFallType = EFallType::FALL_LOOP;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL));
            return;
        }
        
    }


    // 가장 우선순위 낮음.
    if (m_IsAnimationEnd)
    {
        if (m_States[LAND])
        {
            m_pAugusta->GetStateContextForWrite().m_eIdleType = EIdleType::STAND1_ACTION01;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
            return;
        }

        if (!m_States[LAND])
        {
            m_pAugusta->GetStateContextForWrite().m_eFallType = EFallType::FALL_LOOP;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL));
            return;
        }
    }
    
}

void CAugustaGroundSkill::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ESkillType::SKILL_HACK), "Skill_Hack", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESkillType::SKILL_RISE), "Skill_Rise", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(ESkillType::SKILL_RISE_ZERO), "Skill_Rise_Zero", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(ESkillType::SKILL_STRIKE), "Skill_Strike", 1.f, 40.f, 2.f);
    CState::Add_Animations(ENUM_CLASS(ESkillType::SKILLQTE), "SkillQTE", 1.f, 0.f);

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
