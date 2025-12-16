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
		DODGE,
		DODGEABLE,
		FLY,
		ROPE_HOOK,
		ROPE_DRAG,
		THROW_CONTROL,
        SPRINT,
        MOVE,
		MOVE_U,
		MOVE_D,
		MOVE_L,
		MOVE_R,
		FALL,

		HIT,

		BURST,
		ATTACK,
		SKILL_E,
		SKILL_Q,
		SKILL_R,
		DEFAULT_E,
		BURST_E,

		ULTI,

        END
    };

private:
    explicit CRoverGroundIdle() = default;
    virtual ~CRoverGroundIdle() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CRover* m_pRover = { nullptr };

    _bool m_States[IDLESTATE::END] = {};
	_float m_fFallTime = {};

private:
    virtual void Handle_Input() override;
    
    void Update_IdleAnimations(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);
    void Setup_Animations();
    void State_Reset(); // 상태 초기화

public:
    static CRoverGroundIdle* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
