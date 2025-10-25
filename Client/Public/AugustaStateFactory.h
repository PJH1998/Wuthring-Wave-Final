#pragma once
#include "Character.h"
NS_BEGIN(Client)
class CAugustaStateFactory : public CBase
{
public:
	static void Register_States(class CStateMachine* pStateMachineCom, class CAugusta* pPlayer);
	static void Register_Camera(LEVEL ePrototypeLevel, LEVEL eLevel, class CAugusta* pPlayer, class CGameInstance* pGameInstance, class CSpringCamera** ppCamera);
	
};
NS_END

