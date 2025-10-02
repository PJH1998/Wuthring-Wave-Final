#include "ClientPch.h"
#include "Loader_Logo.h"

CLoader_Logo::CLoader_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_Logo::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

HRESULT CLoader_Logo::Loading()
{
    EnterCriticalSection(&m_CriticalSection);

    CoInitializeEx(nullptr, 0);

    if (FAILED(Load_Texture()))
        return E_FAIL;

    if (FAILED(Load_Model()))
        return E_FAIL;

    if (FAILED(Load_Shader()))
        return E_FAIL;

    if (FAILED(Load_Object()))
        return E_FAIL;

    SetWindowText(g_hWnd, TEXT("Loading ¿Ï·á"));

    LeaveCriticalSection(&m_CriticalSection);

    return S_OK;
}

HRESULT CLoader_Logo::Load_Texture()
{
    return S_OK;
}

HRESULT CLoader_Logo::Load_Model()
{
    return S_OK;
}

HRESULT CLoader_Logo::Load_Shader()
{
    return S_OK;
}

HRESULT CLoader_Logo::Load_Object()
{
    return S_OK;
}

CLoader_Logo* CLoader_Logo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLoader_Logo* pInstance = new CLoader_Logo(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Loader_Logo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader_Logo::Free()
{
    __super::Free();
}
