#include "ClientPch.h"
#include "Loader_Heaven.h"
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
#include"MapObject_Water.h"
#include"MapObject_Throw.h"
#include"MapObject_Burn.h"
#include"MapObject_Dome.h"

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
#include "Leviatan.h"
#include "Levi_Alter.h"
#include "Levi_Bayonet.h"
#include "Levi_Bow.h"
#include "Levi_Ray.h"
#include "Levi_Anchor.h"
#include "Levi_Drop.h"
#include "Levi_Wave.h"
#include "Levi_Augusta.h"
#pragma endregion

#pragma region UI
#include "Custom_UI.h"
#include "UI_Button.h"
#include "UI_Image.h"
#include "UI_Text_Damage.h"
#include "Animator_UI.h"
#include "UI_HUD.h"
#include "UI_HUD_Sector_FuncIcons.h"
#include "UI_HUD_Sector_Minimap.h"
#include "UI_FinalEnd.h"

#include "UI_Button_Interact.h"
#include "UI_LockOn.h"
#include "UI_Parry.h"
#include "UI_MobHPBar.h"
#include "UI_TabUtility.h"
#include "UI_GrapplePoint.h"
#include "UI_QTE.h"
#include "UI_Dialog.h"

#include "UI_CurveTrace.h"

#include "UI_Ovfl_Palette.h"
#pragma endregion


#pragma region PLAYER
#include "Wing.h"
#include "StateMachine.h"

// Augusta
#include "AugustaBayonet.h"
#include "AugustaSkillWeapon.h"
#include "AugustaGriffon.h"
#include "AugustaFxObject.h"
#include "AugustaEnergyBlade.h"
#include "AugustaHeadProp.h"
#include "AugustaBurstWeapon.h"
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

// Sequence Player
#include "Yuno.h"
#include "YunoMoon.h"

// SequenceAugusta
#include "SequenceAugusta.h"

#include "SequenceLupa.h"
#include "LupaSpear.h"

#include "SequencePlayer.h"
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

#include "Heaven_SkyBox.h"

CLoader_Heaven::CLoader_Heaven(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_Heaven::Initialize()
{
	m_iNumLoadingThread = 17;

	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });

	m_pGameInstance->Add_Work([this]() {Load_Augusta(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Rover(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Galbrena(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Player(); Complete_Load(); });
	//
	m_pGameInstance->Add_Work([this]() {Load_Yuno(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_SequenceAugusta(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_SequenceLupa(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_SequencePlayer(); Complete_Load(); });

	m_pGameInstance->Add_Work([this]() {Load_Monster(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Leviatan(); Complete_Load(); });
	
	m_pGameInstance->Add_Work([this]() {Load_Effect(); Complete_Load(); });

	m_pGameInstance->Add_Work([this]() {Load_UI(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Font(); Complete_Load(); });

	m_pGameSystem->Add_Action("../Bin/Resource/Sequence/Action/");

    return S_OK;
}

HRESULT CLoader_Heaven::Load_Texture()
{
	cout << "Texture" << endl;

    return S_OK;
}

HRESULT CLoader_Heaven::Load_Model()
{
	m_pGameInstance->Load_Resource("../Bin/Resource/Map/Heaven/");
	m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/Heaven_1213_third/", m_eCurLevel, "Heaven");

	//// SkyBox
	//_matrix PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_Component_Model_Skybox_Dome"),
	//	CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../Bin/Resource/Skybox/SkyDome.dat"))))
	//	CRASH("SkyDome");
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_Component_Model_Skybox_Background"),
	//	CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../Bin/Resource/Skybox/SkyBackground_Heaven.dat"))))
	//	CRASH("SkyBackground");
	//PreTransformMatrix = XMMatrixScaling(0.07f, 0.07f, 0.07f) * XMMatrixRotationRollPitchYaw(0.f, XMConvertToRadians(110.f), 0.f);
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_Component_Model_Skybox_FX"),
	//	CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../Bin/Resource/Skybox/SkyFX_Heaven.dat"))))
	//	CRASH("SkyFX");

		// SkyBox
	_matrix PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationRollPitchYaw(0.f, XMConvertToRadians(115.f), 0.f);;
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_Component_Model_HeavenSB_Dome"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../Bin/Resource/Skybox/Heaven_SkyBox_Dome.dat"))))
		CRASH("Failed to Add Prototype HeavenSB_Dome");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_Component_Model_HeavenSB_Cloud"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../Bin/Resource/Skybox/Heaven_SkyBox_Cloud.dat"))))
		CRASH("Failed to Add Prototype HeavenSB_Cloud");
	PreTransformMatrix = XMMatrixScaling(25.f, 25.f, 1.f) * XMMatrixRotationRollPitchYaw(0.f, 0.f, 0.f) * XMMatrixTranslationFromVector(XMVectorSet(0.f, 0.f, -10.f, 1.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_Component_Model_HeavenSB_Fx"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../Bin/Resource/Skybox/Heaven_SkyBox_FX.dat"))))
		CRASH("Failed to Add Prototype HeavenSB_Fx");

	cout << "Model" << endl;

    return S_OK;
}

