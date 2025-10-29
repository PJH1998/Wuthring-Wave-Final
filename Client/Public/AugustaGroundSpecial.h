#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Augusta Skill State - Skill_Hack, Skill_Rise, Skill_Strike, SkillQTE, Burst01 처리
// Default Skill State
class CAugustaGroundSpecial final : public CGroundState
{
private:
    enum SPEICALSTATE
    {
       ATTACK,
       MOVE,
       DASH,
       STOP,
       END
    };

private:
    explicit CAugustaGroundSpecial() = default;
    virtual ~CAugustaGroundSpecial() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;


private:
    class CAugusta* m_pAugusta = { nullptr };
    _bool m_States[SPEICALSTATE::END] = {};
    map<_string, _string> m_PartsAnimations = {};

private:
    virtual void Handle_Input() override;
    void Update_SkillAnimations(_float fTimeDelta);
    void Check_Physcis(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta) ;
    void SetUp_Animations();
    void State_Reset();

public:
    static CAugustaGroundSpecial* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
