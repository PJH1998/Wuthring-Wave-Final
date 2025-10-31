#include "ClientPch.h"
#include "Loader_GamePlay.h"
#include"GameSystem.h"
#include"MapObject.h"

#pragma region FALSE_SOVEREIGN
#include "MonsterTest.h"
#pragma endregion

CLoader_GamePlay::CLoader_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_GamePlay::Initialize()
{
	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });
	//m_pGameInstance->Add_Work([this]() {Load_MonsterTest(); Complete_Load(); });

	m_pGameInstance->Add_Work([this]() {Load_Augusta(); Complete_Load(); });
	//m_pGameInstance->Add_Work([this]() {Load_Rover(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Player(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_MonsterTest(); Complete_Load(); });

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Texture()
{
	cout << "Texture" << endl;

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Model()
{
	m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/Asphodel_Barrens_1030_second_final/", m_eCurLevel);
	//m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/The_False_Sovereign_1031_Final/", m_eCurLevel);
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

	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject"),
		CMapObject::Create(m_pDevice, m_pContext));

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Player()
{
	return S_OK;
}

HRESULT CLoader_GamePlay::Load_Augusta()
{
	return S_OK;
}

HRESULT CLoader_GamePlay::Load_Rover()
{
	return S_OK;
}

HRESULT CLoader_GamePlay::Load_MonsterTest()
{
	//cout << "MonsterTest" << endl;

	// Prototype_Component_BehaviorTree_Test
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_BehaviorTree_FalseSovereign"),
		CBehavior_Tree::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/FalseSovereign/FalseSovereign_BT.json"))))
		CRASH("BehaviorTree Create Failed");

	// Prototype_Component_AnimMachine_Test
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_AnimMachine_FalseSovereign"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/FalseSovereign/Animation/FalseSovereign_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_Component_Model_FalseSovereign
	//_fmatrix PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	_fmatrix PreTransformMatrix = XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_FalseSovereign"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/FalseSovereign/FalseSovereignTest.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_GameObject_MonsterTest
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_MonsterTest"),
		CMonsterTest::Create(m_pDevice, m_pContext))))
		CRASH("MonsterTest Prototype Create Failed");
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
