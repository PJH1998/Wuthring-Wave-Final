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

    
}

void CAugustaGroundSkill::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    
}

void CAugustaGroundSkill::OnExit()
{
    CGroundState::OnExit();
}

void CAugustaGroundSkill::Handle_Input()
{
}

void CAugustaGroundSkill::Update_SkillAnimations(_float fTimeDelta)
{
}

void CAugustaGroundSkill::Check_Physcis(_float fTimeDelta)
{
}

void CAugustaGroundSkill::Check_StateTransition(_float fTimeDelta)
{
}

void CAugustaGroundSkill::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ESkillType::SKILL_HACK), "Skill_Hack", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESkillType::SKILL_RISE), "Skill_Rise", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESkillType::SKILL_RISE_ZERO), "Skill_Rise_Zero", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESkillType::SKILL_STRIKE), "Skill_Strike", 1.f, 0.f);
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
