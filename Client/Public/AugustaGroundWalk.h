#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Augusta Walk State (Walk_F/B/LF/RF/LB/RB, Stop_Walk_L/R)
class CAugustaGroundWalk final : public CGroundState
{
private:
    explicit CAugustaGroundWalk() = default;
    virtual ~CAugustaGroundWalk() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    void Update_WalkDirection(_float fTimeDelta);
    void Check_StateTransition();

public:
    static CAugustaGroundWalk* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
