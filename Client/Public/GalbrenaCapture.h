#pragma once
#include "CaptureState.h"

NS_BEGIN(Client)
class CGalbrenaCapture final : public CCaptureState
{
private:
	enum CAPTURESTATE
	{
		MOVE = 0,
		JUMP,
		DRAG,
		FALL,
		LAND,
		GRAB_EXIT,
		END
	};

	enum CAPTURESTEP
	{
		STEP_START,
		STEP_LOOP,
		STEP_END,
		STEP_NONE,
	};


private:
	explicit CGalbrenaCapture() = default ;
	virtual  ~CGalbrenaCapture() = default ;

public:
	virtual HRESULT Initialize(class CCharacter* pCharacter) override;
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;

private:
	class CGalbrena* m_pGalbrena = { nullptr };
	_bool m_States[CAPTURESTATE::END] = {};

	CAPTURESTEP m_eCaptureStep = { CAPTURESTEP::STEP_NONE };


private:
	virtual void Handle_Input() override;
	void Update_CaptureAnimation(_float fTimeDelta);
	void Check_Physics(_float fTimeDelta);
	void Check_StateTransition(_float fTimeDelta);

	void Setup_Animations();
	void State_Reset();

public:
	static CGalbrenaCapture* Create(class CCharacter* pOwner);
	virtual void Free() override;

};
NS_END

