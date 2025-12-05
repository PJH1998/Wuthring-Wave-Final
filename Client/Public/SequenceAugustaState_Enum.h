#pragma once
#include "Client_Define.h"
#include "StateCategory_Enum.h"

NS_BEGIN(Client)

#pragma region DEPTH1
enum class ESequenceAugustaGroundState : _uint
{
	SKILL = 0,
	GROUND_END
};


#pragma endregion

#pragma region DEPTH2
enum class ESequenceAugustaSkillType : _uint
{
	ATTACK_PULL = 0,
	ATTACK_SPEEDDRIVE,
	ATTACK_SPSKILL,
	END
};



#pragma endregion





NS_END