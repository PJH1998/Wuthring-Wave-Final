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
		LAND,
        DASH,
        SPRINT,
        MOVE,
		FLY,
        ATTACK,
        MOVE_U,
        MOVE_D,
        MOVE_L,
        MOVE_R,
        AIR_ATTACK_E,
        SKILL_E, // Normal E 상태
        SKILL_Q, // Echo Q 상태
        SKILL_R, 

		// 쿨타임만 있는 얘들
		NORMAL_E,

		// 소모값 있는 얘들 => Cost가 있을때만 쓸 수 있음.
        POINT_E,  // 중앙 Point가 가득찬 상태.
        ECHO_R,  // 중앙 Echo가 가득 찬 상태. 
        SWORD_R, // Sword R 상태 => Skill 검으로 바뀜. (Sword UI가 가득 찼을때)
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
