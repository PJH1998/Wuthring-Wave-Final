#include "ClientPch.h"
#include "Loader_GamePlay.h"
#include"GameSystem.h"

#pragma region MAP
#include"MapObject.h"
#include"Trigger_Box.h"
#include"MapObject_Destruction.h"
#include"MapObject_Destruction_Debris.h"
#include"MapObject_NonSonoro.h"
#include"MapObject_Sonoro.h"
#include"MapObject_Instance.h"
#include"MapObject_Meteo.h"
#pragma endregion

#pragma region MONSTER
#include "MonsterTest.h"
#include "Ggobul.h"
#include "FS_Scythe.h"
#include "AttackVolume.h"
#include "Spawner.h"
#include "HavocWarrior.h"
#include "ElectroPredator.h"
#include "AoEDoT.h"
#include "Projectile.h"
#include "Corosaurus.h"
#pragma endregion

#pragma region UI
#include "Custom_UI.h"
#include "UI_Button.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "Animator_UI.h"
#include "UI_HUD.h"
#include "UI_Text_Damage.h"
#include "UI_Button_Interact.h"
#include "UI_LockOn.h"
#include "UI_Parry.h"
#include "UI_MobHPBar.h"
#include "UI_TabUtility.h"

#include "Mouse.h"
#pragma endregion


#pragma region PLAYER
#include "Wing.h"
#include "StateMachine.h"

// Augusta
#include "AugustaBayonet.h"
#include "AugustaSkillWeapon.h"
#include "AugustaGriffon.h"
#include "Augusta.h"

// Rover
#include "RoverSword.h"
#include "RoverDarkWing.h"
#include "RoverDarkScythe.h"
#include "Rover.h"

// Galbrena
#include "Galbrena.h"
#include "GalbrenaShotGun.h"
#include "GalbrenaDarkWing.h"

// Player
#include "Player.h"
#pragma endregion

#pragma region SFX
#include "SFX_Prefab.h"
#include "SonoraChange.h"
#include "Augusta_UltiSFX.h"
#include "Augusta_UltiPostSFX.h"
#include "GalbrenaUlti_SFX_Slash.h"
#include "GalbrenaUlti_SFX_Star.h"
#include "GalbrenaUlti_SFX_Circle.h"
#include "GalbrenaUlti_PostSFX.h"
#pragma endregion


CLoader_GamePlay::CLoader_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_GamePlay::Initialize()
{
	m_iNumLoadingThread = 13;
	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });
	//m_pGameInstance->Add_Work([this]() {Load_MonsterTest(); Complete_Load(); });

	m_pGameInstance->Add_Work([this]() {Load_Augusta(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Rover(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Galbrena(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Player(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_MonsterTest(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Monster(); Complete_Load(); });
	
	m_pGameInstance->Add_Work([this]() {Load_Effect(); Complete_Load(); });

	m_pGameInstance->Add_Work([this]() {Load_UI(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Font(); Complete_Load(); });

	m_pGameSystem->Add_Action("../Bin/Resource/Sequence/Action/");

	Load_ScreenEffect();

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Texture()
{
	cout << "Texture" << endl;

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Model()
{
	// Map Load
	m_pGameInstance->Load_Resource("../Bin/Resource/Map/Asphodel_Barrens/Textures/");
	m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/Asphodel_Barrens_1114_first/", m_eCurLevel, "Asphodel_Barrens");

	m_pGameInstance->Load_Resource("../Bin/Resource/Map/The_False_Sovereign/Textures/");
	m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/The_False_Soerveign_1114_first/", m_eCurLevel, "The_False_Sovereign");

	//m_pGameInstance->Load_Resource("../Bin/Resource/Map/Heaven/");
	//m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/Heaven/", m_eCurLevel);

	// SkyBox
	_matrix PreTransformMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Skybox_Dome"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../Bin/Resource/Skybox/SkyDome.dat"))))
		CRASH("SkyDome");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Skybox_Background"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../Bin/Resource/Skybox/SkyBackground_Gameplay.dat"))))
		CRASH("SkyBackground");
	PreTransformMatrix = XMMatrixScaling(0.05f, 0.05f, 0.05f) * XMMatrixRotationRollPitchYaw(0.f, XMConvertToRadians(90.f), 0.f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Skybox_FX"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../Bin/Resource/Skybox/SkyFX_Gameplay.dat"))))
		CRASH("SkyFX");
	//PreTransformMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Skybox_Cloud"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../Bin/Resource/Skybox/SkyCloud_Gameplay.dat"))))
		CRASH("SkyCloud");

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