HRESULT CLoader_Heaven::Load_Shader()
{
	cout << "Shader" << endl;

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_Component_Shader_HeavenSkyBox"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxSkyBox_Heaven.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		CRASH("Failed to Add Prototype Shader_VtxSkyBox_Heaven");

    return S_OK;
}

HRESULT CLoader_Heaven::Load_Object()
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
	
	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_Water"),
		CMapObject_Water::Create(m_pDevice, m_pContext));
	
	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Spawner"),
		CSpawner::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_Throw"),
		CMapObject_Throw::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_Burn"),
		CMapObject_Burn::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_Dome"),
		CMapObject_Dome::Create(m_pDevice, m_pContext));

#pragma endregion

#pragma region SPAWN_OBJECT
	// Prototype_GameObject_Projectile
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Projectile"),
		CProjectile::Create(m_pDevice, m_pContext))))
		CRASH("Projectile Create Failed");
#pragma endregion

	// Prototype_GameObject_AttackVolume
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_GameObject_AttackVolume"),
		CAttackVolume::Create(m_pDevice, m_pContext))))
		CRASH("AttackVolume Create Failed");

#pragma region SKYBOX

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_GameObject_Heaven_SkyBox"),
		CHeaven_SkyBox::Create(m_pDevice, m_pContext))))
		CRASH("Failed to Add Prototype Heaven_SkyBox");

#pragma endregion

	return S_OK;
}

HRESULT CLoader_Heaven::Load_Player()
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

