#pragma once
#include "AirState.h"

NS_BEGIN(Client)

// Augusta Attack State - Attack01~04, Attack_*, SpAttack* 처리
class CAugustaAirFly final : public CAirState
{
private:
    enum AIRFLYSTATE // 내부에서 전환 가능한 상태.
    {
		FLY_U,
		FLY_D,
		FLY_L,
		FLY_R,
        MOVE,
		ATTACK, // 낙하 공격
        JUMP,
        DOUBLE_JUMP,
        LAND,
		FALL,
        END
    };

private:
    explicit CAugustaAirFly() = default;
    virtual ~CAugustaAirFly() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _bool m_States[AIRFLYSTATE::END] = {};
    _float m_fSpeed = {};

	_float3 m_vDirection = {};
    

private:
    virtual void Handle_Input() override;
    void Update_FlyAnimations(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void SetUp_Animations();
    void State_Reset();

public:
    static CAugustaAirFly* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
