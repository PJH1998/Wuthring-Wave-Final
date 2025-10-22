#pragma once
#include "AugustaBaseState.h"

NS_BEGIN(Client)
class CAugustaStop_Run_L final : public CAugustaBaseState
{
private:
	explicit CAugustaStop_Run_L() = default;
	virtual ~CAugustaStop_Run_L() = default;

public:
    virtual HRESULT Initialize(const STATE_DATA& StateData);
    virtual void OnEnter() override; // 들어갔을 시.
    virtual void OnUpdate(_float fTimeDelta) override; // Update
    virtual void OnExit(); // Exit 시


private:
	void Ready_Transitions();

public:
	static CAugustaStop_Run_L* Create(const STATE_DATA& stateData);
	virtual void Free()override;
};
NS_END
