#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Run State - Run 관련 모든 애니메이션 관리
class CGalbrenaGroundRun final : public CGroundState
{
private:
	enum RUNSTATE // Transition에 사용하는 상태들을 정의 해두기.
	{
		JUMP = 0,
		DASH,
		DODGE,
		DODGEABLE,
		FLY,
		MOVE,
		FALL,
		WALL,
		LAND,

		RUN_U,
		RUN_D,
		RUN_L,
		RUN_R,
		SPRINT,
		HIT,

		SKILL_E,
		SKILL_Q,
		SKILL_R,
		DEFAULT_E,
		BURST_E,

		BURST,
		BURST_ATTACK, // Burst 상태인경우? Burst 공격.
		ULTI,
		ATTACK,
		
		END
	};

private:
	explicit CGalbrenaGroundRun() = default;
	virtual ~CGalbrenaGroundRun() = default;

public:
	virtual HRESULT Initialize(class CGameObject* pOwner) override;
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;

private:
	class CGalbrena* m_pGalbrena = { nullptr };

	// Run State가 관리하는 애니메이션 리스트
	_float3 m_vMoveDirection = {};
	_bool m_States[RUNSTATE::END] = {};
	_float m_fSpeed = {};
	_float m_fFallTime = { 0.f };

private:
    virtual void Handle_Input() override;
    void Update_RunAnimation(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void Setup_Animations();
    void State_Reset();

public:
    static CGalbrenaGroundRun* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