HRESULT CLoader_Heaven::Load_Augusta()
{
	_wstring wStrModelTag = L"Prototype_Component_Model_Augusta";
	//_string strFilePath = "../../Client/Bin/Resource/Model/Player/Augusta/Augusta.dat";
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/AugustaFacial/Augusta.dat";
	_matrix	PreTransformMatrix = XMMatrixIdentity();
	_float fSize = 0.01f;
	//_float fSize = 0.0001f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(180.f));

	// 1. 모델 초기화.
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
	//    CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
	//    CRASH("Prototype Create Failed");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::CHARACTER, PreTransformMatrix, strFilePath.c_str()))))
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
	strFilePath = "../../Client/Bin/Resource/Model/Player/AugustaFacial/Weapon/Bayonet/Bayonet.dat";
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
	strFilePath = "../../Client/Bin/Resource/Model/Player/AugustaFacial/Weapon/SkillWeapon/SkillWeapon.dat";
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
	strFilePath = "../../Client/Bin/Resource/Model/Player/AugustaFacial/Weapon/Griffon/Griffon.dat";
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

	wStrModelTag = L"Prototype_Component_Model_Augusta_FxObject";
	strFilePath = "../../Client/Bin/Resource/Model/Player/AugustaFacial/Weapon/FxObject/FxObject.dat";
	fSize = 0.01f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationZ(XMConvertToRadians(-90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	_wstring wStrFxObjectTag = TEXT("Prototype_GameObject_Augusta_FxObject");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrFxObjectTag
		, CAugustaFxObject::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");


	// EnergyBlade의 경우는 FxObject가 소유 => FxObject의 뼈에 붙을예정.
	wStrModelTag = L"Prototype_Component_Model_Augusta_EnergyBlade";
	strFilePath = "../../Client/Bin/Resource/Model/Player/AugustaFacial/Weapon/EnergyBlade/EnergyBlade.dat";
	fSize = 0.01f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	_wstring wStrEnergyBladeTag = TEXT("Prototype_GameObject_Augusta_EnergyBlade");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrEnergyBladeTag
		, CAugustaEnergyBlade::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	wStrModelTag = L"Prototype_Component_Model_Augusta_HeadProp";
	strFilePath = "../../Client/Bin/Resource/Model/Player/AugustaFacial/Weapon/HeadProp/HeadProp.dat";
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationX(XMConvertToRadians(-90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	_wstring wStrHeadPropTag = TEXT("Prototype_GameObject_Augusta_HeadProp");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrHeadPropTag
		, CAugustaHeadProp::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	wStrModelTag = L"Prototype_Component_Model_Augusta_BurstWeapon";
	strFilePath = "../../Client/Bin/Resource/Model/Player/AugustaFacial/Weapon/BurstWeapon/AugustaBurstWeapon.dat";
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	_wstring wStrBurstWeaponTag = TEXT("Prototype_GameObject_Augusta_BurstWeapon");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrBurstWeaponTag
		, CAugustaBurstWeapon::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");
#pragma endregion



	return S_OK;
}

HRESULT CLoader_Heaven::Load_Rover()
{
	_wstring wStrModelTag = L"Prototype_Component_Model_Rover";
	//_string strFilePath = "../../Client/Bin/Resource/Model/Player/Rover/Rover.dat";
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/RoverFacial/Rover.dat";
	_matrix	PreTransformMatrix = XMMatrixIdentity();
	//_float fSize = 0.0001f;
	_float fSize = 0.01f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(180.f));

	// 1. 모델 초기화.
  /*  if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");*/
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::CHARACTER, PreTransformMatrix, strFilePath.c_str()))))
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
	strFilePath = "../../Client/Bin/Resource/Model/Player/RoverFacial/Weapon/Sword/Sword.dat";
	fSize = 0.01f;
	//fSize = 0.0001f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationX(XMConvertToRadians(-90.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// 2. 객체 초기화.
	_wstring wstrSwordTag = TEXT("Prototype_GameObject_Rover_Sword");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wstrSwordTag
		, CRoverSword::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	wStrModelTag = L"Prototype_Component_Model_Rover_DarkWing";
	strFilePath = "../../Client/Bin/Resource/Model/Player/RoverFacial/Weapon/DarkWing/DarkRoverWing.dat";
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
	strFilePath = "../../Client/Bin/Resource/Model/Player/RoverFacial/Weapon/DarkScythe/DarkScythe.dat";
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

HRESULT CLoader_Heaven::Load_Galbrena()
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
	strFilePath = "../../Client/Bin/Resource/Model/Player/GalbrenaFacial/Weapon/ShotGun/ShotGun.dat";
	fSize = 0.01f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationX(XMConvertToRadians(-90.f));
	//PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// DarkWing
	wStrModelTag = L"Prototype_Component_Model_Galbrena_DarkWing";
	strFilePath = "../../Client/Bin/Resource/Model/Player/GalbrenaFacial/Weapon/DarkWing/DarkWing.dat";
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

HRESULT CLoader_Heaven::Load_SequencePlayer()
{
	_wstring wStrPlayerTag = TEXT("Prototype_GameObject_SequencePlayer");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrPlayerTag
		, CSequencePlayer::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	return S_OK;
}

