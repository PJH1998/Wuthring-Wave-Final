#include "EditorPch.h"
#include "Level_Edit.h"

#include "Event_Level.h"

CLevel_Edit::CLevel_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Edit::Initialize()
{
    return S_OK;
}

void CLevel_Edit::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Main"));
}

void CLevel_Edit::Render()
{

}

CLevel_Edit* CLevel_Edit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Edit* pInstance = new CLevel_Edit(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Edit");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Edit::Free()
{
    __super::Free();

}
