#pragma once
#include "AirState.h"

NS_BEGIN(Client)

// Augusta Attack State - Attack01~04, Attack_*, SpAttack* 처리
class CAugustaAirAttack final : public CAirState
{
private:
    enum AIRATTACKSTATE // 내부에서 전환 가능한 상태.
    {
        ATTACK,
        MOVE,
        JUMP,
        DOUBLE_JUMP,
        LAND,
		UNIQUE_GRIFFON,
		AIRATTACK_HACKDOWN_END,
        END
    };

private:
    explicit CAugustaAirAttack() = default;
    virtual ~CAugustaAirAttack() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _bool m_States[AIRATTACKSTATE::END] = {};
    _float m_fSpeed = {};

	map<_string, _string> m_PartsAnimations = {}; // Parts의 애니메이션이 서로 달라서?
    


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
    static CAugustaAirAttack* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
