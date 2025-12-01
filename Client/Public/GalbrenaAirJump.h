#pragma once
#include "AirState.h"

NS_BEGIN(Client)

// Jump State - Jump 관련 모든 애니메이션 관리
class CGalbrenaAirJump final : public CAirState
{
private:
    enum JUMPSTATE
    {
        JUMP = 0,
        LAND,
        MOVE,
		FLY,
		ROPE_HOOK,
        DOUBLE_JUMP,
        AIR_ATTACK,
		HIT,
        END
    };

private:
    explicit CGalbrenaAirJump() = default;
    virtual ~CGalbrenaAirJump() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CGalbrena* m_pGalbrena = { nullptr };

    _bool m_States[JUMPSTATE::END] = {};

private:
    virtual void Handle_Input() override;
    void Update_JumpAnimation(_float fTimeDelta);

    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();


public:
    static CGalbrenaAirJump* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
