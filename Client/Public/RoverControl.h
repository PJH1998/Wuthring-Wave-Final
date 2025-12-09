#pragma once
#include "InteractionState.h"

NS_BEGIN(Client)
class CRoverControl final : public CInteractionState
{
private:
	enum CONTROLSTATE
	{
		MOVE = 0,
		DETACH,
		THROW,
		RUN,
		END
	};

	enum CONTROLSTEP
	{
		STEP_ATTACH,		// 잡는 상태
		STEP_ATTACH_LOOP,   // 잡기 유지.
		STEP_DETACH,		// 잡기 해제.
		STEP_THROW,		// 던지기.
		STEP_NONE,
	};

private:
	explicit CRoverControl() = default;
	virtual ~CRoverControl() = default;

public:
	virtual HRESULT Initialize(class CCharacter* pCharacter) override;
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;

private:
	class CRover* m_pRover = { nullptr };
	_bool m_States[CONTROLSTATE::END] = {};

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
	static CRoverControl* Create(class CCharacter* pOwner);
	virtual void Free() override;

};
NS_END