HRESULT CLoader_Heaven::Load_Yuno()
{
	_wstring wStrModelTag = L"Prototype_Component_Model_Yuno";
	_string strFilePath = "../../Client/Bin/Resource/Model/SequencePlayer/YunoFacial/Yuno.dat";
	_matrix	PreTransformMatrix = XMMatrixIdentity();
	_float fSize = 0.01f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(180.f));

	// 1. Model 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::CHARACTER, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// 2. StateMachine 초기화
	_wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_Yuno";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrStateMachineTag,
		CStateMachine::Create(m_pDevice, m_pContext))))
		CRASH("PlayerState Machine");

	// 3. 객체 초기화
	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_Yuno");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrActorTag
		, CYuno::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");


	wStrModelTag = L"Prototype_Component_Model_Yuno_Moon";
	strFilePath = "../../Client/Bin/Resource/Model/SequencePlayer/YunoFacial/Weapon/Moon/Moon.dat";
	fSize = 0.01f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationX(XMConvertToRadians(-90.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// 2. 객체 초기화.
	_wstring wstrSwordTag = TEXT("Prototype_GameObject_Yuno_Moon");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wstrSwordTag
		, CYunoMoon::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	return S_OK;
}

HRESULT CLoader_Heaven::Load_SequenceAugusta()
{

	_wstring wStrModelTag = L"Prototype_Component_Model_SequenceAugusta";
	_string strFilePath = "../../Client/Bin/Resource/Model/SequencePlayer/Augusta/Augusta.dat";
	_matrix	PreTransformMatrix = XMMatrixIdentity();
	_float fSize = 0.0001f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(180.f));

	// 1. Model 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// 2. StateMachine 초기화
	_wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_SequenceAugusta";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrStateMachineTag,
		CStateMachine::Create(m_pDevice, m_pContext))))
		CRASH("PlayerState Machine");

	// 3. 객체 초기화
	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_SequenceAugusta");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrActorTag
		, CSequenceAugusta::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	return S_OK;
}

