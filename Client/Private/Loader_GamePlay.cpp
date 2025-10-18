#include "ClientPch.h"
#include "Loader_GamePlay.h"

CLoader_GamePlay::CLoader_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_GamePlay::Initialize()
{
	CoInitializeEx(nullptr, 0);
	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Texture()
{
	cout << "Texture" << endl;

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Model()
{
	cout << "Model" << endl;

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Shader()
{
	cout << "Shader" << endl;

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Object()
{
	cout << "Object" << endl;

    return S_OK;
}

CLoader_GamePlay* CLoader_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLoader_GamePlay* pInstance = new CLoader_GamePlay(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Loader_GamePlay");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader_GamePlay::Free()
{
    __super::Free();
}
