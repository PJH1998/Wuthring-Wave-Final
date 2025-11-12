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

// Player
#include "Player.h"
#pragma endregion


CLoader_GamePlay::CLoader_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_GamePlay::Initialize()
{
	m_iNumLoadingThread = 11;
	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });
	//m_pGameInstance->Add_Work([this]() {Load_MonsterTest(); Complete_Load(); });w

	m_pGameInstance->Add_Work([this]() {Load_Augusta(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Rover(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Player(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_MonsterTest(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Monster(); Complete_Load(); });
	
	m_pGameInstance->Add_Work([this]() {Load_Effect(); Complete_Load(); });

	m_pGameInstance->Add_Work([this]() {Load_UI(); Complete_Load(); });

	m_pGameSystem->Add_Action("../Bin/Resource/Sequence/Action/");
    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Texture()
{
	cout << "Texture" << endl;

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Model()
{
	//m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/Asphodel_Barrens_1111_dest_Fix/", m_eCurLevel);
	m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/The_False_Soerveign_1112_first/", m_eCurLevel);

	// SkyBox
	_matrix PreTransformMatrix = XMMatrixScaling(0.001f, 0.001f, 0.001f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Skybox_Background"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../Bin/Resource/Skybox/SkyBackground21.dat"))))
		CRASH("SkyBackground");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Skybox_Dome"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../Bin/Resource/Skybox/SkyDome.dat"))))
		CRASH("SkyDome");


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

	for (auto& treeDesc : vecDescs)
	{
		for (auto& infoDesc : treeDesc.vecUIInfoDescs)
		{
			const   _wstring    strFilePath = infoDesc.tUIDesc.strFilePath;
			const   _wstring	strFileName = infoDesc.tUIDesc.strFileName;
			const   _uint       iNumFiles = infoDesc.tUIDesc.iNumFiles;

			if (strFileName == L"T_JiabeilinaEnergyBgCombined")
				int i = 10;

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


	// ==============================
	cout << "[CLoader_Test_UI][UI Custom] Prototype" << endl;
	// ==============================

	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_HUD",
		CUI_HUD::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test_UI::Load_Prototype] UI_HUD Load Failed. The UI_HUD may have already been loaded.\n");

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

HRESULT CLoader_GamePlay::Load_MonsterTest()
{
	cout << "MonsterTest" << endl;
		// Prototype_GameObject_AttackVolume
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_AttackVolume"),
		CAttackVolume::Create(m_pDevice, m_pContext))))
		CRASH("AttackVolume Create Failed");

	// Prototype_Component_BehaviorTree_Test
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_BehaviorTree_Test"),
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

#pragma region GGOBUL
	// Prototype_Component_Model_Ggobul
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Ggobul"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/Ggobul/Ggobul.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_Component_AnimMachine_Ggobul
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_AnimMachine_Ggobul"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Ggobul/Animation/Ggobul_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_GameObject_Ggobul
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Ggobul"),
		CGgobul::Create(m_pDevice, m_pContext))))
		CRASH("MonsterTest Prototype Create Failed");
#pragma endregion

#pragma region SCYTHE_TANTACLE
	// Prototype_Component_Model_Scythe
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Scythe"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/FS_Scythe/FS_Scythe.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_Component_AnimMachine_Scythe
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_AnimMachine_Scythe"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/FS_Scythe/Animation/FS_Scythe_StateMachine.json"))))
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
		CBehavior_Tree::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/HavocWarrior/MonsterOrdinary_BT.json"))))
		CRASH("BehaviorTree Create Failed");

#pragma region HAVOC_WARRIOR
	// Prototype_Component_AnimMachine_HavocWarrior
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_AnimMachine_HavocWarrior"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/HavocWarrior/Animation/HavocWarrior_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_Component_Model_HavocWarrior
	_fmatrix PreTransformMatrix = XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_HavocWarrior"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/HavocWarrior/HavocWarrior.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_GameObject_HavocWarrior
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_HavocWarrior"),
		CHavocWarrior::Create(m_pDevice, m_pContext))))
		CRASH("MonsterTest Prototype Create Failed");
#pragma endregion

#pragma region ELECTRO_PREDATOR
	// Prototype_Component_AnimMachine_ElectroPredator
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_AnimMachine_ElectroPredator"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/ElectroPredator/Animation/ElectroPredator_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_Component_Model_ElectroPredator
	//_fmatrix PreTransformMatrix = XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_ElectroPredator"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/ElectroPredator/ElectroPredator.dat"))))
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
#pragma endregion

#pragma region CORROSAURUS
	// Prototype_Component_BehaviorTree_CoroSaurus
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_BehaviorTree_CoroSaurus"),
		CBehavior_Tree::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Corrosaurus/Corrosaurus_BT.json"))))
		CRASH("BehaviorTree Create Failed");

	// Prototype_Component_AnimMachine_CoroSaurus
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_AnimMachine_CoroSaurus"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Corrosaurus/Animation/Corrosaurus_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_Component_Model_CoroSaurus
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_CoroSaurus"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/Corrosaurus/Corrosaurus.dat"))))
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