#pragma region MAP
	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject"),
		CMapObject::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_Destruction"),
		CMapObject_Destruction::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_Destruction_Debris"),
		CMapObject_Destruction_Debris::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_TriggerBox"),
		CTrigger_Box::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_Sonoro"),
		CMapObject_Sonoro::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_NonSonoro"),
		CMapObject_NonSonoro::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_Instance"),
		CMapObject_Instance::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_Meteo"),
		CMapObject_Meteo::Create(m_pDevice, m_pContext));
	
	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Spawner"),
		CSpawner::Create(m_pDevice, m_pContext));
#pragma endregion
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


#pragma region COMMON 객체 WING
	_wstring wStrModelTag = L"Prototype_Component_Model_Wing";
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/Wing/Wing.dat";
	_float fSize = 0.01f;
	//fSize = 0.0001f;
	_matrix PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationX(XMConvertToRadians(-90.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// 2. 객체 초기화.
	_wstring wstrBayonetTag = TEXT("Prototype_GameObject_Wing");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wstrBayonetTag
		, CWing::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");
#pragma endregion

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
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/Rover/Rover.dat";
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

	wStrModelTag = L"Prototype_Component_Model_Rover_DarkWing";
	strFilePath = "../../Client/Bin/Resource/Model/Player/Rover/Weapon/DarkWing/DarkRoverWing.dat";
	fSize = 0.01f;
	//fSize = 0.0001f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationX(XMConvertToRadians(-90.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// 2. 객체 초기화.
	_wstring wstrDrakWingTag = TEXT("Prototype_GameObject_Rover_DarkWing");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wstrDrakWingTag
		, CRoverDarkWing::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	wStrModelTag = L"Prototype_Component_Model_Rover_DarkScythe";
	strFilePath = "../../Client/Bin/Resource/Model/Player/Rover/Weapon/DarkScythe/DarkScythe.dat";
	fSize = 0.01f;
	//fSize = 0.0001f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationX(XMConvertToRadians(-90.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// 2. 객체 초기화.
	_wstring wstrDarkScytheTag = TEXT("Prototype_GameObject_Rover_DarkScythe");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wstrDarkScytheTag
		, CRoverDarkScythe::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");
#pragma endregion

	return S_OK;
}

HRESULT CLoader_GamePlay::Load_Galbrena()
{
	_wstring wStrModelTag = L"Prototype_Component_Model_Galbrena";
	//_string strFilePath = "../../Client/Bin/Resource/Model/Player/Galbrena/Galbrena.dat";
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/GalbrenaFacial/Galbrena.dat";
	_matrix	PreTransformMatrix = XMMatrixIdentity();

	// Editor에서 isCharacter AnimationActor 생성과 동일하게.
	_float fSize = 0.01f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(180.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::CHARACTER, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");


	// 2. StateMachine 초기화
	_wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_Galbrena";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrStateMachineTag,
		CStateMachine::Create(m_pDevice, m_pContext))))
		CRASH("PlayerState Machine");


	// 3. 객체 초기화
	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_Galbrena");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrActorTag
		, CGalbrena::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");


#pragma region Parts
	wStrModelTag = L"Prototype_Component_Model_Galbrena_ShotGun";
	strFilePath = "../../Client/Bin/Resource/Model/Player/Galbrena/Weapon/ShotGun/ShotGun.dat";
	fSize = 0.01f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationX(XMConvertToRadians(-90.f));
	//PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// DarkWing
	wStrModelTag = L"Prototype_Component_Model_Galbrena_DarkWing";
	strFilePath = "../../Client/Bin/Resource/Model/Player/Galbrena/Weapon/DarkWing/DarkWing.dat";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// 2. 객체 초기화.
	_wstring wstrObjectTag = TEXT("Prototype_GameObject_Galbrena_FirstGun");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wstrObjectTag
		, CGalbrenaShotGun::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");


	wstrObjectTag = TEXT("Prototype_GameObject_Galbrena_SecondGun");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wstrObjectTag
		, CGalbrenaShotGun::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");


	wstrObjectTag = TEXT("Prototype_GameObject_Galbrena_DarkWing");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wstrObjectTag
		, CGalbrenaDarkWing::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

