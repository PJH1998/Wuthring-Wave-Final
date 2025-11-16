#pragma once
#include "Base.h"

NS_BEGIN(Client)
class CGalbrenaFactory final : public CBase
{
public:
	static void Register_States(class CStateMachine* pStateMachineCom, class CGalbrena* pCharacter);
};
NS_END

