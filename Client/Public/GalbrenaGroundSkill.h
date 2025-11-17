#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Galbrena Skill State - Skill_Hack, Skill_Rise, Skill_Strike, SkillQTE, Burst01 처리
// Default Skill State
class CGalbrenaGroundSkill final : public CGroundState
{
private:
    enum SKILLSTATE
    {
       HACK = 0,
       RISE_ZERO,
       QTE,
       IDLE,
       MOVE,
       JUMP, 
       LAND,
       SKILL_E,
       SKILL_R,

	   SKILL_RISE_ZERO,
	   SKILL_RISE,
	   AIRATTACK_HACKDOWN_START,

	   ATTACK_PULL,
	   ATTACK_SP_SKILL,

	   POINT_E,
	   SWORD_R,

       AIR_ATTACK,
       FALL,
       END
    };

private:
    explicit CGalbrenaGroundSkill() = default;
    virtual ~CGalbrenaGroundSkill() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;


private:
    class CGalbrena* m_pGalbrena = { nullptr };
    _bool m_States[SKILLSTATE::END] = {};
    map<_string, _string> m_PartsAnimations = {};

private:
    virtual void Handle_Input() override;
    void Update_SkillAnimations(_float fTimeDelta);
    void Check_Physcis(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta) ;
    void SetUp_Animations();
    void State_Reset();

	void Handle_Animation_SpecialState();


public:
    static CGalbrenaGroundSkill* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
