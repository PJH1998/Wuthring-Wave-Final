#include "ClientPch.h"
#include "Level_Logo.h"

#include "Event_Level.h"

CLevel_Logo::CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Logo::Initialize()
{
	CRigidbody* pRigidBody = CRigidbody::Create(m_pDevice, m_pContext);
	pRigidBody->Initialize_Clone(nullptr);

    return S_OK;
}

void CLevel_Logo::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Logo"));

    //if (m_pGameInstance->Get_DIKeyState(DIK_F1) == KEYSTATE::DOWN)
    //{
    //    CHANGE_LEVEL_EVENT event{ LEVEL::LORD, true };
    //    m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
    //}
}

HRESULT CLevel_Logo::Render()
{
    return S_OK;
}

CLevel_Logo* CLevel_Logo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Logo* pInstance = new CLevel_Logo(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Logo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Logo::Free()
{
    __super::Free();

}
