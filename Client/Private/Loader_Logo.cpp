#include "ClientPch.h"
#include "Loader_Logo.h"

#include "Dummy.h"

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

	m_fProgress = 100.f;

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
	_fmatrix PreMatrix = XMMatrixScaling(0.001f, 0.001f, 0.001f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_Component_Model_Dummy"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreMatrix, "../Bin/Resource/Dummy/Wolf/Wolf.dat"))))
		return E_FAIL;

    return S_OK;
}

HRESULT CLoader_Logo::Load_Shader()
{
    return S_OK;
}

HRESULT CLoader_Logo::Load_Object()
{
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_GameObject_Dummy"),
		CDummy::Create(m_pDevice, m_pContext))))
		return E_FAIL;

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
