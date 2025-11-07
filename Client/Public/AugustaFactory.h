#pragma once
#include "Client_Define.h"
NS_BEGIN(Client)
class CAugustaFactory final : public CBase
{
public:
	static void Register_States(class CStateMachine* pStateMachineCom, class CAugusta* pCharacter);
};
NS_END

