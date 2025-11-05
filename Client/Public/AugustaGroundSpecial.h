#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Augusta Skill State - Skill_Hack, Skill_Rise, Skill_Strike, SkillQTE, Burst01 처리
// Default Skill State

// 1 -> 2 -> 3 -> 1 -> 2 -> 3
class CAugustaGroundSpecial final : public CGroundState
{
private:
    enum SPEICALSTATE
    {
       ATTACK,
       MOVE,
       DASH,
       STOP,
       LAND,

	   ATTACK01,
	   ATTACK02,
	   ATTACK03,
	   ATTACK04,
	   ATTACK05,
	   ATTACK06,
	   ATTACKOMNI,
       END
    };

    enum COMBO
    {
        NONE = 0,
        COMBO_ATTACK01,
        COMBO_ATTACK02,
        COMBO_ATTACK03,
        COMBO_ATTACK04,
        COMBO_ATTACK05,
        COMBO_ATTACK06,
        COMBO_ATTACKOMNI,
        COMBO_END
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
    _uint m_iComboCount = { };

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
