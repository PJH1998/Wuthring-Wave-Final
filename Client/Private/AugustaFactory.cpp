#include "ClientPch.h"
#include "AugustaFactory.h"
#include "Augusta.h"
#include "StateMachine.h"

#include "SpringCamera.h"

// HSM State Enum
#include "AugustaState_Enum.h"

// HSM Ground 카테고리 State들
#include "AugustaGroundIdle.h"
#include "AugustaGroundWalk.h"
#include "AugustaGroundRun.h"
#include "AugustaGroundLand.h"
#include "AugustaGroundDash.h"
#include "AugustaGroundAttack.h"
#include "AugustaGroundSkill.h"
#include "AugustaGroundBurst.h"
#include "AugustaGroundSpecial.h"
#include "AugustaGroundQTE.h"


// Air 카테고리 State들
#include "AugustaAirJump.h"
#include "AugustaAirFall.h"
#include "AugustaAirAttack.h"
#include "AugustaAirSkill.h"
#include "AugustaAirFly.h"

// Climb State들
#include "AugustaClimbMove.h"
#include "AugustaClimbExit.h"

// Hit State
#include "AugustaHit.h"


void CAugustaFactory::Register_States(CStateMachine* pStateMachineCom, CAugusta* pCharacter)
{
    // === HSM enum 기반 State 등록 ===
    // enum 값을 index로 사용하여 타입 안정성 확보

    // Ground 카테고리 하위 State들
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE), CAugustaGroundIdle::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::WALK), CAugustaGroundWalk::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN), CAugustaGroundRun::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND), CAugustaGroundLand::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::DASH), CAugustaGroundDash::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::ATTACK), CAugustaGroundAttack::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL), CAugustaGroundSkill::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::BURST), CAugustaGroundBurst::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SPECIAL), CAugustaGroundSpecial::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::QTE), CAugustaGroundQTE::Create(pCharacter));

    // Air 하위 State들
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP), CAugustaAirJump::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL), CAugustaAirFall::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::AIR_ATTACK), CAugustaAirAttack::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::AIR_SKILL), CAugustaAirSkill::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FLY), CAugustaAirFly::Create(pCharacter));

    // Climb 하위 State들
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_MOVE), CAugustaClimbMove::Create(pCharacter));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_EXIT), CAugustaClimbExit::Create(pCharacter));

    // Hit 하위 State
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EAugustaHitState::HIT), CAugustaHit::Create(pCharacter));
}



