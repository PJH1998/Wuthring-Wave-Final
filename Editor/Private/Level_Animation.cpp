#include "EditorPch.h"
#include "Level_Animation.h"

#include "Event_Level.h"

CLevel_Animation::CLevel_Animation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Animation::Initialize()
{
    return S_OK;
}

void CLevel_Animation::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Anim"));

}

void CLevel_Animation::Render()
{

}

CLevel_Animation* CLevel_Animation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Animation* pInstance = new CLevel_Animation(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Animation");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Animation::Free()
{
    __super::Free();

}
