#include "ClientPch.h"
#include "Level_Test_UI.h"

CLevel_Test_UI::CLevel_Test_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CLevel(pDevice, pContext)
{
}

HRESULT CLevel_Test_UI::Initialize()
{




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
    __super::Free();
}