#pragma endregion
	return S_OK;
}

HRESULT CLoader_GamePlay::Load_UI()
{
	const   _uint       iDestLevel = ENUM_CLASS(LEVEL::GAMEPLAY);


	// ==============================
	cout << "[CLoader_Test_UI] Texture" << endl;
	// ==============================

	vector<CCustom_UI::CUSTOM_UITREE_DESC> vecDescs = {};       // parsed data from json

	// * Json Parse                 // for pre-loading textures
	// UI_HUD
	//_string strFilePath_UI_HUD = "../../Client/Bin/Resource/UI/FJson/UITree/TestHUD.json"; // ksta
	_string strFilePath_UI_HUD = "../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD_251030_2037.json"; // ksta
	vecDescs.push_back(Load_UITree(strFilePath_UI_HUD));

	_string strFilePath_UI_Interact = "../../Client/Bin/Resource/UI/FJson/UITree/Root_Interact.json"; // ksta
	vecDescs.push_back(Load_UITree(strFilePath_UI_Interact));

	_string strFilePath_UI_LockOn = "../../Client/Bin/Resource/UI/FJson/UITree/Root_LockOn.json"; // ksta
	vecDescs.push_back(Load_UITree(strFilePath_UI_LockOn));

	_string strFilePath_UI_Parry = "../../Client/Bin/Resource/UI/FJson/UITree/Root_Parry.json"; // ksta
	vecDescs.push_back(Load_UITree(strFilePath_UI_Parry));

	_string strFilePath_UI_MobHP = "../../Client/Bin/Resource/UI/FJson/UITree/Root_MobHPBarDynamic.json"; // ksta
	vecDescs.push_back(Load_UITree(strFilePath_UI_MobHP));

	_string strFilePath_UI_TabUtility = "../../Client/Bin/Resource/UI/FJson/UITree/Root_TabUtility.json"; // ksta
	vecDescs.push_back(Load_UITree(strFilePath_UI_TabUtility));


	for (auto& treeDesc : vecDescs)
	{
		for (auto& infoDesc : treeDesc.vecUIInfoDescs)
		{
			const   _wstring    strFilePath = infoDesc.tUIDesc.strFilePath;
			const   _wstring	strFileName = infoDesc.tUIDesc.strFileName;
			const   _uint       iNumFiles = infoDesc.tUIDesc.iNumFiles;

			infoDesc.tUIDesc.strFilePath;
			if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Texture_Custom_") + strFileName,
				CTexture::Create(m_pDevice, m_pContext, strFilePath.c_str(), iNumFiles))))
				OutputDebugString(L"[CLoader_Test_UI::Ready_Prototypes] Texture Load Failed. The texture may have already been loaded.\n");
		}
	}

	// ==============================
	cout << "[CLoader_Test_UI] Model" << endl;
	// ==============================
	// 
	// VIBuffer_Rect
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test_UI::Load_Model] VIBuffer_Rect Load Failed. The VIBuffer_Rect may have already been loaded.\n");

	// VIBuffer_Rect_Instance_UI
	CVIBuffer_Rect_Instance_UI::RECT_INSTANCE_UI_DESC tRectInstDesc = {};
	tRectInstDesc.iNumInstance = 500U;
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect_Instance_UI"),
		CVIBuffer_Rect_Instance_UI::Create(m_pDevice, m_pContext, &tRectInstDesc))))
		OutputDebugString(L"[Loader_Test_UI::Load_Model] VIBuffer_Rect_Instance_UI Load Failed. The VIBuffer_Rect_Instance_UI may have already been loaded.\n");


	// ==============================
	cout << "[CLoader_Test_UI] Shader" << endl;
	// ==============================

	// Shader
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		OutputDebugString(L"[Loader_Test_UI::Load_Shader] Shader Load Failed. The Shader may have already been loaded.\n");

	// Shader_Instance
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxInstance.hlsl"), VTXUIINSTANCE::Elements, VTXUIINSTANCE::iNumElements))))
		OutputDebugString(L"[Loader_Test_UI::Load_Shader] Shader_Instance Load Failed. The Shader_Instance may have already been loaded.\n");

	// Shader_Font
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_Text_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_TextInstance.hlsl"), VTXUIINSTANCE::Elements, VTXUIINSTANCE::iNumElements))))
		OutputDebugString(L"[Loader_Test::Load_Shader] Shader_TextInstance Load Failed. The Shader_TextInstance may have already been loaded.\n");


	// ==============================
	cout << "[CLoader_Test_UI] Object" << endl;
	// ==============================

	// * Components Load
	// Animator_UI
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_Component_Animator_UI",
		CAnimator_UI::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[CCustom_UI::Load_Shader] Animator_UI Load Failed. The Animator_UI may have already been loaded.\n");

	// * Objects Load
	// Custom UI
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Button",
		CUI_Button::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test_UI:Load_Object] UI_Button Load Failed. The CUI_Button may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Image",
		CUI_Image::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test_UI::Load_Object] UI_Image Load Failed. The UI_Image may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Text",
		CUI_Text::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test_UI::Load_Object] UI_Text Load Failed. The CUI_Text may have already been loaded.\n");

	// Custom Text
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Text_Damage",
		CUI_Text_Damage::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_Text_Damage Load Failed. The UI_Text_Damage may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Button_Interact",
		CUI_Button_Interact::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_Button_Interact Load Failed. The UI_Text_Damage may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_LockOn",
		CUI_LockOn::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_LockOn Load Failed. The UI_LockOn may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Parry",
		CUI_Parry::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_Parry Load Failed. The UI_Parry may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_MobHPBar",
		CUI_MobHPBar::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_MobHPBar Load Failed. The UI_MobHPBar may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_TabUtility",
		CUI_TabUtility::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_TabUtility Load Failed. The UI_TabUtility may have already been loaded.\n");

	// ==============================
	cout << "[CLoader_Test_UI][UI Custom] Prototype" << endl;
	// ==============================

	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_HUD",
		CUI_HUD::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test_UI::Load_Prototype] UI_HUD Load Failed. The UI_HUD may have already been loaded.\n");

	// Mouse
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Mouse"),
		CMouse::Create(m_pDevice, m_pContext))))
		CRASH("Mouse");

	return S_OK;
}

