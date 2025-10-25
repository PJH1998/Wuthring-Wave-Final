#pragma once
#include "AirState.h"

NS_BEGIN(Client)

// Jump State - Jump 관련 모든 애니메이션 관리
class CAugustaAirFall final : public CAirState
{
private:
    explicit CAugustaAirFall() = default;
    virtual ~CAugustaAirFall() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    void Setup_Animations();


    void Update_FallAnimation(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);


public:
    static CAugustaAirFall* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