HRESULT CLoader_Heaven::Load_SequenceLupa()
{
	_wstring wStrModelTag = L"Prototype_Component_Model_SequenceLupa";
	_string strFilePath = "../../Client/Bin/Resource/Model/SequencePlayer/Lupa/Lupa.dat";
	_matrix	PreTransformMatrix = XMMatrixIdentity();
	_float fSize = 0.0001f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(180.f));

	// 1. Model 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// 2. StateMachine 초기화
	_wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_SequenceLupa";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrStateMachineTag,
		CStateMachine::Create(m_pDevice, m_pContext))))
		CRASH("PlayerState Machine");

	// 3. 객체 초기화
	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_SequenceLupa");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrActorTag
		, CSequenceLupa::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	wStrModelTag = L"Prototype_Component_Model_Lupa_Spear";
	strFilePath = "../../Client/Bin/Resource/Model/SequencePlayer/Lupa/Weapon/Spear/LupaSpear.dat";
	fSize = 0.01f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationX(XMConvertToRadians(-90.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");

	// 2. 객체 초기화.
	_wstring wstrSpearTag = TEXT("Prototype_GameObject_Lupa_Spear");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wstrSpearTag
		, CLupaSpear::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	return S_OK;
}


HRESULT CLoader_Heaven::Load_UI()
{
	const   _uint       iDestLevel = ENUM_CLASS(LEVEL::HEAVEN);


	// ==============================
	cout << "[Loader_Test] Texture" << endl;
	// ==============================

	vector<CCustom_UI::CUSTOM_UITREE_DESC> vecDescs = {};       // parsed data from json

	// * Json Parse                 // for pre-loading textures
	// UI_HUD
	//_string strFilePath_UI_HUD = "../../Client/Bin/Resource/UI/FJson/UITree/TestHUD.json"; // ksta
	_string strFilePath_UI_HUD = "../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD_251030_2037.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_HUD));

	_string strFilePath_UI_HUD_Sector_Minimap = "../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD_Sector_Minimap.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_HUD_Sector_Minimap));

	_string strFilePath_UI_HUD_Sector_FuncIcons = "../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD_Sector_FuncIcons.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_HUD_Sector_FuncIcons));

	_string strFilePath_UI_FinalEnd = "../../Client/Bin/Resource/UI/FJson/UITree/Root_FinalEnd.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_FinalEnd));


	_string strFilePath_UI_Interact = "../../Client/Bin/Resource/UI/FJson/UITree/Root_Interact.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_Interact));

	_string strFilePath_UI_LockOn = "../../Client/Bin/Resource/UI/FJson/UITree/Root_LockOn.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_LockOn));

	_string strFilePath_UI_Parry = "../../Client/Bin/Resource/UI/FJson/UITree/Root_Parry.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_Parry));

	_string strFilePath_UI_MobHP = "../../Client/Bin/Resource/UI/FJson/UITree/Root_MobHPBarDynamic.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_MobHP));

	_string strFilePath_UI_TabUtility = "../../Client/Bin/Resource/UI/FJson/UITree/Root_TabUtility.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_TabUtility));

	_string strFilePath_UI_OverflowingPalette = "../../Client/Bin/Resource/UI/FJson/UITree/Root_Palette.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_OverflowingPalette));

	_string strFilePath_UI_GrapplePoint = "../../Client/Bin/Resource/UI/FJson/UITree/Root_GrapplePoint.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_GrapplePoint));

	_string strFilePath_UI_QTE = "../../Client/Bin/Resource/UI/FJson/UITree/Root_QTE1.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_QTE));

	_string strFilePath_UI_Dialog = "../../Client/Bin/Resource/UI/FJson/UITree/Root_Dialog.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_Dialog));




	_string strFilePath_UI_ExtraTexturesLoad = "../../Client/Bin/Resource/UI/FJson/UITree/Root_LoadDummy.json";
	vecDescs.push_back(Load_UITree(strFilePath_UI_ExtraTexturesLoad));
	// Prototype_Component_Texture_Custom_ ...
	// Palette_BG



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
			{
				OutputDebugString(L"[Loader_Test::Ready_Prototypes] Texture Load Failed. The texture may have already been loaded.\n");
			}
		}
	}




	// ==============================
	cout << "[Loader_Test] Model" << endl;
	// ==============================
	// 
	// VIBuffer_Rect
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Model] VIBuffer_Rect Load Failed. The VIBuffer_Rect may have already been loaded.\n");

	// VIBuffer_Rect_Instance_UI
	CVIBuffer_Rect_Instance_UI::RECT_INSTANCE_UI_DESC tRectInstDesc = {};
	tRectInstDesc.iNumInstance = 500U;
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect_Instance_UI"),
		CVIBuffer_Rect_Instance_UI::Create(m_pDevice, m_pContext, &tRectInstDesc))))
		OutputDebugString(L"[Loader_Test::Load_Model] VIBuffer_Rect_Instance_UI Load Failed. The VIBuffer_Rect_Instance_UI may have already been loaded.\n");

	// VIBuffer_CurveTrace
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_CurveTrace"),
		CVIBuffer_CurveTrace::Create(m_pDevice, m_pContext, 64))))
		OutputDebugString(L"[Loader_Test::Load_Model] VIBuffer_CurveTrace Load Failed. The VIBuffer_CurveTrace  may have already been loaded.\n");


	// ==============================
	cout << "[Loader_Test] Shader" << endl;
	// ==============================

	// Shader
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		OutputDebugString(L"[Loader_Test::Load_Shader] Shader Load Failed. The Shader may have already been loaded.\n");

	// Shader_Instance
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxInstance.hlsl"), VTXUIINSTANCE::Elements, VTXUIINSTANCE::iNumElements))))
		OutputDebugString(L"[Loader_Test::Load_Shader] Shader_Instance Load Failed. The Shader_Instance may have already been loaded.\n");

	// Shader_Font
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_Text_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_TextInstance.hlsl"), VTXUIINSTANCE::Elements, VTXUIINSTANCE::iNumElements))))
		OutputDebugString(L"[Loader_Test::Load_Shader] Shader_TextInstance Load Failed. The Shader_TextInstance may have already been loaded.\n");

	// Shader_CurveTrace
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxCurveTrace"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxCurveTrace.hlsl"), VTXUICURVE::Elements, VTXUICURVE::iNumElements))))
		OutputDebugString(L"[Loader_Test::Load_Shader] Shader_CurveTrace Load Failed. The Shader_CurveTrace may have already been loaded.\n");

	// Shader_CurveTrace_Sphere
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxCurveTrace_Sphere"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxCurveTrace_Sphere.hlsl"), VTXUICURVE::Elements, VTXUICURVE::iNumElements))))
		OutputDebugString(L"[Loader_Test::Load_Shader] Shader_CurveTrace_Sphere Load Failed. The Shader_CurveTrace_Sphere may have already been loaded.\n");


	// ==============================
	cout << "[Loader_Test] Object" << endl;
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
		OutputDebugString(L"[Loader_Test:Load_Object] UI_Button Load Failed. The CUI_Button may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Image",
		CUI_Image::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_Image Load Failed. The UI_Image may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Text",
		CUI_Text::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_Text Load Failed. The CUI_Text may have already been loaded.\n");

	// Custom Text
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Text_Damage",
		CUI_Text_Damage::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_Text_Damage Load Failed. The UI_Text_Damage may have already been loaded.\n");

	// Custom UI (Props)
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
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_GrapplePoint",
		CUI_GrapplePoint::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_GrapplePoint Load Failed. The UI_GrapplePoint may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_QTE",
		CUI_QTE::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_QTE Load Failed. The UI_QTE may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Dialog",
		CUI_Dialog::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_Dialog Load Failed. The UI_Dialog may have already been loaded.\n");


	// Custom UI (but not UI)
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_UI_CurveTrace",
		CUI_CurveTrace::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_CurveTrace Load Failed. The UI_CurveTrace may have already been loaded.\n");

	// Custom UI (MiniGames)
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Ovfl_Palette",
		CUI_Ovfl_Palette::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Object] UI_Ovfl_Palette Load Failed. The UI_Ovfl_Palette may have already been loaded.\n");




	// ==============================
	cout << "[Loader_Test][UI Custom] Prototype" << endl;
	// ==============================

	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_HUD",
		CUI_HUD::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Prototype] UI_HUD_Sector_FuncIcons Load Failed. The UI_HUD_Sector_FuncIcons may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_HUD_Sector_Minimap",
		CUI_HUD_Sector_Minimap::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Prototype] UI_HUD_Sector_Minimap Load Failed. The UI_HUD_Sector_Minimap may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_HUD_Sector_FuncIcons",
		CUI_HUD_Sector_FuncIcons::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Prototype] UI_HUD_Sector_FuncIcons Load Failed. The UI_HUD_Sector_FuncIcons may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_FinalEnd",
		CUI_FinalEnd::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test::Load_Prototype] UI_Container_FinalEnd Load Failed. The UI_Container_FinalEnd may have already been loaded.\n");


	return S_OK;
}