HRESULT CLoader_GamePlay::Load_Font()
{
	// ==============================
	cout << "[Loader_Test] Font " << endl;
	// ==============================
	_uint iPixelHeight = 64U;

	if (FAILED(m_pGameInstance->Add_Font(L"WW_Medium", "../../Client/Bin/Resource/Font/Font_SUITE/SUITE-Medium.ttf", iPixelHeight)))
		OutputDebugString(L"[Loader_Test::Load_Font] Font Load Failed. The Font may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Font(L"WW_SemiBold", "../../Client/Bin/Resource/Font/Font_SUITE/SUITE-SemiBold.ttf", iPixelHeight)))
		OutputDebugString(L"[Loader_Test::Load_Font] Font Load Failed. The Font may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Font(L"WW_Bold", "../../Client/Bin/Resource/Font/Font_SUITE/SUITE-Bold.ttf", iPixelHeight)))
		OutputDebugString(L"[Loader_Test::Load_Font] Font Load Failed. The Font may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Font(L"WW_ExtraBold", "../../Client/Bin/Resource/Font/Font_SUITE/SUITE-ExtraBold.ttf", iPixelHeight)))
		OutputDebugString(L"[Loader_Test::Load_Font] Font Load Failed. The Font may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Font(L"WW_Heavy", "../../Client/Bin/Resource/Font/Font_SUITE/SUITE-Heavy.ttf", iPixelHeight)))
		OutputDebugString(L"[Loader_Test::Load_Font] Font Load Failed. The Font may have already been loaded.\n");

	return S_OK;
}

HRESULT CLoader_GamePlay::Load_Effect()
{
	m_pGameSystem->Create_Effect("../../Client/Bin/Resource/Effect/Prefabs/Common", m_eCurLevel);
	m_pGameSystem->Load_EffectTexture_FromFolder("../../Client/Bin/Resource/Effect/Prefabs/Common/Texture", m_eCurLevel);
	m_pGameSystem->Load_EffectMeshDat_FromFolder("../../Client/Bin/Resource/Effect/Prefabs/Common/Dat", m_eCurLevel);

	m_pGameSystem->Create_Effect("../../Client/Bin/Resource/Effect/Prefabs/WeiZuoShenWang", m_eCurLevel);


	return S_OK;
}

HRESULT CLoader_GamePlay::Load_ScreenEffect()
{
#pragma region SFX_TEXTURE
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_SFX_Slash"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Mask_300156.png"), 1))))
		CRASH("Failed Add Prototype SFX_Slash");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_SFX_Noise"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Noise_12001.png"), 1))))
		CRASH("Failed Add Prototype SFX_Noise");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_SFX_Star"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Mask_11000_WP20002.png"), 1))))
		CRASH("Failed Add Prototype SFX_Star");

