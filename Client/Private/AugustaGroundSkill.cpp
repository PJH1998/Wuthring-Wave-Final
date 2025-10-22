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

    return S_OK;
}

void CAugustaGroundSkill::OnEnter()
{
    CGroundState::OnEnter();

    // 스킬 타입에 따라 애니메이션 재생
    // if (m_strSkillType == "Hack") → Skill_Hack
    // if (m_strSkillType == "Rise") → Skill_Rise
    // if (m_strSkillType == "Strike") → Skill_Strike
    // if (m_strSkillType == "QTE") → SkillQTE
    // if (m_strSkillType == "Burst") → Burst01
}

void CAugustaGroundSkill::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 스킬 애니메이션 재생 및 효과 처리
}

void CAugustaGroundSkill::OnExit()
{
    CGroundState::OnExit();
}

void CAugustaGroundSkill::Check_StateTransition()
{
    // TODO: 스킬 애니메이션 종료 시 Idle로
    // TODO: 특정 스킬은 공격으로 캔슬 가능
}

CAugustaGroundSkill* CAugustaGroundSkill::Create(class CGameObject* pOwner, const _string& skillType)
{
    CAugustaGroundSkill* pInstance = new CAugustaGroundSkill();
    pInstance->m_strSkillType = skillType;

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
