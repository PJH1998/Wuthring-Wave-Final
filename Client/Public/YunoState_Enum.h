#pragma once
#include "Client_Define.h"
#include "StateCategory_Enum.h"

NS_BEGIN(Client)

#pragma region DEPTH1
enum class EYunoGroundState : _uint
{
	IDLE = 0,
	GROUND_END
};

enum class EYunoAirState : _uint
{
	AIR_ATTACK = 0,
	AIR_END
};
#pragma endregion

#pragma region DEPTH2
enum class EYunoIdleType : _uint
{
	STAND1_ACTION01 = 0,
	STAND1_ACTION02,
	STAND1_ACTION03,
	STAND2,
	STANDCHANGE,
	END
};

enum class EYunoAirAttackType : _uint
{
	AIRATTACK_END = 0,
	AIRATTACK_LOOP,
	AIRATTACK_START,
	END
};

#pragma endregion





NS_END