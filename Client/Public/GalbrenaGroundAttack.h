#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Galbrena Attack State - Attack01~04, Attack_*, SpAttack* 처리
class CGalbrenaGroundAttack final : public CGroundState
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
        HEAVY_ATTACK_READY,
		DASH,
        SKILL_Q,
        SKILL_E,
        SKILL_R,
        MOVE,
		HIT,
		HIT_PENDING,
        JUMP,

		DEFAULT_E,
		BURST_E,
		BURST,
		ULTI,
        END
    };

private:
    explicit CGalbrenaGroundAttack() = default;
    virtual ~CGalbrenaGroundAttack() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CGalbrena* m_pGalbrena = { nullptr };
    _uint m_iComboCount = { 0 };  // 현재 콤보 단계 (0~4)

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
    static CGalbrenaGroundAttack* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
