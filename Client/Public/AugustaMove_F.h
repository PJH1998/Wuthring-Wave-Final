#pragma once
#include "AugustaBaseState.h"

NS_BEGIN(Client)
class CAugustaMove_F final : public CAugustaBaseState
{
private:
	explicit CAugustaMove_F() = default;
	virtual ~CAugustaMove_F() = default;

public:
    virtual HRESULT Initialize(const STATE_DATA& StateData);
    virtual void OnEnter() override; // 들어갔을 시.
    virtual void OnUpdate(_float fTimeDelta) override; // Update
    virtual void OnExit(); // Exit 시


private:
	void Ready_Transitions();

public:
	static CAugustaMove_F* Create(const STATE_DATA& stateData);
	virtual void Free()override;
};
NS_END
