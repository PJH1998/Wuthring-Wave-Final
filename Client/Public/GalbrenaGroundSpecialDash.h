#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Galbrena Sprint State - Sprint_F
class CGalbrenaGroundSpecialDash final : public CGroundState
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
    explicit CGalbrenaGroundSpecialDash() = default;
    virtual ~CGalbrenaGroundSpecialDash() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
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

	_bool Hit_Judge();

public:
    static CGalbrenaGroundSpecialDash* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