#pragma endregion

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_SFX_Prefab"),
		CSFX_Prefab::Create(m_pDevice, m_pContext))))
		CRASH("Failed Add Prototype SFX_Prefab");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_SFX_SonoraChange"),
		CSonoraChange::Create(m_pDevice, m_pContext))))
		CRASH("Failed Add Prototype SFX_SonoraChange");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_SFX_Augusta_UltiSFX"),
		CAugusta_UltiSFX::Create(m_pDevice, m_pContext))))
		CRASH("Failed Add Prototype SFX_Augusta_UltiSFX");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_SFX_Augusta_UltiPostSFX"),
		CAugusta_UltiPostSFX::Create(m_pDevice, m_pContext))))
		CRASH("Failed Add Prototype SFX_Augusta_UltiPostSFX");

	if(FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_SFX_Galbrena_UltiSlash"),
		CGalbrenaUlti_SFX_Slash::Create(m_pDevice, m_pContext))))
		CRASH("Failed Add Prototype SFX_Augusta_UltiPostSFX");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_SFX_Galbrena_UltiStar"),
		CGalbrenaUlti_SFX_Star::Create(m_pDevice, m_pContext))))
		CRASH("Failed Add Prototype_SFX_Galbrena_UltiStar");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_SFX_Galbrena_UltiCircle"),
		CGalbrenaUlti_SFX_Circle::Create(m_pDevice, m_pContext))))
		CRASH("Failed Add Prototype SFX_Galbrena_UltiCircle");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_SFX_Galbrena_UltiPostSFX"),
		CGalbrenaUlti_PostSFX::Create(m_pDevice, m_pContext))))
		CRASH("Failed Add Prototype_SFX_Galbrena_UltiPostSFX");

	return S_OK;
}

HRESULT CLoader_GamePlay::Load_MonsterTest()
{
	cout << "MonsterTest" << endl;
		// Prototype_GameObject_AttackVolume
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_AttackVolume"),
		CAttackVolume::Create(m_pDevice, m_pContext))))
		CRASH("AttackVolume Create Failed");

	// Prototype_Component_BehaviorTree_Test
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_BehaviorTree_Test"),
		CBehavior_Tree::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/FalseSovereign/FalseSovereign_BT.json"))))
		CRASH("BehaviorTree Create Failed");

	// Prototype_Component_AnimMachine_Test
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_AnimMachine_FalseSovereign"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/FalseSovereign/Animation/FalseSovereign_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_Component_Model_FalseSovereign
	//_fmatrix PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	_fmatrix PreTransformMatrix = XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_FalseSovereign"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/Monster/FalseSovereign/FalseSovereignTest.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_GameObject_MonsterTest
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_MonsterTest"),
		CMonsterTest::Create(m_pDevice, m_pContext))))
		CRASH("MonsterTest Prototype Create Failed");

#pragma region GGOBUL
	// Prototype_Component_Model_Ggobul
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Ggobul"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/Monster/Ggobul/Ggobul.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_Component_AnimMachine_Ggobul
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_AnimMachine_Ggobul"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/Ggobul/Animation/Ggobul_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_GameObject_Ggobul
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Ggobul"),
		CGgobul::Create(m_pDevice, m_pContext))))
		CRASH("MonsterTest Prototype Create Failed");
