#include "ClientPch.h"
#include "RoverFactory.h"
#include "Rover.h"

// HSM Ground 카테고리 State들
#include "RoverGroundIdle.h"
#include "RoverGroundRun.h"
#include "RoverGroundLand.h"
#include "RoverGroundDash.h"
#include "RoverGroundAttack.h"
#include "RoverGroundBurst.h"
#include "RoverGroundSpecial.h"
#include "RoverGroundSkill.h"
#include "RoverGroundQTE.h"

// Air 카테고리 State들
#include "RoverAirFall.h"
#include "RoverAirJump.h"
#include "RoverAirAttack.h"
#include "RoverAirFly.h"

// Hit 카테고리 State
#include "RoverHit.h"


void CRoverFactory::Register_States(CStateMachine* pStateMachineCom, CRover* pCharacter)
{
	// === HSM enum 기반 State 등록 ===
   // enum 값을 index로 사용하여 타입 안정성 확보

   // Ground 카테고리 하위 State들
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE), CRoverGroundIdle::Create(pCharacter));
	//pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::WALK), CRoverGroundWalk::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN), CRoverGroundRun::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::LAND), CRoverGroundLand::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::DASH), CRoverGroundDash::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::ATTACK), CRoverGroundAttack::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::BURST), CRoverGroundBurst::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::SKILL), CRoverGroundSkill::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::SPECIAL), CRoverGroundSpecial::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::QTE), CRoverGroundQTE::Create(pCharacter));

	// Air 하위 State들
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP), CRoverAirJump::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL), CRoverAirFall::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::AIR_ATTACK), CRoverAirAttack::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FLY), CRoverAirFly::Create(pCharacter));
	//pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::AIR_SKILL), CRoverAirSkill::Create(pCharacter));

	// Climb 하위 State들
	//pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ERoverClimbState::CLIMB_MOVE), CRoverClimbMove::Create(pCharacter));
	//pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ERoverClimbState::CLIMB_EXIT), CRoverClimbExit::Create(pCharacter));
	//
	//// Hit 하위 State
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(ERoverHitState::HIT), CRoverHit::Create(pCharacter));
}
