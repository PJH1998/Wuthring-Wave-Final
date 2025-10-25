#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Run State - Run 관련 모든 애니메이션 관리
class CAugustaGroundRun final : public CGroundState
{
private:
    enum RUNSTATE // Transition에 사용하는 상태들을 정의 해두기.
    {
        JUMP = 0,
        SPRINT = 1,
        RUN_U = 2,
        RUN_D = 3,
        RUN_L = 4,
        RUN_R = 5,
        MOVE = 6,
        WALL = 7,
        LAND = 8,
        END
    };

private:
    explicit CAugustaGroundRun() = default;
    virtual ~CAugustaGroundRun() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };

    // Run State가 관리하는 애니메이션 리스트
    _float3 m_vMoveDirection = {};
    

    _bool m_States[RUNSTATE::END] = {};

private:
    virtual void Handle_Input() override;
    void Update_RunAnimation(_float fTimeDelta);
    void Check_Physics();
    void LockOnCheck_StateTransition(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void Setup_Animations();
    void State_Reset();

public:
    static CAugustaGroundRun* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
