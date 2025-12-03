#pragma once
#include "Base.h"

NS_BEGIN(Client)
class CYunoFactory final : public CBase
{
public:
	static void Register_States(class CStateMachine* pStateMachineCom, class CYuno* pCharacter);
};
NS_END

