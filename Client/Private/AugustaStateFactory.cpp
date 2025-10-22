#include "ClientPch.h"
#include "AugustaStateFactory.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "InputController.h"

// HSM Ground 카테고리 State들
#include "AugustaGroundIdle.h"
#include "AugustaGroundWalk.h"
#include "AugustaGroundRun.h"
#include "AugustaGroundSprint.h"
#include "AugustaGroundAttack.h"
#include "AugustaGroundSkill.h"

// HSM State Enum
#include "AugustaState_Enum.h"


void CAugustaStateFactory::Register_AugustaStates(CStateMachine* pStateMachineCom, CAugusta* pPlayer)
{
    // === HSM enum 기반 State 등록 ===
    // enum 값을 index로 사용하여 타입 안정성 확보

    // Ground 카테고리 하위 State들
    pStateMachineCom->Add_State(static_cast<_uint>(EStateCategory::GROUND), static_cast<_uint>(EAugustaGroundState::IDLE), CAugustaGroundIdle::Create(pPlayer));
    pStateMachineCom->Add_State(static_cast<_uint>(EStateCategory::GROUND), static_cast<_uint>(EAugustaGroundState::WALK), CAugustaGroundWalk::Create(pPlayer));
    pStateMachineCom->Add_State(static_cast<_uint>(EStateCategory::GROUND), static_cast<_uint>(EAugustaGroundState::RUN), CAugustaGroundRun::Create(pPlayer));
    pStateMachineCom->Add_State(static_cast<_uint>(EStateCategory::GROUND), static_cast<_uint>(EAugustaGroundState::SPRINT), CAugustaGroundSprint::Create(pPlayer));
    pStateMachineCom->Add_State(static_cast<_uint>(EStateCategory::GROUND), static_cast<_uint>(EAugustaGroundState::ATTACK), CAugustaGroundAttack::Create(pPlayer));
    pStateMachineCom->Add_State(static_cast<_uint>(EStateCategory::GROUND), static_cast<_uint>(EAugustaGroundState::SKILL), CAugustaGroundSkill::Create(pPlayer, "Hack"));

    // TODO: Air, Climb, Hit 카테고리 State 추가
}

void CAugustaStateFactory::Register_KeyInputs(CInputController* pInputControllerCom, CAugusta* pPlayer)
{
    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::W), DIK_W);
    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::A), DIK_A);
    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::S), DIK_S);
    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::D), DIK_D);
    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::SPACE), DIK_SPACE);
    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::Q), DIK_Q);
    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::E), DIK_E);
    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::R), DIK_R);
    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::T), DIK_T);
    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::LSHIFT), DIK_LSHIFT);

    // 마우스 키입력 등록
    pInputControllerCom->Register_MouseKeyInput(ENUM_CLASS(KEYINPUT::LB), MOUSEKEYSTATE::LB);
    pInputControllerCom->Register_MouseKeyInput(ENUM_CLASS(KEYINPUT::WB), MOUSEKEYSTATE::WB);
    pInputControllerCom->Register_MouseKeyInput(ENUM_CLASS(KEYINPUT::RB), MOUSEKEYSTATE::RB);
}
