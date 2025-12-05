#include "ClientPch.h"
#include "SequenceLupaFactory.h"
#include "SequenceLupa.h"

#include "SequenceLupaGroundSkill.h"

void CSequenceLupaFactory::Register_States(CStateMachine* pStateMachineCom, CSequenceLupa* pCharacter)
{
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND)
		, ENUM_CLASS(ESequenceLupaGroundState::SKILL), CSequenceLupaGroundSkill::Create(pCharacter));
}
