#pragma once
#include "AirState.h"

NS_BEGIN(Client)

// Augusta Attack State - Attack01~04, Attack_*, SpAttack* 처리
class CAugustaAirFly final : public CAirState
{
private:
    enum AIRFLYSTATE // 내부에서 전환 가능한 상태.
    {
		INPUT_U,
		INPUT_D,
		INPUT_L,
		INPUT_R,
		INPUT_ACCEL,
		FLY_U,
		FLY_D,
		FLY_L,
		FLY_R,
		FLY_STAND,
        MOVE,
		ATTACK, // 낙하 공격
        JUMP,
        DOUBLE_JUMP,
        LAND,
		FALL,
        END
    };

private:
    explicit CAugustaAirFly() = default;
    virtual ~CAugustaAirFly() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pCharacter) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

private:
    class CAugusta* m_pAugusta = { nullptr };
    _bool m_States[AIRFLYSTATE::END] = {};
    
	_float m_fSoundTimer = {};
	_float m_fMaxTime = {};
	vector<_wstring> m_SoundTags = {};

	

	_float m_fSpeed = {};		// Speed (힘)
	_float m_fAccel = {};		// 가속.
	_float m_fLift = {};		// 양력
	_float m_fDrag = {};		// 공기 저항.
	_float3 m_vGravity = {};    // Gravity (감속)
	_vector m_vForce = {}; // 현재 작용하는 힘.

	GPU_BLEND_INFO m_GpuBlendInfo = {};

	_float m_fVerticalBoostTimer = {};
	_float m_fMaxVerticalAccel = { 15.f }; // 최대 수직 가속도
	
     

private:
	void Process_Timer(_float fTimeDelta);
    virtual void Handle_Input() override;
    void Update_FlyAnimations(_float fTimeDelta);
    void Check_Physics(_float fTimeDelta);
    void Check_StateTransition(_float fTimeDelta);

    void SetUp_Animations();
    void State_Reset();

public:
    static CAugustaAirFly* Create(class CCharacter* pOwner);
    virtual void Free() override;
};

NS_END