HRESULT CLoader_Heaven::Load_Font()
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

HRESULT CLoader_Heaven::Load_Effect()
{
	m_pGameSystem->Create_Effect("../../Client/Bin/Resource/Effect/Prefabs/Common", m_eCurLevel);
	m_pGameSystem->Load_EffectTexture_FromFolder("../../Client/Bin/Resource/Effect/Prefabs/Common/Texture", m_eCurLevel);
	m_pGameSystem->Load_EffectMeshDat_FromFolder("../../Client/Bin/Resource/Effect/Prefabs/Common/Dat", m_eCurLevel);
	m_pGameSystem->Load_EffectVAMeshDat_FromFolder("../../Client/Bin/Resource/Effect/EffectVA/Dat", m_eCurLevel);
	m_pGameSystem->Load_EffectVATexture_FromFolder("../../Client/Bin/Resource/Effect/EffectVA/Color", m_eCurLevel);
	m_pGameSystem->Load_EffectVATexture_FromFolder("../../Client/Bin/Resource/Effect/EffectVA/Mask", m_eCurLevel);
	m_pGameSystem->Load_EffectLightData_FromFolder("../../Client/Bin/Resource/Effect/Prefabs/Common/Light");

	m_pGameSystem->Create_Effect("../../Client/Bin/Resource/Effect/Prefabs/Leviatan", m_eCurLevel);
	m_pGameSystem->Create_Effect("../../Client/Bin/Resource/Effect/Prefabs/Sequence", m_eCurLevel);

	return S_OK;
}

#pragma region MONSTER
HRESULT CLoader_Heaven::Load_Monster()
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

HRESULT CLoader_Heaven::Load_Leviatan()
{
	cout << "Leviatan" << endl;

	// Prototype_Component_Model_Leviatan
	//_fmatrix PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	_fmatrix PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Leviatan"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::CHARACTER, PreTransformMatrix, "../../Client/Bin/Resource/Model/Monster/Leviatan/Leviatan.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_Component_BehaviorTree_Leviatan1
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_BehaviorTree_Leviatan1"),
		CBehavior_Tree::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/Leviatan/Leviatan_BT.json"))))
		CRASH("BehaviorTree Create Failed");

	// Prototype_Component_AnimMachine_Leviatan_Phase1
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_AnimMachine_Leviatan_Phase1"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/Leviatan/Animation/Leviatan_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_Component_BehaviorTree_Leviatan2
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_BehaviorTree_Leviatan2"),
		CBehavior_Tree::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/Leviatan/Leviatan_BT2.json"))))
		CRASH("BehaviorTree Create Failed");

	// Prototype_Component_AnimMachine_Leviatan_Phase2
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_AnimMachine_Leviatan_Phase2"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/Monster/Leviatan/Animation/Leviatan_StateMachine2.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_GameObject_Leviatan
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Leviatan"),
		CLeviatan::Create(m_pDevice, m_pContext))))
		CRASH("Leviatan Prototype Create Failed");

