#include "ClientPch.h"
#include "Level_Test_UI.h"

CLevel_Test_UI::CLevel_Test_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CLevel(pDevice, pContext)
{
}

HRESULT CLevel_Test_UI::Initialize()
{
    // HUD ���ӿ�����Ʈ �߰�
    const   _uint       iDestLevel = m_pGameInstance->Get_CurrentLevel();

    const _wstring strLayertag_UI = L"Layer_Custom_UI";
    const _wstring strPrototypeTag_UI[] = {
         L"Prototype_GameObject_Custom_UI_Container_HUD"
    };


    for (auto& strPrototypeTag : strPrototypeTag_UI)
    {
        CUIObject* pTargetUI = static_cast<CUIObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, strPrototypeTag, PROTOTYPE::GAMEOBJECT));
        if (FAILED(m_pGameInstance->Add_RootUI(L"UI_UHD", pTargetUI)))
            CRASH("Failed to Add RootUI to UI_Manager.");
        if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, strLayertag_UI, pTargetUI)))
            CRASH("Failed to Add RootUI to Object_Manager.");
    }

    //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_HUD", iDestLevel, L"Layer_Custom_UI")))
    //    CRASH("Create HUD FAILED.");

    return S_OK;
}

void CLevel_Test_UI::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Test_UI"));
}

void CLevel_Test_UI::Render()
{
}

CLevel_Test_UI* CLevel_Test_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Test_UI* pInstance = new CLevel_Test_UI(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Test_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Test_UI::Free()
{
    m_pGameInstance->Clear_RootUI();

    __super::Free();
}
