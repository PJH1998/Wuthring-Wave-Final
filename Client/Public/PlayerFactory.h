#pragma once
#include "Base.h"
NS_BEGIN(Client)
class CPlayerFactory : public CBase
{
public:
	static void Register_KeyInputs(class CInputController* pInputControllerCom, class CPlayer* pPlayer);
	static void Register_Camera(LEVEL ePrototypeLevel, LEVEL eLevel, class CPlayer* pPlayer, class CGameInstance* pGameInstance, class CSpringCamera** ppCamera);
};
NS_END
