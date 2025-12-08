#pragma once
#include "InteractionState.h"

NS_BEGIN(Client)
class CGalbrenaControl final : public CInteractionState
{
private:
	enum CONTROLSTATE
	{
		MOVE = 0,
		CONTROL_EXIT,
		END
	};

	enum CONTROLSTEP
	{
		STEP_START,
		STEP_LOOP,
		STEP_END,
		STEP_NONE,
	};

private:
	explicit CGalbrenaControl() = default;
	virtual ~CGalbrenaControl() = default;

public:
	virtual HRESULT Initialize(class CCharacter* pCharacter) override;
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;

private:
	class CGalbrena* m_pGalbrena = { nullptr };
	_bool m_States[CONTROLSTATE::END] = {};

	ROPEDIR m_eRopeDir = { ROPEDIR::END };
	CONTROLSTEP m_eControlStep = { CONTROLSTEP::STEP_NONE };

	_float m_fTargetDistance = { }; // 타겟과의 거리? => 이건 Character가 알고있죠.
	

private:
	void Enter_Rope();

private:
	virtual void Handle_Input() override;
	void Update_ControlAnimation(_float fTimeDelta);
	void Check_Physics(_float fTimeDelta);
	void Check_StateTransition(_float fTimeDelta);

	void Setup_Animations();
	void State_Reset();


public:
	static CGalbrenaControl* Create(class CCharacter* pOwner);
	virtual void Free() override;

};
NS_END

