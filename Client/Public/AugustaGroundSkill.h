#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Augusta Skill State - Skill_Hack, Skill_Rise, Skill_Strike, SkillQTE, Burst01 처리
class CAugustaGroundSkill final : public CGroundState
{
private:
    enum SKILLSTATE
    {

    };

private:
    explicit CAugustaGroundSkill() = default;
    virtual ~CAugustaGroundSkill() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _string m_strSkillType = {};  // "Hack", "Rise", "Strike", "QTE", "Burst"

private:
    void Check_StateTransition();

public:
    static CAugustaGroundSkill* Create(class CGameObject* pOwner, const _string& skillType);
    virtual void Free() override;
};

NS_END
