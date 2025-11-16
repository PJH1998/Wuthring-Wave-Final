#include "ClientPch.h"
#include "GalbrenaFactory.h"
#include "Galbrena.h"

// HSM Ground 카테고리 State들
#include "GalbrenaGroundIdle.h"
#include "GalbrenaGroundRun.h"
#include "GalbrenaGroundSprint.h"
#include "GalbrenaGroundLand.h"
#include "GalbrenaGroundDash.h"
//#include "GalbrenaGroundAttack.h"
//#include "GalbrenaGroundBurst.h"
//#include "GalbrenaGroundSpecial.h"
//#include "GalbrenaGroundSkill.h"
//#include "GalbrenaGroundQTE.h"
#include "GalbrenaGroundDodge.h"

// Air 카테고리 State들
#include "GalbrenaAirFall.h"
#include "GalbrenaAirJump.h"
//#include "GalbrenaAirAttack.h"
#include "GalbrenaAirFly.h"

// Hit 카테고리 State
#include "GalbrenaHit.h"

void CGalbrenaFactory::Register_States(CStateMachine* pStateMachineCom, CGalbrena* pCharacter)
{
   // enum 기반 State 등록
   // enum 값을 index로 사용하여 타입 안정성 확보
   
   // Ground 카테고리 하위 State들
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE), CGalbrenaGroundIdle::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN), CGalbrenaGroundRun::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::SPRINT), CGalbrenaGroundSprint::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::LAND), CGalbrenaGroundLand::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::DASH), CGalbrenaGroundDash::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::DODGE), CGalbrenaGroundDodge::Create(pCharacter));


	// Air 카테고리 하위 State들
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP), CGalbrenaAirJump::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL), CGalbrenaAirFall::Create(pCharacter));
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FLY), CGalbrenaAirFly::Create(pCharacter));

	// Hit 카테고리 하위 State들
	pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EGalbrenaHitState::HIT), CGalbrenaHit::Create(pCharacter));
}
