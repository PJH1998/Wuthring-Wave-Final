#pragma once
#include "ClimbState.h"

NS_BEGIN(Client)

// Jump State - Jump 관련 모든 애니메이션 관리
class CAugustaClimbExit final : public CClimbState
{
private:
    explicit CAugustaClimbExit() = default;
    virtual ~CAugustaClimbExit() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _bool m_IsClimbExit = { false };
    _bool m_IsSecondStep = { false };
private:
    void Setup_Animations();

    void Update_ClimbAnimation(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);


public:
    static CAugustaClimbExit* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
