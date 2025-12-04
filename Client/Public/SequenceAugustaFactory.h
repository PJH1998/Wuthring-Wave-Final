#pragma once
#include "Base.h"
NS_BEGIN(Client)
class CSequenceAugustaFactory final : public CBase
{
public:
	static void Register_States(class CStateMachine* pStateMachineCom, class CSequenceAugusta* pCharacter);
};
NS_END

