#include "ClientPch.h"
#include "Loader_GamePlay.h"
#include"GameSystem.h"
#include"MapObject.h"

#pragma region FALSE_SOVEREIGN
#include "MonsterTest.h"
#pragma endregion

#pragma region PLAYER

#include "StateMachine.h"

// Augusta
#include "AugustaBayonet.h"
#include "AugustaSkillWeapon.h"
#include "AugustaGriffon.h"
#include "Augusta.h"

// Rover
#include "RoverSword.h"
#include "Rover.h"

// Player
#include "Player.h"
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
	m_pGameInstance->Add_Work([this]() {Load_Rover(); Complete_Load(); });
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
	// Controller 초기화
	_wstring wstrControllerTag = L"Prototype_Component_PlayerController";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wstrControllerTag,
		CInputController::Create(m_pDevice, m_pContext))))
		CRASH("PlayerInput Controller");

	_wstring wStrControllerTag = TEXT("Prototype_GameObject_Player");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrControllerTag
		, CPlayer::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	return S_OK;
}

HRESULT CLoader_GamePlay::Load_Augusta()
{
	_wstring wStrModelTag = L"Prototype_Component_Model_Augusta";
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/Augusta/Augusta.dat";
	_matrix	PreTransformMatrix = XMMatrixIdentity();
	//_float fSize = 0.01f;
	_float fSize = 0.0001f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(180.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");


	// 2. StateMachine 초기화
	_wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_Augusta";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrStateMachineTag,
		CStateMachine::Create(m_pDevice, m_pContext))))
		CRASH("PlayerState Machine");


	// 3. 객체 초기화
	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_Augusta");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrActorTag
		, CAugusta::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

#pragma region Parts
	wStrModelTag = L"Prototype_Component_Model_Augusta_Bayonet";
	strFilePath = "../../Client/Bin/Resource/Model/Player/Augusta/Weapon/Bayonet/Bayonet.dat";
	fSize = 0.01f;
	//fSize = 0.0001f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationX(XMConvertToRadians(-90.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// 2. 객체 초기화.
	_wstring wstrBayonetTag = TEXT("Prototype_GameObject_Augusta_Bayonet");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wstrBayonetTag
		, CAugustaBayonet::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");


	wStrModelTag = L"Prototype_Component_Model_Augusta_SkillWeapon";
	strFilePath = "../../Client/Bin/Resource/Model/Player/Augusta/Weapon/SkillWeapon/SkillWeapon.dat";
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	_wstring wStrSkillWeaponTag = TEXT("Prototype_GameObject_Augusta_SkillWeapon");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrSkillWeaponTag
		, CAugustaSkillWeapon::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	wStrModelTag = L"Prototype_Component_Model_Augusta_Griffon";
	strFilePath = "../../Client/Bin/Resource/Model/Player/Augusta/Weapon/Griffon/Griffon.dat";
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationX(XMConvertToRadians(-90.f));
	//PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	_wstring wStrGriffonTag = TEXT("Prototype_GameObject_Augusta_Griffon");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrGriffonTag
		, CAugustaGriffon::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");
#pragma endregion

	return S_OK;
}

HRESULT CLoader_GamePlay::Load_Rover()
{
	_wstring wStrModelTag = L"Prototype_Component_Model_Rover";
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/Rover/DarkRover.dat";
	_matrix	PreTransformMatrix = XMMatrixIdentity();
	//_float fSize = 0.01f;
	_float fSize = 0.0001f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(180.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");


	// 2. StateMachine 초기화
	_wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_Rover";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrStateMachineTag,
		CStateMachine::Create(m_pDevice, m_pContext))))
		CRASH("PlayerState Machine");


	// 3. 객체 초기화
	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_Rover");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrActorTag
		, CRover::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");


#pragma region Parts
	wStrModelTag = L"Prototype_Component_Model_Rover_Sword";
	strFilePath = "../../Client/Bin/Resource/Model/Player/Rover/Weapon/Sword/Sword.dat";
	fSize = 0.01f;
	//fSize = 0.0001f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationX(XMConvertToRadians(-90.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// 2. 객체 초기화.
	_wstring wstrBayonetTag = TEXT("Prototype_GameObject_Rover_Sword");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wstrBayonetTag
		, CRoverSword::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

#pragma endregion

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