#pragma region ALTER
	// Prototype_Component_Model_Levi_Alter
	_fmatrix PreAlterMatrix = XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Levi_Alter"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreAlterMatrix, "../../Client/Bin/Resource/Model/Monster/Levi_Alter/Levi_Alter.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_GameObject_Levi_Alter
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Alter"),
		CLevi_Alter::Create(m_pDevice, m_pContext))))
		CRASH("Levi_Alter Prototype Create Failed");
#pragma endregion

#pragma region WEAPON
	// Prototype_Component_Model_Leviatan_Bayonet
	_matrix PreWeaponMatrix = XMMatrixScaling(0.0001f, 0.0001f, 0.0001f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Leviatan_Bayonet"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreWeaponMatrix, "../../Client/Bin/Resource/Model/Monster/Levi_Prop/Levi_Dajian/Levi_Dajian.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_GameObject_Levi_Bayonet
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Bayonet"),
		CLevi_Bayonet::Create(m_pDevice, m_pContext))))
		CRASH("Leviatan Prototype Create Failed");

	// Prototype_Component_Model_Leviatan_Bow
	//PreWeaponMatrix = XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Leviatan_Bow"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreWeaponMatrix, "../../Client/Bin/Resource/Model/Monster/Levi_Prop/Bow/Levi_Bow.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_Component_Model_Leviatan_Projectile
	PreWeaponMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationX(XMConvertToRadians(90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Leviatan_Projectile"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreWeaponMatrix, "../../Client/Bin/Resource/Model/Monster/Levi_Prop/Projectile/SwordProjectile.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_Component_Model_Leviatan_SwordAura
	PreWeaponMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationX(XMConvertToRadians(-90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Leviatan_SwordAura"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreWeaponMatrix, "../../Client/Bin/Resource/Model/Monster/Levi_Prop/SwordAura/SwordAura.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_Component_Model_Leviatan_Anchor
	PreWeaponMatrix = XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) * XMMatrixRotationX(XMConvertToRadians(-90.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_Leviatan_Anchor"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreWeaponMatrix, "../../Client/Bin/Resource/Model/Monster/Levi_Prop/Anchor/Levi_Anchor.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_GameObject_Levi_Bow
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Bow"),
		CLevi_Bow::Create(m_pDevice, m_pContext))))
		CRASH("Leviatan Prototype Create Failed");

	// Prototype_GameObject_Levi_Ray
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Ray"),
		CLevi_Ray::Create(m_pDevice, m_pContext))))
		CRASH("Leviatan Prototype Create Failed");

	// Prototype_GameObject_Levi_Anchor
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Anchor"),
		CLevi_Anchor::Create(m_pDevice, m_pContext))))
		CRASH("Leviatan Prototype Create Failed");

	// Prototype_GameObject_Levi_Drop
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Drop"),
		CLevi_Drop::Create(m_pDevice, m_pContext))))
		CRASH("Leviatan Prototype Create Failed");

	// Prototype_GameObject_Levi_Wave
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Wave"),
		CLevi_Wave::Create(m_pDevice, m_pContext))))
		CRASH("Leviatan Prototype Create Failed");

	// Prototype_GameObject_Levi_Augusta
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Augusta"),
		CLevi_Augusta::Create(m_pDevice, m_pContext))))
		CRASH("Leviatan Prototype Create Failed");
#pragma endregion
	return S_OK;
}
#pragma endregion

CCustom_UI::CUSTOM_UITREE_DESC CLoader_Heaven::Load_UITree(_string strFilePath)
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



CLoader_Heaven* CLoader_Heaven::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLoader_Heaven* pInstance = new CLoader_Heaven(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Loader_GamePlay");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader_Heaven::Free()
{
    __super::Free();
}
