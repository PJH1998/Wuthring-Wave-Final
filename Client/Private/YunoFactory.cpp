#include "ClientPch.h"
#include "YunoFactory.h"
#include "Yuno.h"

#include "YunoGroundIdle.h"
#include "YunoAirAttack.h"


void CYunoFactory::Register_States(CStateMachine* pStateMachineCom, CYuno* pCharacter)
{
	// Ground State들.
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EYunoGroundState::IDLE), CYunoGroundIdle::Create(pCharacter));

	// Air State들
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EYunoAirState::AIR_ATTACK), CYunoAirAttack::Create(pCharacter));
}
