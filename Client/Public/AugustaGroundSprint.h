#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Augusta Sprint State - Sprint_F, Super_Sprint_*, SpWalk_*, Stop_Sprint_L/R 처리
class CAugustaGroundSprint final : public CGroundState
{
private:
    explicit CAugustaGroundSprint() = default;
    virtual ~CAugustaGroundSprint() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _float m_fSprintTime = { 0.f };  // Sprint 지속 시간

    void Update_SprintSpeed(_float fTimeDelta);
    void Check_StateTransition();

public:
    static CAugustaGroundSprint* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
