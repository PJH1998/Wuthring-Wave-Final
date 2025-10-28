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

    m_iPartType = CAugusta::PARTTYPE::PART_BAYONET;

    if (eSkillType == ESkillType::SKILL_RISE_ZERO)
    {
        _string strBoneName = "WeaponProp02";
        m_pAugusta->PartAcitvate(m_iPartType, true);
        m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
        return;
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
    //m_pAugusta->PartAcitvate(m_iPartType, false);
}

void CAugustaGroundSkill::Handle_Input()
{
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));

    m_States[LAND] = m_pAugusta->Is_Land(&m_vLandNormal);

}

void CAugustaGroundSkill::Update_SkillAnimations(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // Target이 존재한다면? => Auto Target
    m_pAugusta->Rotate_Target();

    m_pAugusta->Play_PartAnimation(
        m_iPartType,
        m_Animations[m_iCurrentAnimIdx].strAnimName,
        fTimeDelta, nullptr
    );
}

void CAugustaGroundSkill::Check_Physcis(_float fTimeDelta)
{
}

void CAugustaGroundSkill::Check_StateTransition(_float fTimeDelta)
{
    ESkillType eSkillType = static_cast<ESkillType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

    // 땅에 닿으면. 우선 순위
    if (IsEscapePossible && m_States[LAND])
    {
        m_pAugusta->GetStateContextForWrite().m_eLandType = ELandType::LAND_LIGHT;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
        return;
    }

    
    if (IsEscapePossible)
    {
        if (m_States[JUMP])
        {
            m_pAugusta->GetStateContextForWrite().m_eJumpType = EJumpType::JUMP_WALK_LF;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
            return;
        }
    }


    // 가장 우선순위 낮음.
    if (m_IsAnimationEnd)
    {
        m_pAugusta->GetStateContextForWrite().m_eIdleType = EIdleType::STAND1_ACTION01;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
        return;
    }
    
}

void CAugustaGroundSkill::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ESkillType::SKILL_HACK), "Skill_Hack", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESkillType::SKILL_RISE_ZERO), "Skill_Rise_Zero", 1.f, 10.f, 3.f);
    CState::Add_Animations(ENUM_CLASS(ESkillType::SKILLQTE), "SkillQTE", 1.f, 0.f);
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
