#pragma once
#include "AirState.h"

NS_BEGIN(Client)

// Augusta Attack State - Attack01~04, Attack_*, SpAttack* 처리
class CAugustaAirSkill final : public CAirState
{
private:
    enum AIRSKILLSTATE // 내부에서 전환 가능한 상태.
    {
        MOVE,
        JUMP,
        FALL,
        LAND,
        END
    };

private:
    explicit CAugustaAirSkill() = default;
    virtual ~CAugustaAirSkill() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _bool m_States[AIRSKILLSTATE::END] = {};
    _float m_fSpeed = {};
    

private:
    virtual void Handle_Input() override;
    void Update_AttackAnimations(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void LockOn_StateTransition(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void SetUp_Animations();
    void State_Reset();

public:
    static CAugustaAirSkill* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
