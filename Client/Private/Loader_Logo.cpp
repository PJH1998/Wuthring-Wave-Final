#include "ClientPch.h"
#include "Loader_Logo.h"

#include "Dummy.h"

CLoader_Logo::CLoader_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_Logo::Initialize()
{
	CoInitializeEx(nullptr, 0);
	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });

    return S_OK;
}

HRESULT CLoader_Logo::Load_Texture()
{
	cout << "Texture" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_Model()
{
	_fmatrix PreMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_Component_Model_Augusta"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreMatrix, "../Bin/Resource/Player/Augusta/Aogusta.dat"))))
		return E_FAIL;

	cout << "Model" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_Shader()
{
	cout << "Shader" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_Object()
{
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_GameObject_Dummy"),
		CDummy::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	cout << "Object" << endl;

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
