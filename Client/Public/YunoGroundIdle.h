#pragma once
#include "GroundState.h"

NS_BEGIN(Client)
class CYunoGroundIdle final : public CGroundState
{
private:
	enum IDLESTATE
	{
		ATTACK = 0,
		END
	};

private:
	explicit CYunoGroundIdle() = default;
	virtual ~CYunoGroundIdle() = default;

public:
	virtual HRESULT Initialize(class CCharacter* pCharacter) override;
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;

private:
	class CYuno* m_pYuno = { nullptr };

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
	static CYunoGroundIdle* Create(class CCharacter* pOwner);
	virtual void Free() override;
};
NS_END

