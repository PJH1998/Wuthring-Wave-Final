#pragma once
#include "GroundState.h"

NS_BEGIN(Client)

// Run State - Run 관련 모든 애니메이션 관리
class CRoverGroundSprint final : public CGroundState
{
private:
    enum RUNSTATE // Transition에 사용하는 상태들을 정의 해두기.
    {
		HIT = 0, // 피격 상태 최우선순위
        JUMP,
		FLY,
		ROPE_HOOK,
		ROPE_DRAG,
        DASH,
		DODGE,
		DODGEABLE,
        ATTACK,
        WALL,
        LAND,
		FALL,
        RUN_U,
        RUN_D,
        RUN_L,
        RUN_R,
        SPRINT,
		MOVE,

		LOCKON,

        SKILL_E,
        SKILL_Q,
        SKILL_R,

		SPRINT_F,
		BURST,
		BURST_E,
		DEFAULT_E,
		ULTI,

		// 소모값 없음 => 쿨타임 존재.
		NORMAL_E,

		// 소모값 있는 얘들 => Cost가 있을때만
		
		POINT_E,  // 중앙 Point가 가득찬 상태.
		ECHO_R,  // 중앙 Echo가 가득 찬 상태. 
		SWORD_R, // Sword R 상태 => Skill 검으로 바뀜. (Sword UI가 가득 찼을때)
        
        END
    };

private:
    explicit CRoverGroundSprint() = default;
    virtual ~CRoverGroundSprint() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CRover* m_pRover = { nullptr };

    // Run State가 관리하는 애니메이션 리스트
    _float3 m_vMoveDirection = {};
    _bool m_States[RUNSTATE::END] = {};
	_bool m_IsPrevLockOn = {};
    _float m_fSpeed = {};

	_float m_fFallTime = {};
	

private:
    virtual void Handle_Input() override;
    void Update_RunAnimation(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void Setup_Animations();
    void State_Reset();

public:
    static CRoverGroundSprint* Create(class CGameObject* pOwner);
    virtual void Free() override;
};

NS_END
