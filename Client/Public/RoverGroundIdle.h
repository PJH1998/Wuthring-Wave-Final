#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Idle State - Stand 관련 모든 애니메이션 관리
class CRoverGroundIdle final : public CGroundState
{
private:
    enum IDLESTATE
    {
        JUMP = 0,
        DASH,
		LAND,
        SPRINT,
        MOVE,
        ATTACK,
        MOVE_U,
        MOVE_D,
        MOVE_L,
        MOVE_R,
        AIR_ATTACK_E,
        SKILL_E,
        SKILL_Q,
        SKILL_R,

		// 소모값 필요한 얘들
        UNIQUE_E, // Unique E상태. => 그리폰 타서 공격.
        UNIQUE_R,
        BURST_R, // Burst R 상태 => Skill 검으로 바뀜.
        END
    };

private:
    explicit CRoverGroundIdle() = default;
    virtual ~CRoverGroundIdle() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CRover* m_pRover = { nullptr };
    vector<ANIM_DATA> m_IdleStates = {};

    _bool m_States[IDLESTATE::END] = {};

private:
    virtual void Handle_Input() override;
    
    void Update_IdleAnimations(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();
    void State_Reset(); // 상태 초기화

public:
    static CRoverGroundIdle* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
