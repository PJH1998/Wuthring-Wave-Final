#pragma once
#include "InteractionState.h"

NS_BEGIN(Client)
class CGalbrenaRope final : public CInteractionState
{
private:
	enum ROPESTATE
	{
		MOVE = 0,
		JUMP,
		FALL,
		LAND,
		ROPE_EXIT,
		END
	};

private:
	explicit CGalbrenaRope() = default;
	virtual ~CGalbrenaRope() = default;

public:
	virtual HRESULT Initialize(class CGameObject* pOwner) override;
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;

private:
	class CGalbrena* m_pGalbrena = { nullptr };
	_bool m_States[ROPESTATE::END] = {};

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
	static CGalbrenaRope* Create(class CGameObject* pOwner);
	virtual void Free() override;

};
NS_END

