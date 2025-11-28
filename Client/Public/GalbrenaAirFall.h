#pragma once
#include "AirState.h"

NS_BEGIN(Client)

// Jump State - Jump 관련 모든 애니메이션 관리
class CGalbrenaAirFall final : public CAirState
{
private:
    enum FALLSTATE
    {
        MOVE = 0,
        LAND = 1,
		FLY,
		ROPE_HOOK,
		HIT,
		AIR_ATTACK,
        END
    };

private:
    explicit CGalbrenaAirFall() = default;
    virtual ~CGalbrenaAirFall() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CGalbrena* m_pGalbrena = { nullptr };

    _bool m_States[FALLSTATE::END] = {};
private:
    virtual void Handle_Input() override;
    void Update_FallAnimation(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void Setup_Animations();
    void State_Reset();


public:
    static CGalbrenaAirFall* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
