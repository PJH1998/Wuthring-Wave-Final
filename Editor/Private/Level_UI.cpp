#include "EditorPch.h"
#include "Level_UI.h"

#include "Event_Level.h"

CLevel_UI::CLevel_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_UI::Initialize()
{
    return S_OK;
}

void CLevel_UI::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("UI"));
}

void CLevel_UI::Render()
{

}

CLevel_UI* CLevel_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_UI* pInstance = new CLevel_UI(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_UI::Free()
{
    __super::Free();

}
