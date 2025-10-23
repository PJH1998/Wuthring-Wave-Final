#pragma once
#include "Base.h"
NS_BEGIN(Client)
class CPlayerFactory : public CBase
{
public:
	static void Register_KeyInputs(class CInputController* pInputControllerCom, class CPlayer* pPlayer);
};
NS_END
