#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Augusta Attack State - Attack01~04, Attack_*, SpAttack* 처리
class CAugustaGroundAttack final : public CGroundState
{
private:
    enum ATTACKSTATE
    {
        HEAVY_ATTACK_PENDING,
        ATTACK1,
        ATTACK2,
        ATTACK3,
        ATTACK4,
        HEAVY_ATTACK,
        SKILL_Q = 3,
        SKILL_E = 4,
        SKILL_R = 5,
        JUMP,
        END
    };

private:
    explicit CAugustaGroundAttack() = default;
    virtual ~CAugustaGroundAttack() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _uint m_iComboCount = { 0 };  // 현재 콤보 단계 (0~3)

    _float m_fAttackPressTime = { 0.f };
    _bool m_States[ATTACKSTATE::END] = {};
    

private:
    virtual void Handle_Input() override;
    void Update_AttackAnimations(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void LockOn_StateTransition(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void SetUp_Animations();
    void State_Reset();

public:
    static CAugustaGroundAttack* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
