#pragma once
#include "HitState.h"

NS_BEGIN(Client)

class CGalbrenaHit final : public CHitState
{
private:
    enum HITSTATE
    {
        MOVE = 0,
        JUMP,
        FALL,
        LAND,
        END
    };

private:
    explicit CGalbrenaHit() = default;
    virtual ~CGalbrenaHit() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CGalbrena* m_pGalbrena = { nullptr };
    _bool m_States[HITSTATE::END] = {};
	_vector m_vKnockbackVelocity = {}; // 날아가는 속도와 방향.

private:
	// Enter 초기에 작업해야할 것들 정의하기.
	void Enter_Hit();

private:
    virtual void Handle_Input() override;
    void Update_HitAnimation(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void Setup_Animations();
    void State_Reset();


public:
    static CGalbrenaHit* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
