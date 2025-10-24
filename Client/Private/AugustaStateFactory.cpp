#include "ClientPch.h"
#include "AugustaStateFactory.h"
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
#include "AugustaGroundSprint.h"
#include "AugustaGroundAttack.h"
#include "AugustaGroundSkill.h"

// Air 카테고리 State들
#include "AugustaAirJump.h"
#include "AugustaAirFall.h"


void CAugustaStateFactory::Register_States(CStateMachine* pStateMachineCom, CAugusta* pPlayer)
{
    // === HSM enum 기반 State 등록 ===
    // enum 값을 index로 사용하여 타입 안정성 확보

    // Ground 카테고리 하위 State들
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE), CAugustaGroundIdle::Create(pPlayer));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::WALK), CAugustaGroundWalk::Create(pPlayer));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN), CAugustaGroundRun::Create(pPlayer));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND), CAugustaGroundLand::Create(pPlayer));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SPRINT), CAugustaGroundSprint::Create(pPlayer));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::ATTACK), CAugustaGroundAttack::Create(pPlayer));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL), CAugustaGroundSkill::Create(pPlayer, "Hack"));

    // Air 하위 State들
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP), CAugustaAirJump::Create(pPlayer));
    pStateMachineCom->Add_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL), CAugustaAirFall::Create(pPlayer));
}

void CAugustaStateFactory::Register_Camera(LEVEL ePrototypeLevel, LEVEL eLevel, class CAugusta* pPlayer, class CGameInstance* pGameInstance, class CSpringCamera** ppCamera)
{
     CSpringCamera::CAMERA_DESC CameraDesc{};
     CameraDesc.fSpeedPerSec = 100.f;
     CameraDesc.fRotationPerSec = XMConvertToRadians(90.f);
     CameraDesc.fFovy = XMConvertToRadians(60.f);
     CameraDesc.fNear = 0.1f;
     CameraDesc.fFar = 5000.f;
     CameraDesc.vEye = _float4(0.f, 200.f, -150.f, 1.f);
     CameraDesc.vAt = _float4(0.f, 0.f, 200.f, 1.f);
     CameraDesc.fMouseSensor = 0.004f;

     CSpringCamera* pSpringCamera = dynamic_cast<CSpringCamera*>(pGameInstance->Clone_Prototype(ENUM_CLASS(ePrototypeLevel)
        , TEXT("Prototype_GameObject_SpringCamera"), PROTOTYPE::GAMEOBJECT
        , &CameraDesc));

     ASSERT_CRASH(pSpringCamera);
    *ppCamera = pSpringCamera;

    // Camera 등록.
    pGameInstance->Add_Camera(ENUM_CLASS(LEVEL::STATIC), TEXT("Camera_Spring"), pSpringCamera);
    Safe_AddRef(pSpringCamera);

    pGameInstance->Change_MainCamera(ENUM_CLASS(LEVEL::STATIC), TEXT("Camera_Spring"));
}



