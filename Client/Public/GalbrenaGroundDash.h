#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Galbrena Sprint State - Sprint_F
class CGalbrenaGroundDash final : public CGroundState
{
private:
    enum DASHSTATE
    {
        JUMP = 0,
        MOVE,
		LAND,
		DASH,
		DODGE,
		DODGEABLE,
		HIT,
        END
    };

private:
    explicit CGalbrenaGroundDash() = default;
    virtual ~CGalbrenaGroundDash() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CGalbrena* m_pGalbrena = { nullptr };
    _bool m_States[END] = {};

private:
    virtual void Handle_Input() override;
    void Update_SprintAnimation(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();
    void State_Reset();

public:
    static CGalbrenaGroundDash* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
