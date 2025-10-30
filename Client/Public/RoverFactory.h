#pragma once
#include "Base.h"
#include "Client_Define.h"
NS_BEGIN(Client)
class CRoverFactory final : public CBase
{
public:
	static void Register_States(class CStateMachine* pStateMachineCom, class CAugusta* pPlayer);
};
NS_END

