#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Run State - Run 관련 모든 애니메이션 관리
class CAugustaGroundLand final : public CGroundState
{
private:
    explicit CAugustaGroundLand() = default;
    virtual ~CAugustaGroundLand() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };

    // Run State가 관리하는 애니메이션 리스트
    _float3 m_vMoveDirection = {};

    void Setup_Animations();
    void Update_LandAnimation(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);


public:
    static CAugustaGroundLand* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
