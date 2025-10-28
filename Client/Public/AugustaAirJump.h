#pragma once
#include "AirState.h"

NS_BEGIN(Client)

// Jump State - Jump 관련 모든 애니메이션 관리
class CAugustaAirJump final : public CAirState
{
private:
    enum JUMPSTATE
    {
        JUMP = 0,
        LAND,
        MOVE,
        DOUBLE_JUMP,
        END
    };

private:
    explicit CAugustaAirJump() = default;
    virtual ~CAugustaAirJump() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };

    _bool m_States[JUMPSTATE::END] = {};

private:
    virtual void Handle_Input() override;
    void Update_JumpAnimation(_float fTimeDelta);

    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();


public:
    static CAugustaAirJump* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
