#pragma once
#include "AirState.h"

NS_BEGIN(Client)

// Yuno Air Attack State
class CYunoAirAttack final : public CAirState
{
private:
    enum AIRATTACKSTATE // 내부에서 전환 가능한 상태.
    {
        ATTACK,
		LAND,
        END
    };

private:
    explicit CYunoAirAttack() = default;
    virtual ~CYunoAirAttack() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CYuno* m_pYuno = { nullptr };
    _bool m_States[AIRATTACKSTATE::END] = {};
    _float m_fSpeed = {};

private:
    virtual void Handle_Input() override;
    void Update_AttackAnimations(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void SetUp_Animations();
    void State_Reset();

private:
	void Handle_Animation_SpecialState(); // 특수한 애니메이션 상태를 처리한다.

public:
    static CYunoAirAttack* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
