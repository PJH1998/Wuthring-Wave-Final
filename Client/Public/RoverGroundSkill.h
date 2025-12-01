#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Rover Skill State - Skill_Hack, Skill_Rise, Skill_Strike, SkillQTE, Burst01 처리
// Default Skill State
class CRoverGroundSkill final : public CGroundState
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
    explicit CRoverGroundSkill() = default;
    virtual ~CRoverGroundSkill() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;


private:
    class CRover* m_pRover = { nullptr };
    _bool m_States[SKILLSTATE::END] = {};
    map<_string, _string> m_PartsAnimations = {};

private:
    virtual void Handle_Input() override;
    void Update_SkillAnimations(_float fTimeDelta);
    void Check_Physcis(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta) ;
    void SetUp_Animations();
    void State_Reset();


public:
    static CRoverGroundSkill* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
