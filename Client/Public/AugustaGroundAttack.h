#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Augusta Attack State - Attack01~04, Attack_*, SpAttack* 처리
class CAugustaGroundAttack final : public CGroundState
{
private:
    enum ATTACKSTATE // 내부에서 전환 가능한 상태.
    {
		ATTACK = 0,
        FIRST_ATTACK,
        SECOND_ATTACK,
        THIRED_ATTACK,
        LAST_ATTACK,
        HEAVY_ATTACK_PENDING,
        HEAVY_ATTACK,
        SKILL_Q,
        SKILL_E,
        SKILL_R,
        MOVE,
		HIT_PENDING, // 맞고 있는지 알려줌
		HIT, // 실제 전환되는 경우

		DODGE,
		DODGEABLE,
		DASH,
        JUMP,
        END
    };

private:
    explicit CAugustaGroundAttack() = default;
    virtual ~CAugustaGroundAttack() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _uint m_iComboCount = { 0 };  // 현재 콤보 단계 (0~3)

    _float m_fAttackPressTime = { 0.f };
    _float m_fAttackPressMaxTime = { 0.5f };
    _bool m_States[ATTACKSTATE::END] = {};
    _bool m_IsNextAttackInput = { false };
    
    
private:
	_bool Hit_Judge();

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
