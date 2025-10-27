#include "ClientPch.h"
#include "Loader_Test_UI.h"


//#include "Dummy.h"

CLoader_Test_UI::CLoader_Test_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader{ pDevice, pContext }
{
}

HRESULT CLoader_Test_UI::Initialize()
{
    CoInitializeEx(nullptr, 0);
    m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
    m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
    m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
    m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });

    return S_OK;
}

HRESULT CLoader_Test_UI::Load_Texture()
{
    cout << "Texture" << endl;



    return S_OK;
}

HRESULT CLoader_Test_UI::Load_Model()
{
    cout << "Model" << endl;

    return S_OK;
}

HRESULT CLoader_Test_UI::Load_Shader()
{
    cout << "Shader" << endl;

    return S_OK;
}

HRESULT CLoader_Test_UI::Load_Object()
{
    cout << "Object" << endl;

    return S_OK;
}

CLoader_Test_UI* CLoader_Test_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLoader_Test_UI* pInstance = new CLoader_Test_UI(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Loader_Test_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader_Test_UI::Free()
{
    __super::Free();
}
