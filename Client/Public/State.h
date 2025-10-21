#pragma once
#include "Base.h"

NS_BEGIN(Client)
class CState : public CBase
{

public:
	virtual void OnEnter();
	virtual void OnUpdate();
	virtual void OnExit();
};
NS_END

