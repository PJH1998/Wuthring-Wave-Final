#pragma once
#include "AugustaBaseState.h"

NS_BEGIN(Client)
class CAugustaStand1_Action02 final : public CAugustaBaseState
{
private:
	explicit CAugustaStand1_Action02() = default;
	virtual ~CAugustaStand1_Action02() = default;

public:
    virtual HRESULT Initialize(const STATE_DATA& StateData);
    virtual void OnEnter() override; // 들어갔을 시.
	virtual void OnUpdate(_float fTimeDelta) override; // Update
    virtual void OnExit() override; // Exit 시


private:
	void Ready_Transitions();

public:
	static CAugustaStand1_Action02* Create(const STATE_DATA& stateData);
	virtual void Free()override;
};
NS_END
