#pragma once
#include "AugustaBaseState.h"

NS_BEGIN(Client)
class CAugustaRun_F final : public CAugustaBaseState
{
private:
	explicit CAugustaRun_F() = default;
	virtual ~CAugustaRun_F() = default;

public:
    virtual HRESULT Initialize(const STATE_DATA& StateData);
    virtual void OnEnter() override; // 들어갔을 시.
    virtual void OnUpdate(_float fTimeDelta) override; // Update
    virtual void OnExit(); // Exit 시


private:
	void Ready_Transitions();

public:
	static CAugustaRun_F* Create(const STATE_DATA& stateData);
	virtual void Free()override;
};
NS_END