#pragma endregion

#pragma region SCYTHE_TANTACLE
	// Prototype_Component_Model_Scythe
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Scythe"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/Monster/FS_Scythe/FS_Scythe.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_Component_AnimMachine_Scythe
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_AnimMachine_Scythe"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/FS_Scythe/Animation/FS_Scythe_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_GameObject_Scythe
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Scythe"),
		CFS_Scythe::Create(m_pDevice, m_pContext))))
		CRASH("MonsterTest Prototype Create Failed");
#pragma endregion

	return S_OK;
}

#pragma region MONSTER
HRESULT CLoader_GamePlay::Load_Monster()
{
	// Prototype_Component_BehaviorTree_Ordinary
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_BehaviorTree_Ordinary"),
		CBehavior_Tree::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/HavocWarrior/MonsterOrdinary_BT.json"))))
		CRASH("BehaviorTree Create Failed");

#pragma region HAVOC_WARRIOR
	// Prototype_Component_AnimMachine_HavocWarrior
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_AnimMachine_HavocWarrior"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/HavocWarrior/Animation/HavocWarrior_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_Component_Model_HavocWarrior
	_fmatrix PreTransformMatrix = XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_HavocWarrior"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/Monster/HavocWarrior/HavocWarrior.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_GameObject_HavocWarrior
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_HavocWarrior"),
		CHavocWarrior::Create(m_pDevice, m_pContext))))
		CRASH("MonsterTest Prototype Create Failed");
#pragma endregion

#pragma region ELECTRO_PREDATOR
	// Prototype_Component_AnimMachine_ElectroPredator
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_AnimMachine_ElectroPredator"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/ElectroPredator/Animation/ElectroPredator_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_Component_Model_ElectroPredator
	//_fmatrix PreTransformMatrix = XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_ElectroPredator"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/Monster/ElectroPredator/ElectroPredator.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_GameObject_ElectroPredator
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_ElectroPredator"),
		CElectroPredator::Create(m_pDevice, m_pContext))))
		CRASH("Electro Predator Prototype Create Failed");

	// Prototype_GameObject_Projectile
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Projectile"),
		CProjectile::Create(m_pDevice, m_pContext))))
		CRASH("Projectile Create Failed");

	// Prototype_GameObject_AOEDOT
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_AOEDOT"),
		CAoEDoT::Create(m_pDevice, m_pContext))))
		CRASH("AoEDoT Prototype Create Failed");

	// Prototype_Component_Model_Arrow
	_fmatrix PreArrowMatrix = XMMatrixScaling(0.00008f, 0.00008f, 0.00008f) * XMMatrixRotationX(XMConvertToRadians(90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Arrow"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreArrowMatrix, "../../Client/Bin/Resource/Model/Monster/Arrow/Arrow.dat"))))
		CRASH("Prototype Create Failed");
#pragma endregion

#pragma region CORROSAURUS
	// Prototype_Component_BehaviorTree_CoroSaurus
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_BehaviorTree_CoroSaurus"),
		CBehavior_Tree::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/Corrosaurus/Corrosaurus_BT.json"))))
		CRASH("BehaviorTree Create Failed");

	// Prototype_Component_AnimMachine_CoroSaurus
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_AnimMachine_CoroSaurus"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/Corrosaurus/Animation/Corrosaurus_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_Component_Model_CoroSaurus
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_CoroSaurus"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/Monster/Corrosaurus/Corrosaurus.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_GameObject_CoroSaurus
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_CoroSaurus"),
		CCorosaurus::Create(m_pDevice, m_pContext))))
		CRASH("MonsterTest Prototype Create Failed");
#pragma endregion
	return S_OK;
}
#pragma endregion

CCustom_UI::CUSTOM_UITREE_DESC CLoader_GamePlay::Load_UITree(_string strFilePath)
{
	ifstream file(strFilePath);
	json jUIInfoData = {};
	if (file.is_open()) {
		file >> jUIInfoData;
	}
	else
		CRASH("File Open Failed.");

	CCustom_UI::CUSTOM_UITREE_DESC tDesc = {};
	from_json(jUIInfoData, tDesc);

	return tDesc;
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
