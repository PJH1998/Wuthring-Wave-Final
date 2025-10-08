#include "EditorPch.h"
#include "Level_Map.h"

#include "Event_Level.h"

CLevel_Map::CLevel_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Map::Initialize()
{
    return S_OK;
}

void CLevel_Map::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Map"));
}

void CLevel_Map::Render()
{

}

CLevel_Map* CLevel_Map::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Map* pInstance = new CLevel_Map(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Map");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Map::Free()
{
    __super::Free();

}
