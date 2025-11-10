#include "ClientPch.h"
#include "Loader_Logo.h"

#include "Dummy.h"
#include "ShadowDummy.h"
#include"Parser.h"
#include "GameSystem.h"

CLoader_Logo::CLoader_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_Logo::Initialize()
{
	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_MonsterTable(); Complete_Load(); });

	//m_pGameInstance->Wait_Thread_End();
    return S_OK;
}

HRESULT CLoader_Logo::Load_Texture()
{
	cout << "Texture" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_Model()
{
	//m_pParser->Ready_Prototype_Map(m_pDevice, m_pContext, "../Bin/Resource/Map/MapData/Client_Test3_NonInteraction.dat", LEVEL::LOGO);
	_fmatrix PreMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_Component_Model_Augusta"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreMatrix, "../Bin/Resource/Model/Player/Augusta/Augusta.dat"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_Component_Model_Wolf"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreMatrix, "../Bin/Resource/Dummy/Wolf/Wolf.dat"))))
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

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_GameObject_ShadowDummy"),
		CShadowDummy::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	cout << "Object" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_MonsterTable()
{
	if (FAILED(CGameSystem::GetInstance()->LoadMonsterTable("../Bin/Resource/Data/MonsterTable.csv")))
		return E_FAIL;

	cout << "Monster Table" << endl;

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
