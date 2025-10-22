#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Idle State - Stand 관련 모든 애니메이션 관리
class CAugustaGroundIdle final : public CGroundState
{
private:
    explicit CAugustaGroundIdle() = default;
    virtual ~CAugustaGroundIdle() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };

private:
    void Check_StateTransition();
    void Setup_Animations();

public:
    static CAugustaGroundIdle* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
