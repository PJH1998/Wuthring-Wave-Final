#include "ClientPch.h"
#include "SequenceAugustaFactory.h"
#include "SequenceAugusta.h"

#include "SequenceAugustaGroundSkill.h"

void CSequenceAugustaFactory::Register_States(CStateMachine* pStateMachineCom, CSequenceAugusta* pCharacter)
{
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND)
		, ENUM_CLASS(ESequenceAugustaGroundState::SKILL), CSequenceAugustaGroundSkill::Create(pCharacter));
}
