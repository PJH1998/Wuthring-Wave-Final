#pragma once
#include "InteractionState.h"

NS_BEGIN(Client)
class CAugustaRopeHook final : public CInteractionState
{
private:
	enum ROPESTATE
	{
		MOVE = 0,
		JUMP,
		FALL,
		LAND,
		REACHED,
		ROPE_EXIT,
		END
	};

	enum ROPESTEP
	{
		STEP_START,
		STEP_START2,
		STEP_LOOP,
		STEP_END,
		STEP_NONE,
	};

private:
	explicit CAugustaRopeHook() = default;
	virtual ~CAugustaRopeHook() = default;

public:
	virtual HRESULT Initialize(class CGameObject* pOwner) override;
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;

private:
	class CAugusta* m_pAugusta = { nullptr };
	_bool m_States[ROPESTATE::END] = {};

	ROPEDIR m_eRopeDir = { ROPEDIR::END };
	ROPESTEP m_eRopeStep = { ROPESTEP::STEP_NONE };

	_float m_fTargetDistance = { }; // 타겟과의 거리? => 이건 Character가 알고있죠.
	

private:
	void Enter_Rope();

private:
	virtual void Handle_Input() override;
	void Update_RopeAnimation(_float fTimeDelta);
	void Check_Physics(_float fTimeDelta);
	void Check_StateTransition(_float fTimeDelta);

	void Setup_Animations();
	void State_Reset();


public:
	static CAugustaRopeHook* Create(class CGameObject* pOwner);
	virtual void Free() override;

};
NS_END

