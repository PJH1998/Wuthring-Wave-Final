#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Galbrena Dodge State - Sprint_F, Super_Sprint_*, SpWalk_*, Stop_Sprint_L/R 처리
class CGalbrenaGroundDodge final : public CGroundState
{
private:
    enum DASHSTATE
    {
        JUMP = 0,
        MOVE,
		LOCKON_W,
		LOCKON_S,
		LOCKON_A,
		LOCKON_D,
		LAND,
		LOCKON,
        END
    };

private:
    explicit CGalbrenaGroundDodge() = default;
    virtual ~CGalbrenaGroundDodge() = default;

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

public:
    static CGalbrenaGroundDodge* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
