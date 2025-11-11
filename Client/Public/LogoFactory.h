#pragma once
#include "Base.h"
#include "Client_Define.h"

NS_BEGIN(Client)
class CLogoFactory final : public CBase
{
public:
	static void Register_LogoMaleStates(class CStateMachine* pStateMachineCom, class CLogoMaleRover* pCharacter);
	
};
NS_END

