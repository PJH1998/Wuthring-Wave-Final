#pragma once
#include "Base.h"
NS_BEGIN(Client)
class CSequencePlayerFactory : public CBase
{
public:
	static void Register_SeuqenceEvent(class CSequencePlayer* pPlayer);
};

NS_END

