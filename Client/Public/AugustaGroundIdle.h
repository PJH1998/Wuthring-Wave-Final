#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Idle State - Stand 관련 모든 애니메이션 관리
class CAugustaGroundIdle final : public CGroundState
{
private:
    enum IDLESTATE
    {
        JUMP = 0,
        SPRINT,
        MOVE,
        ATTACK,
        SKILL_Q,
        SKILL_E,
        SKILL_R,
        END
    };

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
    vector<ANIM_DATA> m_IdleStates = {};

    _bool m_States[IDLESTATE::END] = {};

private:
    virtual void Handle_Input() override;
    
    void Update_IdleAnimations(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);
    void LockOn_StateTransition(_float fTimeDelta);

    void Setup_Animations();
    void State_Reset(); // 상태 초기화

public:
    static CAugustaGroundIdle* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
