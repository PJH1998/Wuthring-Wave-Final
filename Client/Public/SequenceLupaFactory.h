#pragma once
#include "Base.h"
NS_BEGIN(Client)
class CSequenceLupaFactory final : public CBase
{
public:
	static void Register_States(class CStateMachine* pStateMachineCom, class CSequenceLupa* pCharacter);
};
NS_END

