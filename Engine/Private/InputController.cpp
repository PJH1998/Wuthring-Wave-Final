#include "EnginePch.h"
#include "InputController.h"
#include "GameInstance.h"

CInputController::CInputController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{ pDevice, pContext}
{
}

CInputController::CInputController(const CInputController& Prototype)
    : CComponent (Prototype)
{
}

HRESULT CInputController::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CInputController::Initialize_Clone(void* pArg)
{
    return S_OK;
}

void CInputController::Update()
{
    // 1. Update Key Input
    Update_KeyInput();
}

void CInputController::Update_KeyInput()
{
    m_PrevKeyInput = m_KeyInput;

    // 1. ���� ������ Ű ���� �ʱ�ȭ
    m_KeyInput = 0;
    KEYSTATE eState = { KEYSTATE::END };

    // 2. Ű���� �Է� Ȯ��.
    for (_uint i = 0; i < m_KeyboardMappings.size(); ++i)
    {
        const auto& keyMapping = m_KeyboardMappings[i];
        eState = m_pGameInstance->Get_DIKeyState(keyMapping.second);

        if (eState == KEYSTATE::PRESS || eState == KEYSTATE::DOWN)
            m_KeyInput |= keyMapping.first;
    }

    // 3. 
    for (_uint i = 0; i < m_MouseMappings.size(); ++i)
    {
        const auto& keyMapping = m_MouseMappings[i];
        eState = m_pGameInstance->Get_DIMouseState(keyMapping.second);
        if (eState == KEYSTATE::PRESS || eState == KEYSTATE::DOWN)
            m_KeyInput |= keyMapping.first;
    }

}

_bool CInputController::Check_AnyInput(_uint eKeyInput, KEYSTATE eState)
{
    _uint iFlag = static_cast<_uint>(eKeyInput);
    if (eState == KEYSTATE::PRESS)
        return (m_KeyInput & iFlag) != 0;  // ���� ����

    if (eState == KEYSTATE::DOWN)
        return !(m_PrevKeyInput & iFlag) && (m_KeyInput & iFlag);  // ����X, ����O

    if (eState == KEYSTATE::UP)
        return (m_PrevKeyInput & iFlag) && !(m_KeyInput & iFlag);  // ����O, ����X

    return false;
}

_bool CInputController::Check_AllInput(_uint eKeyInput, KEYSTATE eState)
{
    _uint iFlag = static_cast<_uint>(eKeyInput);

    if (eState == KEYSTATE::PRESS)
        return (m_KeyInput & iFlag) == iFlag;  // ���� ����

    if (eState == KEYSTATE::DOWN)
        return !((m_PrevKeyInput & iFlag) == iFlag) && ((m_KeyInput & iFlag) == iFlag);  // ����X, ����O

    if (eState == KEYSTATE::UP)
        return ((m_PrevKeyInput & iFlag) == iFlag) && !((m_KeyInput & iFlag) == iFlag);  // ����O, ����X

    return false;
}


/*
* ����
* Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::W), DIK_W));
*/
void CInputController::Register_KeyBoardKeyInput(_uint iKey, _ubyte keyboardValue)
{
    m_KeyboardMappings.emplace_back(make_pair(iKey, keyboardValue));
}

/*
* ����
* Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::LB), MOUSEKEYSTATE::LB));
*/
void CInputController::Register_MouseKeyInput(_uint iKey, MOUSEKEYSTATE mouseValue)
{
    m_MouseMappings.emplace_back(make_pair(iKey, mouseValue));
}

CInputController* CInputController::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CInputController* pInstance = new CInputController(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CInputController");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CInputController::Clone(void* pArg)
{
    CInputController* pInstance = new CInputController(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : CInputController");
        Safe_Release(pInstance);
    }

    return pInstance;
}
