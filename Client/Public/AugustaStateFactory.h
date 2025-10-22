#pragma once
#include "Base.h"

NS_BEGIN(Client)
class CAugustaStateFactory : public CBase
{
public:
	static void Register_AugustaStates(class CStateMachine* pStateMachineCom, class CAugusta* pPlayer);

	static void Register_KeyInputs(class CInputController* pInputControllerCom, class CAugusta* pPlayer);

	
};
NS_END

