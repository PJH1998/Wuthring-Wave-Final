#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Augusta Skill State - Skill_Hack, Skill_Rise, Skill_Strike, SkillQTE, Burst01 처리
// Default Skill State

// 1 -> 2 -> 3 -> 1 -> 2 -> 3
class CGalbrenaGroundSpecial final : public CGroundState
{
private:
    enum SPEICALSTATE
    {
       ATTACK,
       MOVE,
       DASH,
	   JUMP,
       STOP,
       LAND,
	   EX_ATTACK01,
	   EX_ATTACK02,
	   EX_ATTACK03,
	   EX_ATTACK04,
	   EX_ATTACK05,
	   EX_SKILL01,
	   EX_SKILL02,
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
    explicit CGalbrenaGroundSpecial() = default;
    virtual ~CGalbrenaGroundSpecial() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;


private:
    class CGalbrena* m_pGalbrena = { nullptr };
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
    static CGalbrenaGroundSpecial* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
