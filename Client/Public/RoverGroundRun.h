#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Run State - Run 관련 모든 애니메이션 관리
class CRoverGroundRun final : public CGroundState
{
private:
    enum RUNSTATE // Transition에 사용하는 상태들을 정의 해두기.
    {
        JUMP = 0,
        DASH,
        ATTACK,
        WALL,
        LAND,
        RUN_U,
        RUN_D,
        RUN_L,
        RUN_R,
        SPRINT_F,
        SKILL_E,
        SKILL_Q,
        SKILL_R,
        UNIQUE_E, 
        UNIQUE_R,
        BURST_R,
        MOVE,
        END
    };

private:
    explicit CRoverGroundRun() = default;
    virtual ~CRoverGroundRun() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CRover* m_pRover = { nullptr };

    // Run State가 관리하는 애니메이션 리스트
    _float3 m_vMoveDirection = {};
    _bool m_States[RUNSTATE::END] = {};
    _float m_fSpeed = {};

private:
    virtual void Handle_Input() override;
    void Update_RunAnimation(_float fTimeDelta);
    void Check_Physics();
    void Check_StateTransition(_float fTimeDelta);

    void Setup_Animations();
    void State_Reset();

public:
    static CRoverGroundRun* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
