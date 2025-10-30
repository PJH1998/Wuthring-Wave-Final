#pragma once
#include "HitState.h"

NS_BEGIN(Client)

class CAugustaHit final : public CHitState
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
    explicit CAugustaHit() = default;
    virtual ~CAugustaHit() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _bool m_States[HITSTATE::END] = {};

private:
    virtual void Handle_Input() override;
    void Update_HitAnimation(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void Setup_Animations();
    void State_Reset();


public:
    static CAugustaHit* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
