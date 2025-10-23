#include "ClientPch.h"
#include "PlayerFactory.h"
#include "Player.h"
#include "InputController.h"

void CPlayerFactory::Register_KeyInputs(CInputController* pInputControllerCom, CPlayer* pPlayer)
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

    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::D1), DIK_1);
    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::D2), DIK_2);
    pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::D3), DIK_3);


    // 마우스 키입력 등록
    pInputControllerCom->Register_MouseKeyInput(ENUM_CLASS(KEYINPUT::LB), MOUSEKEYSTATE::LB);
    pInputControllerCom->Register_MouseKeyInput(ENUM_CLASS(KEYINPUT::WB), MOUSEKEYSTATE::WB);
    pInputControllerCom->Register_MouseKeyInput(ENUM_CLASS(KEYINPUT::RB), MOUSEKEYSTATE::RB);
}
