#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Run State - Run 관련 모든 애니메이션 관리
class CRoverGroundLand final : public CGroundState
{
private:
    enum LANDSTATE
    {
        RUN = 0,
        END
    };

private:
    explicit CRoverGroundLand() = default;
    virtual ~CRoverGroundLand() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CRover* m_pRover = { nullptr };

    // Run State가 관리하는 애니메이션 리스트
    _float3 m_vMoveDirection = {};
    _bool m_States[LANDSTATE::END] = {};

private:
    virtual void Handle_Input() override;
    void Update_LandAnimation(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();
    void State_Reset();

public:
    static CRoverGroundLand* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
