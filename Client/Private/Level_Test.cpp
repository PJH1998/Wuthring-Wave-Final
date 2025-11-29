#include "ClientPch.h"
#include "AnimationDummy.h"
#include "GameSystem.h"
#include "Level_Test.h"
#include "MapObject.h"
#pragma region MONSTER
#include "MonsterTest.h"
#include "Ggobul.h"
#include "FS_Scythe.h"
#include "HavocWarrior.h"
#include "ElectroPredator.h"
#include "Corosaurus.h"
#include "PatternDummy.h"
#include "Leviatan.h"
#pragma endregion
#include "Player.h"
#include "ShadowMap.h"
#include "SkyBox.h"
#include "Projectile.h"
#include "AoEDoT.h"
#include "Spawner.h"
#include"Trigger_Box.h"
#include "UI_Text_Damage.h"
#include "SceneCamera.h"
#include "UI_Parry.h"
#include "UI_MobHPBar.h"
#include "UI_GrafflePoint.h"

#include "DummyNPC.h"
//#define KSTA_UITEST_OLD
#ifdef KSTA_UITEST_OLD
#include "UI_Text.h"
#endif // KSTA_UITEST_OLD

#pragma region OBJECT
#include "RopeAnchor.h"
#pragma endregion


CLevel_Test::CLevel_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :	CLevel(pDevice,pContext), m_pGameSystem { CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CLevel_Test::Initialize()
{
	// SetUp OctoTree
	m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096, 4096, 4096));

	//TEST
	//SHADOW_MAP_DESC ShadowMapDesc = {};
	//ShadowMapDesc.iNumSectorX = 2;
	//ShadowMapDesc.iNumSectorZ = 2;
	//ShadowMapDesc.iSectorSizeX = 2048;
	//ShadowMapDesc.iSectorSizeZ = 2048;
	//ShadowMapDesc.vCenterPos = _float3(0.f, 0.f, 0.f);
	//ShadowMapDesc.vExtents = _float3(500.f, 250.f, 500.f);
	//ShadowMapDesc.vLightDir = _float3(0.f, -1.f, 0.5f);

	//if (FAILED(m_pGameInstance->Setting_ShadowMap(ShadowMapDesc)))
	//	CRASH("Test");

	//로더에서 부른 것과 같은 거 부르기.
	m_pGameSystem->Clone_MapObjects(m_eCurLevel);

    Ready_Layer_Player();
	//Ready_Dummy();
	//Ready_MonsterTest();
	//Ready_HavocWarrior();
	//Ready_ElectroPredator();
	Ready_CoroSaurus();
	//Ready_Spawner();
	Ready_AnimInstanceTest();
	//Ready_Leviatan();

    Ready_Effect();
	Ready_RopeAnchor();

    LIGHT_DESC LightDesc{};
    LightDesc.eType = LIGHT_DESC::DIRECTION;
    LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(1.f, -0.5f, -1.f, 0.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

    m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
    m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
    m_pGameInstance->SetUp_CameraNF();

	// Test
	_uint iLevel = m_pGameInstance->Get_CurrentLevel();

	m_pGameInstance->Add_Prototype(iLevel, TEXT("Prototype_GameObject_TriggerBox"),

	CTrigger_Box::Create(m_pDevice, m_pContext));

	CTrigger_Box::TRIGGER Tri;
	Tri.iLevel = iLevel;
	Tri.vExtends = _float3(5.f, 10.f, 5.f);
	_matrix Mat = XMMatrixTranslationFromVector(XMVectorSet(10.f, -3.f, 10.f, 1.f));
	_float4x4 TT;
	XMStoreFloat4x4(&TT, Mat);
	Tri.WorldMatrix = &TT;
	m_pGameInstance->Add_GameObject_ToLayer(iLevel, TEXT("Prototype_GameObject_TriggerBox"), iLevel, TEXT("Layer_Trigger"), &Tri);

	Ready_Scene();
	//Ready_Skybox();
	Ready_UI();

    return S_OK;
}

void CLevel_Test::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("Test"));
    
#ifdef _DEBUG
	Shader_Gui();
#endif

	Toggle_HUD();

	Testing_UI(fTimeDelta);

	
}

void CLevel_Test::Render()
{
}

void CLevel_Test::Ready_Layer_Player()
{
    _float3 vScale{}, vRotation{}, vPosition{};
    vScale = { 1.f, 1.f, 1.f };
    vRotation = { 0.f, 0.f, 0.f };
    vPosition = { 0.f, -10.f, 50.f };
    //vPosition = { 3455.f, 160.f, 2951.f };

    CPlayer::PLAYER_DESC Desc{};
    Desc.eCurLevel = m_eCurLevel;
    Desc.vScale = vScale;
    Desc.vRotation = vRotation;
    Desc.vPosition = vPosition;
    Desc.iPlayerCount = CPlayer::CHARACTERTYPE::TYPE_END;
    Desc.wStrInputControllerTag = TEXT("Prototype_Component_PlayerController");

    // 0. vector 크기 정의
    Desc.PlayerSpecs.resize(CPlayer::CHARACTERTYPE::TYPE_END);

	// 1. Rover(주인공) 캐릭터 정의
	Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::ROVER].CharacterDesc = PlayerData::GetRoverCloneData(vScale, vRotation, vPosition, m_eCurLevel);
	Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::ROVER].strActorTag = TEXT("Prototype_GameObject_Actor_Rover");

    // 2. Augusta 정의.
    Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::AUGUSTA].CharacterDesc = PlayerData::GetAugustaCloneData(vScale, vRotation, vPosition, m_eCurLevel);
    Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::AUGUSTA].strActorTag = TEXT("Prototype_GameObject_Actor_Augusta");

	// 3. Galbrena 정의
	Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::GALBRENA].CharacterDesc = PlayerData::GetGalbrenaCloneData(vScale, vRotation, vPosition, m_eCurLevel);
	Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::GALBRENA].strActorTag = TEXT("Prototype_GameObject_Actor_Galbrena");



    

    // 4. Player(Character 모음) 생성.
    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Player"),
        ENUM_CLASS(m_eCurLevel), TEXT("Layer_Players"), &Desc)))
        CRASH("Failed Ready Player");
}

void CLevel_Test::Ready_Dummy()
{

	//m_pGameSystem->Create_MonsterDummy(LEVEL::TEST, _float3(0.f, 5.f, 50.f), XMMatrixScaling(0.0001f, 0.0001f, 0.0001f));
	
	CPatternDummy::PAT_DUMMYDESC DummyDesc{};
	DummyDesc.eLevel = m_eCurLevel;
	//DummyDesc.strModelTag = TEXT("Prototype_Component_Model_FalseSovereign");				//신왕
	//DummyDesc.strInitAnimTag = "Stand1";
	//DummyDesc.strFolderPath = "../Bin/Resource/Model/Monster/FalseSovereign/Notify";
	
	DummyDesc.strModelTag = TEXT("Prototype_Component_Model_CoroSaurus");					//코로
	DummyDesc.strInitAnimTag = "Stand";
	DummyDesc.strFolderPath = "../Bin/Resource/Model/Monster/CorroSaurus/Notify";
	 
	//DummyDesc.strModelTag = TEXT("Prototype_Component_Model_Ggobul");						//꼬불이
	//DummyDesc.strInitAnimTag = "SAttack01_1";
	//DummyDesc.strFolderPath = "../Bin/Resource/Model/Monster/Ggobul/Notify";
	// 
	//DummyDesc.strModelTag = TEXT("Prototype_Component_Model_Scythe");						//촉수
	//DummyDesc.strFolderPath = "../Bin/Resource/Model/Monster/FS_Scythe/Notify";
	//DummyDesc.strInitAnimTag = "Stand1";
	DummyDesc.vInitPosition = _float3(0.f, -7.f, -6.f);
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_PatternDummy"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Monster"), &DummyDesc)))
		CRASH("Failed Ready Monster");

	CGgobul::GGOBUL_DESC Ggobul{};
	Ggobul.eCurLevel = m_eCurLevel;
	Ggobul.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	Ggobul.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	Ggobul.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_Ggobul"));
	Ggobul.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	Ggobul.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
	Ggobul.strFolderPath = "../Bin/Resource/Model/Monster/Ggobul/Notify";
	Ggobul.fRotationPerSec = XMConvertToRadians(90.f);
	Ggobul.fSpeedPerSec = 10.f;
	Ggobul.fAttackDmg =0.f;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Ggobul"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_MonsterEffect"), TEXT("Pool_Ggobul"), 3, &Ggobul)))
		CRASH("Failed Ready Ggobul");

	//Scythe
	CFS_Scythe::SCYTHE_DESC Tantacle{};
	Tantacle.eCurLevel = m_eCurLevel;
	Tantacle.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	Tantacle.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	Tantacle.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_Scythe"));
	Tantacle.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	Tantacle.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
	Tantacle.strFolderPath = "../Bin/Resource/Model/Monster/FS_Scythe/Notify";
	Tantacle.fRotationPerSec = XMConvertToRadians(90.f);
	Tantacle.fSpeedPerSec = 10.f;
	Tantacle.fAttackDamage = 0.f;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Scythe"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_MonsterEffect"), TEXT("Pool_Scythe"), 2, &Tantacle)))
		CRASH("Failed Ready Scythe");
}

void CLevel_Test::Ready_MonsterTest()
{
	MONSTER_INFO* const pInfo = m_pGameSystem->Get_MonsterInfo("FalseSovereign");
	// False Sovereign
    CMonsterTest::MONSTERTEST_DESC MobDesc{};
    MobDesc.eCurLevel = m_eCurLevel;
    MobDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
    MobDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
    MobDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_FalseSovereign"));
    MobDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
    MobDesc.fRotationPerSec = XMConvertToRadians(90.f);
    MobDesc.fSpeedPerSec = 10.f;
    MobDesc.vInitPosition = _float3(0.f, -8.f, 4.f);
    MobDesc.pAnimationTag = "Born1";
	MobDesc.strFolderPath = "../Bin/Resource/Model/Monster/FalseSovereign/Notify";
	MobDesc.fHP = pInfo->fMaxHp;
	MobDesc.fAttackDmg = pInfo->fAttack;
	MobDesc.fMaxStamina = pInfo->fMaxStamina;
	MobDesc.vDetectRange = _float3(25.f, 13.f, 25.f);
    if(FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MonsterTest"),
        ENUM_CLASS(m_eCurLevel), TEXT("Layer_MonsterTest"), &MobDesc)))
        CRASH("Failed Ready MonsterTest");

	//Ggobul
	CGgobul::GGOBUL_DESC Ggobul{};
	Ggobul.eCurLevel = m_eCurLevel;
	Ggobul.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	Ggobul.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	Ggobul.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_Ggobul"));
	Ggobul.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	Ggobul.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
	Ggobul.strFolderPath = "../Bin/Resource/Model/Monster/Ggobul/Notify";
	Ggobul.fRotationPerSec = XMConvertToRadians(90.f);
	Ggobul.fSpeedPerSec = 10.f;
	Ggobul.fAttackDmg = MobDesc.fAttackDmg;
	if(FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel),TEXT("Prototype_GameObject_Ggobul"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_MonsterEffect"),TEXT("Pool_Ggobul"), 3, &Ggobul)))
		CRASH("Failed Ready Ggobul");

	//Scythe
	CFS_Scythe::SCYTHE_DESC Tantacle{};
	Tantacle.eCurLevel = m_eCurLevel;
	Tantacle.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	Tantacle.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	Tantacle.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_Scythe"));
	Tantacle.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	Tantacle.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
	Tantacle.strFolderPath = "../Bin/Resource/Model/Monster/FS_Scythe/Notify";
	Tantacle.fRotationPerSec = XMConvertToRadians(90.f);
	Tantacle.fSpeedPerSec = 10.f;
	Tantacle.fAttackDamage = MobDesc.fAttackDmg;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Scythe"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_MonsterEffect"), TEXT("Pool_Scythe"), 2, &Tantacle)))
		CRASH("Failed Ready Scythe");

	CProjectile::PROJECTILEDESC Projectile{};
	Projectile.fAttackDamage = MobDesc.fAttackDmg;
	Projectile.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	Projectile.iTargetLayers = { ENUM_CLASS(COLLISIONLAYER::PLAYER),ENUM_CLASS(COLLISIONLAYER::MAP) };
	Projectile.fRadius = 0.7f;
	Projectile.fSpeedPerSec = 15.f;
	Projectile.wstrModelTag = TEXT("Prototype_Component_Model_Arrow");
	Projectile.eType = TEXT_COLOR_TYPE::DARK;
	//Projectile.wstrEffectTag = ;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Projectile"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Projectile"), TEXT("Pool_Projectile_ShinWang"), 15, &Projectile)))
		CRASH("Failed Ready Projectile (False Sovereign)");

	//CAoEDoT::AOEDOT_DESC AoEDesc{};
	//AoEDesc.fAttackDamage = MobDesc.fAttackDmg * 2.f;
	//AoEDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_SKILL);
	//AoEDesc.iTargetLayers = { ENUM_CLASS(COLLISIONLAYER::PLAYER) };
	//AoEDesc.vExtent = _float3(1.f, 1.f, 3.f);
	//AoEDesc.vOffset = _float3(0.f, 1.f, 0.f);
	////AoEDesc.wstrEffectTag
	//if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_AOEDOT"),
	//	ENUM_CLASS(m_eCurLevel), TEXT("Layer_EnemyAD"), TEXT("Pool_AOEDOT_ShinWang"), 2, &AoEDesc)))
	//	CRASH("Failed Ready AoEDot (False Sovereign)");
}

void CLevel_Test::Ready_HavocWarrior()
{
	MONSTER_INFO* const pInfo = m_pGameSystem->Get_MonsterInfo("HavocWarrior");
	// Havoc Warrior
	CHavocWarrior::HAVOCWARRIOR_DESC tDesc{};
	tDesc.eCurLevel = m_eCurLevel;
	tDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	tDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	tDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_HavocWarrior"));
	tDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	tDesc.strFolderPath = "../Bin/Resource/Model/Monster/HavocWarrior/Notify";
	tDesc.fRotationPerSec = XMConvertToRadians(90.f);
	tDesc.fSpeedPerSec = 10.f;
	tDesc.vInitPosition = _float3(3.f, -8.f, 0.f);
	tDesc.pAnimationTag = "Stand1";
	tDesc.fHp = pInfo->fMaxHp;
	tDesc.fAttackDmg = pInfo->fAttack;
	tDesc.fImpluseRate = pInfo->fImpluseRate;
	//if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_HavocWarrior"),
	//	ENUM_CLASS(m_eCurLevel), TEXT("Layer_Monster"), &tDesc)))
	//	CRASH("Failed Ready Monster");
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_HavocWarrior"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), pInfo->wstrPoolTag, 4, &tDesc)))
		CRASH("Failed Ready Monster (Havoc Warrior)");
}

void CLevel_Test::Ready_ElectroPredator()
{
	MONSTER_INFO* const pInfo = m_pGameSystem->Get_MonsterInfo("ElectroPredator");
	// Electro Predator
	CElectroPredator::ELECTROPREDATOR_DESC ADesc{};
	ADesc.eCurLevel = m_eCurLevel;
	ADesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	ADesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	ADesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_ElectroPredator"));
	ADesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	ADesc.strFolderPath = "../Bin/Resource/Model/Monster/ElectroPredator/Notify";
	ADesc.fRotationPerSec = XMConvertToRadians(100.f);
	ADesc.fSpeedPerSec = 10.f;
	ADesc.vInitPosition = _float3(3.f, -8.f, 3.f);
	ADesc.pAnimationTag = "Stand2";
	ADesc.fHp = pInfo->fMaxHp;
	ADesc.fAttackDmg = pInfo->fAttack;
	ADesc.fImpluseRate = pInfo->fImpluseRate;
	//if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_ElectroPredator"),
	//	ENUM_CLASS(m_eCurLevel), TEXT("Layer_Monster"), &ADesc)))
	//	CRASH("Failed Ready Monster");

	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_ElectroPredator"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), pInfo->wstrPoolTag, 2, &ADesc)))
		CRASH("Failed Ready Monster (Electro Predatror)");

	CProjectile::PROJECTILEDESC Projectile{};
	Projectile.fAttackDamage = ADesc.fAttackDmg;
	Projectile.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	Projectile.iTargetLayers = { ENUM_CLASS(COLLISIONLAYER::PLAYER),ENUM_CLASS(COLLISIONLAYER::MAP) };
	Projectile.fRadius = 0.7f;
	Projectile.fSpeedPerSec = 15.f;
	Projectile.wstrModelTag = TEXT("Prototype_Component_Model_Arrow");
	Projectile.eType = TEXT_COLOR_TYPE::ELEC;
	Projectile.wstrEffectTag = TEXT("Projectile_Effect");
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Projectile"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Projectile"), TEXT("Pool_Projectile_Electro"), 15, &Projectile)))
		CRASH("Failed Ready Projectile (Electro Predatror)");

	CAoEDoT::AOEDOT_DESC AoEDesc{};
	AoEDesc.fAttackDamage = ADesc.fAttackDmg * 0.25f;
	AoEDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	AoEDesc.iTargetLayers = { ENUM_CLASS(COLLISIONLAYER::PLAYER) };
	AoEDesc.vExtent = _float3(1.f, 1.f, 1.f);
	AoEDesc.vOffset = _float3(0.f, 1.f, 0.f);
	AoEDesc.wstrEffectTag = TEXT("Electro_GroundAttack");
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_AOEDOT"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_AOEDOT"), TEXT("Pool_AOEDOT_Electro"), 7, &AoEDesc)))
		CRASH("Failed Ready AoEDot (Electro Predatror)");
}

void CLevel_Test::Ready_CoroSaurus()
{
	MONSTER_INFO* const pInfo = m_pGameSystem->Get_MonsterInfo("CoroSaurus");
	// Corosaurus
	CCorosaurus::CORROSAURUS_DESC CoroDesc{};
	CoroDesc.eCurLevel = m_eCurLevel;
	CoroDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	//CoroDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
	CoroDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	CoroDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_CoroSaurus"));
	CoroDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	CoroDesc.fRotationPerSec = XMConvertToRadians(90.f);
	CoroDesc.fSpeedPerSec = 10.f;
	CoroDesc.vInitPosition = _float3(0.f, -8.f, 4.f);
	CoroDesc.pAnimationTag = "Idle1";
	CoroDesc.strFolderPath = "../Bin/Resource/Model/Monster/Corrosaurus/Notify";
	CoroDesc.fHP = pInfo->fMaxHp;
	CoroDesc.fAttackDmg = pInfo->fAttack;
	CoroDesc.fMaxStamina = pInfo->fMaxStamina;
	CoroDesc.vDetectRange = _float3(25.f, 13.f, 25.f);
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_CoroSaurus"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), &CoroDesc)))
		CRASH("Failed Ready Monster");
}

void CLevel_Test::Ready_Effect()
{
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/Common", m_eCurLevel, 15);
	//m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/WeiZuoShenWang", m_eCurLevel, 15);
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/Corro", m_eCurLevel, 10);
}

void CLevel_Test::Ready_Skybox()
{
	CSkyBox::SKYBOX_DESC SkyboxDesc = {};
	SkyboxDesc.iNumModel = 2;
	SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_Dome"));
	SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_Background"));
	//SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_FX2"));

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Skybox"), ENUM_CLASS(m_eCurLevel),
		TEXT("Layer_BackGround"), &SkyboxDesc)))
		CRASH("Skybox");
}

void CLevel_Test::Ready_Spawner()
{
	_string test[3] = {"ElectroPredator","HavocWarrior", "HavocWarrior"};
	//const MONSTER_INFO* pMobInfo = m_pGameSystem->Get_MonsterInfo("HavocWarrior");
	CSpawner::SPAWNERDESC Spawner{};
	Spawner.vPosition = _float4(0.f, -6.f, -20.f, 1.f);
	Spawner.vExtent = _float3(20.f, 20.f, 20.f);
	//Spawner.vSpawnPosition = _float3(1.f, 0.f, 1.f);
	Spawner.vSpawnPositions = { _float4(0.f, -6.f, -20.f, 1.f), _float4(1.f, -6.f, -21.f, 1.f), _float4(-1.f, -6.f, -21.f, 1.f) };
	//Spawner.vSpawnRotateDegree = _float3(0.f, 60.f, 0.f);
	Spawner.fSpawnTime = 10.f;
	for (size_t i = 0; i < 3; i++)
	{
		const MONSTER_INFO* pMobInfo = m_pGameSystem->Get_MonsterInfo(test[i].c_str());
		Spawner.strMonsterKey.push_back(pMobInfo->strName);
	}

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Spawner"), ENUM_CLASS(m_eCurLevel),
		TEXT("Layer_BackGround"), &Spawner)))
		CRASH("Spawner");
}

void CLevel_Test::Ready_AnimInstanceTest()
{
	CDummyNPC::DUMMYNPC_DESC NPCDesc{};
	NPCDesc.eCurLevel = m_eCurLevel;
	NPCDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh_Instance"));
	NPCDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxInstance_AnimMesh"));
	NPCDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_AnimInstanceTest"));
	NPCDesc.wstrObjectPrototypeTag = TEXT("Prototype_GameObject_DummyCell");
	NPCDesc.vStartPositions = _float3(18.f, -6.f, -30.f);
	NPCDesc.wstrSkinningPrototypeTag = TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh_Skinning");
	m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_DummyNPC"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Z_Test"), &NPCDesc);
}

void CLevel_Test::Ready_Leviatan()
{
	MONSTER_INFO* const pInfo = m_pGameSystem->Get_MonsterInfo("Leviatan");

	CLeviatan::LEVIATAN_DESC MobDesc{};
	MobDesc.eCurLevel = m_eCurLevel;
	MobDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMeshCharacter"));
	MobDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshCharacter"));
	MobDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_Leviatan"));
	MobDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	MobDesc.fRotationPerSec = XMConvertToRadians(90.f);
	MobDesc.fSpeedPerSec = 10.f;
	MobDesc.vInitPosition = _float3(0.f, -8.f, -4.f);
	MobDesc.pAnimationTag = "Stand2";
	MobDesc.strFolderPath = "../Bin/Resource/Model/Monster/Leviatan/Notify";
	MobDesc.fHP = pInfo->fMaxHp;
	MobDesc.fAttackDmg = pInfo->fAttack;
	MobDesc.fMaxStamina = pInfo->fMaxStamina;
	MobDesc.vDetectRange = _float3(35.f, 20.f, 35.f);
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Leviatan"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), &MobDesc)))
		CRASH("Failed Ready MonsterTest");
}

void CLevel_Test::Ready_UI()
{
	// UI
	const   _uint       iDestLevel = ENUM_CLASS(m_eCurLevel);
	const _wstring strLayertag_UI = L"Layer_Custom_UI";
	const _wstring strPrototypeTag_UI[] = {
		 L"Prototype_GameObject_Custom_UI_Container_HUD"
	};
	for (auto& strPrototypeTag : strPrototypeTag_UI)
	{
		CUIObject* pTargetUI = static_cast<CUIObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, strPrototypeTag, PROTOTYPE::GAMEOBJECT));
		//if (FAILED(m_pGameInstance->Add_RootUI(L"UI_HUD", pTargetUI)))
		//	CRASH("Failed to Add RootUI to UI_Manager.");
		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, strLayertag_UI, pTargetUI)))
			CRASH("Failed to Add RootUI to Object_Manager.");
	}

	CUI_Text_Damage::TEXT_UI_TIMED_DESC tDesc = {};
	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_Text_Damage"),
		iDestLevel, TEXT("Layer_Custom_UI_Text_Damage"), TEXT("Pool_Text_Damage"), 50, &tDesc)))
		CRASH("Failed Ready Text_Damage");

	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_Button_Interact"),
		iDestLevel, TEXT("Layer_Custom_UI_Button_Interact"), TEXT("Pool_Button_Interact"), 1)))
		CRASH("Failed Ready Button_Interact");

	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_LockOn"),
		iDestLevel, TEXT("Layer_Custom_UI_LockOn"), TEXT("Pool_Button_LockOn"), 1)))
		CRASH("Failed Ready LockOn");

	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_Parry"),
		iDestLevel, TEXT("Layer_Custom_UI_Parry"), TEXT("Pool_Image_Parry"), 1)))
		CRASH("Failed Ready Parry");

	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_MobHPBar"),
		iDestLevel, TEXT("Layer_Custom_UI_MobHPBar"), TEXT("Pool_Image_MobHPBar"), 1)))
		CRASH("Failed Ready MobHPBar");

	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_TabUtility"),
		iDestLevel, TEXT("Layer_Custom_UI_TabUtility"), TEXT("Pool_Custom_TabUtility"), 1)))
		CRASH("Failed Ready TabUtility");

	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_GrafflePoint"),
		iDestLevel, TEXT("Layer_Custom_UI_GrafflePoint"), TEXT("Pool_Custom_GrafflePoint"), 50)))
		CRASH("Failed Ready GrafflePoint");


	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_Ovfl_Palette"),
		iDestLevel, TEXT("Layer_Custom_UI_Ovfl_Palette"), TEXT("Pool_Custom_Ovfl_Palette"), 1)))
		CRASH("Failed Ready Ovfl_Palette");


	m_pGameSystem->PreAssign_TargetUIs();
	// _UI
}

void CLevel_Test::Ready_Scene()
{
	// Camera
	CCamera::CAMERA_DESC CameraDesc = {};
	CameraDesc.fFovy = XMConvertToRadians(60.f);
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.f;
	CameraDesc.vEye = _float4(-1.019107f, 5.458634f, -15.936163f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.f);
	CameraDesc.fMouseSensor = 0.004f;
	if (FAILED(m_pGameInstance->Add_Camera(ENUM_CLASS(LEVEL::TEST), TEXT("Scene"), ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_SceneCamera"), &CameraDesc)))
		CRASH("SceneCamera");
}

void CLevel_Test::Ready_RopeAnchor()
{
	_float3 vScale{}, vRotation{}, vPosition{};
	vScale = { 1.f, 1.f, 1.f };
	vRotation = { 0.f, 0.f, 0.f };
	vPosition = { 0.f, 1.f, 50.f };
	CRopeAnchor::ROPEOBJECT_DESC Desc{};
	Desc.vScale = vScale;
	Desc.vRotation = vRotation;
	Desc.vPosition = vPosition;
	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 10.f;
	Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxMesh"));
	Desc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_RopeAnchor"));

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_RopeAnchor"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_RopeAnchor"), &Desc)))
		CRASH("Failed Ready Player");

	vPosition = { 0.f, 1.f, 55.f };
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_RopeAnchor"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_RopeAnchor"), &Desc)))
		CRASH("Failed Ready Player");
}

void CLevel_Test::Testing_UI(_float fTimeDelta)
{
#ifdef KSTA_UITEST_OLD

	_uint iDestLevel = ENUM_CLASS(m_eCurLevel);
	static _bool isInitialized = false;
	
	static CUI_Text* testText = nullptr;

	CUI_Text_Damage::TEXT_UI_TIMED_DESC tDesc = {};
	tDesc.isInstance = true;
	tDesc.vecInstanceDescs = {};

	tDesc.iShaderFlag = ENUM_CLASS(FONT_FLAG::FL_OUTLINE) | ENUM_CLASS(FONT_FLAG::FL_ALPHA_EDITABLE);
	tDesc.vColor = _float4{ 0.0f, 0.0f, 1.0f, 1.0f };
	tDesc.vOutlineColor = _float4{ 0.0f, 1.0f, 1.0f, 1.0f };
	tDesc.fFontOutlineWidth = 2.f;

	tDesc.strFontTag = L"WW_SemiBold";
	tDesc.strText = L"Test 테스트입니다.";
	tDesc.vScreenPos = _float2{ 0.f, 0.f }; // _float2{ 500.f, 500.f };
	tDesc.fScale = 0.25f;
	tDesc.vLifeTime = { 0.f, 10.f };
	tDesc.strUIName = L"TestFont";

	tDesc.iPassType = 0;

	tDesc.isTargetExist = true;
	tDesc.vTargetWorldPos = _float4{ 2.42f, -10.19f, -3.56f, 1.0f };


	if (!isInitialized)
	{
		// Test Initializing
		isInitialized = true;
		
		//if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_Text_Test",
		//	CUI_Text::Create(m_pDevice, m_pContext))))
		//	CRASH("프로토타입 못만들었대~~");



		//m_pGameInstance->Spawn_PoolingObject(L"Pool_Text_Damage", _fmatrix(), &tDesc);
		// ===== test

		// =====



		//CUI_Text* pTextObj = dynamic_cast<CUI_Text*>w
		//	(m_pGameInstance->Clone_Prototype(iDestLevel, L"Prototype_GameObject_Custom_Text_Test", PROTOTYPE::GAMEOBJECT, &tDesc));
		//if (!pTextObj)
		//	CRASH("폰트오브젝트 못만들었대~~");
		//
		//
		//m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, L"Layer_UI_Font", pTextObj);
		//testText = pTextObj;
	}


	_float fRandX = m_pGameInstance->Rand(-5.f, 5.f);
	_float fRandY = m_pGameInstance->Rand(-5.f, 5.f);
	_float fRandZ = m_pGameInstance->Rand(-5.f, 5.f);

	tDesc.vTargetWorldPos = {
		tDesc.vTargetWorldPos.x + fRandX,
		tDesc.vTargetWorldPos.y + fRandX,
		tDesc.vTargetWorldPos.z + fRandZ,
		tDesc.vTargetWorldPos.w
	};

	
	static _float fElapsedTime_TestSpawn = 0.f;
	fElapsedTime_TestSpawn += fTimeDelta;
	const _float fTestSpawnSpace = 5.f;
	if (fElapsedTime_TestSpawn >= fTestSpawnSpace)
	{
		fElapsedTime_TestSpawn = 0.f;

		const _float fOffsetY = 5.f;
		m_pGameSystem->Render_Damage(
			_float4{ 2.42f, -10.19f + fOffsetY, -3.56f, 1.0f },
			static_cast<_uint>(m_pGameInstance->Rand(100.f, 50000.f)),
			static_cast<TEXT_COLOR_TYPE>(m_pGameInstance->Rand(1.f, 4.999f)),
			3.f
		);

		m_pGameInstance->Spawn_PoolingObject(L"Pool_Text_Damage", _fmatrix(), &tDesc);
	}



	//CUI_Text::TEXT_UI_DESC tDesc = testText->Get_TextUIDesc();
		
	//_float4x4 matPipelineView = *m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW);
	//_float4x4 matPipelineProj = *m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ);
	//
	//_float4 world = { tDesc.vTargetWorldPos.x, tDesc.vTargetWorldPos.y, tDesc.vTargetWorldPos.z, 1.f };   // (x,y,z)
	//_matrix view = XMLoadFloat4x4(&matPipelineView);
	//_matrix proj = XMLoadFloat4x4(&matPipelineProj);
	//_vector pos = XMVectorSet(world.x, world.y, world.z, 1.0f);
	//
	//pos = XMVector3Transform(pos, view);
	//pos = XMVector3Transform(pos, proj);
	//_vector ndc = pos / XMVectorSplatW(pos);
	//
	//_float3 ndc3;
	//XMStoreFloat3(&ndc3, ndc);
	//_float screenX = (ndc3.x * 0.5f + 0.5f) * 1920.f;     // 화면 해상도 X
	//_float screenY = (1.0f - (ndc3.y * 0.5f + 0.5f)) * 1080.f; // Y 반전
	//
	//tDesc.vScreenPos = _float2(screenX, screenY);

	//testText->Set_TextUIDesc(tDesc);







#endif // KSTA_UITEST_OLD

	// interact
#pragma region [NUMPAD +] KSTA_UITEST_INTERACT

	static _bool isPrinted_FirstInfoMsg = false;

	if (!isPrinted_FirstInfoMsg)
	{
		std::cout << "[Level_Test::Testing_UI] If you want to enable UI Test, Press [Ctrl + I]." << std::endl;
		std::cout << "[Level_Test::Testing_UI] Default is Disabled Mode." << std::endl;

		isPrinted_FirstInfoMsg = true;
	}



	static _bool isEnableUITest = false;
	
	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) == KEYSTATE::PRESS &&
		m_pGameInstance->Get_DIKeyState(DIK_I) == KEYSTATE::DOWN)
	{
		isEnableUITest = !isEnableUITest;

		if (isEnableUITest)
		{
			std::cout << "[Level_Test::Testing_UI] UI Testing Enabled." << std::endl;
			std::cout << "[Level_Test::Testing_UI] \t[NUMPAD4] MobHP, \t[NUMPAD1] MobHP -10, \t[NUMPAD2] MobHP +10: " << std::endl;
			std::cout << "[Level_Test::Testing_UI] \t[NUMPAD+] Interact, \t[NUMPAD6] Parry, \t[NUMPAD.] LockOn" << std::endl;
			std::cout << "[Level_Test::Testing_UI] \t[TAB] TabUI(Hold), \t[NUMPAD5] Overflowing Palette" << std::endl;
		}

		if (!isEnableUITest)	std::cout << "[Level_Test::Testing_UI] UI Testing Disabled." << std::endl;
	}



	if (!isEnableUITest) return;



	
	static _uint iInteractIndex = 0;
	enum INTERACT_INDEX { TEST_INTERACT0, TEST_INTERACT1, TEST_INTERACTEND };


	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPADPLUS) == KEYSTATE::DOWN &&
		(m_pGameInstance->Find_UIObject(L"UI_Interact") == nullptr || m_pGameInstance->Find_UIObject(L"UI_Interact")->IsActivate() == false))
	{
		switch (iInteractIndex)
		{
		case TEST_INTERACT0:
			m_pGameSystem->Show_InteractUI(L"테스트하나");
			iInteractIndex++;
			if (iInteractIndex >= TEST_INTERACTEND) iInteractIndex = 0;
			break;
		case TEST_INTERACT1:
			m_pGameSystem->Show_InteractUI(L"테스트둘");
			iInteractIndex++;
			if (iInteractIndex >= TEST_INTERACTEND) iInteractIndex = 0;
			break;
		}
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_NUMPADPLUS) == KEYSTATE::DOWN &&
		(m_pGameInstance->Find_UIObject(L"UI_Interact") != nullptr || m_pGameInstance->Find_UIObject(L"UI_Interact")->IsActivate() == true))
	{
		m_pGameSystem->Hide_InteractUI(true);
	}


	if (m_pGameSystem->Get_InteractUI_Feedback(UI_EVENT_TYPE::CLICK_ENTER))
		cout << "[Level_Test::Testing_UI] 눌렸음!!" << endl;
	if (m_pGameSystem->Get_InteractUI_Feedback(UI_EVENT_TYPE::HOVER_ENTER))
		cout << "[Level_Test::Testing_UI] 마우스올라감" << endl;
	if (m_pGameSystem->Get_InteractUI_Feedback(UI_EVENT_TYPE::HOVER_EXIT))
		cout << "[Level_Test::Testing_UI] 마우스내려감" << endl;

#pragma endregion


#pragma region [NUMPAD .] KSTA_UITEST_LOCKON
	CCustom_UI* pRootUILockOn = m_pGameSystem->Find_RootUI(L"UI_LockOn");

	if (m_pGameInstance->Get_DIKeyState(DIK_DECIMAL) == KEYSTATE::DOWN &&
		!pRootUILockOn->IsActivate())
		m_pGameSystem->Attach_LockOnUI(nullptr);
	else if (m_pGameInstance->Get_DIKeyState(DIK_DECIMAL) == KEYSTATE::DOWN &&
		pRootUILockOn->IsActivate())
		m_pGameSystem->Detach_LockOnUI();
#pragma endregion


#pragma region [NUMPAD 6] KSTA_UITEST_PARRY
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD6) == KEYSTATE::DOWN)
	{
		if (m_pGameInstance->Find_UIObject(L"UI_Parry")->IsActivate() == true)
			static_cast<CUI_Parry*>(m_pGameInstance->Find_UIObject(L"UI_Parry"))->Enable_Parried();

		m_pGameInstance->Spawn_PoolingObject(L"Pool_Image_Parry", _fmatrix(), nullptr);
	}
#pragma endregion


#pragma region [NUMPAD 4] KSTA_UITEST_MOBHPBAR
	//static _bool isActiveMobHPBar = false;
	//if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD5) == KEYSTATE::DOWN)
	//	isActiveMobHPBar = !isActiveMobHPBar;

	//if (isActiveMobHPBar)
		
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD4) == KEYSTATE::DOWN)
	{
		if (m_pGameInstance->Find_UIObject(L"UI_MobHPBar")->IsActivate() == true)
			m_pGameInstance->Find_UIObject(L"UI_MobHPBar")->SetActivate(false);
		else
			m_pGameInstance->Spawn_PoolingObject(L"Pool_Image_MobHPBar", _fmatrix(), nullptr);
	}

	//if (isActiveMobHPBar)
	//{
	//	UI_MOBINFO_DESC tTmpDesc = {};
	//
	//	const _uint iNumTestMobs = 3;
	//
	//	for (_uint i = 0; i < iNumTestMobs; i++)
	//	{
	//		_float3 fTestOffset = {
	//			m_pGameInstance->Rand(-10.f, 10.f),
	//			m_pGameInstance->Rand(-10.f, 10.f) - 10.f,
	//			m_pGameInstance->Rand(-10.f, 10.f)
	//		};
	//
	//		tTmpDesc.vMobPos = fTestOffset;
	//		tTmpDesc.fMobCurHP = 50.f;
	//		tTmpDesc.fMobCurHP = 70.f;
	//
	//		m_pGameSystem->Update_MobStatus(tTmpDesc);
	//	}
	//}

#pragma endregion


#pragma region [TAB] KSTA_UITEST_TABUTILITY
	static _bool isTabUtilityActive = false;
	static _uint iTmpSelectedUtility = ENUM_CLASS(UI_TAB_UTILITY::NOTHING);

	//_uint iTabUtilitySelectedIndex = UINT_MAX;
	_bool isTabUtilityHided = false;

	if (!isTabUtilityActive &&
		m_pGameInstance->Get_DIKeyState(DIK_TAB) == KEYSTATE::DOWN)
	{
		m_pGameSystem->Show_TabUtilityUI(iTmpSelectedUtility);
		isTabUtilityActive = true;
	}
	else if (isTabUtilityActive &&
		m_pGameInstance->Get_DIKeyState(DIK_TAB) == KEYSTATE::UP)
	{
		iTmpSelectedUtility = m_pGameSystem->HideNGet_TabUtilityUI();
		isTabUtilityActive = false;
		isTabUtilityHided = true;
	}


	_string strSelectedUtilityName = {};
	if (isTabUtilityHided)
	{
		switch (iTmpSelectedUtility)
		{
		case ENUM_CLASS(Client::UI_TAB_UTILITY::GRAPPLE):			strSelectedUtilityName = "GRAPPLE";		break;
		case ENUM_CLASS(Client::UI_TAB_UTILITY::SENSOR):			strSelectedUtilityName = "SENSOR";		break;
		case ENUM_CLASS(Client::UI_TAB_UTILITY::FLIGHT):			strSelectedUtilityName = "FLIGHT";		break;
		case ENUM_CLASS(Client::UI_TAB_UTILITY::LEVITATOR):			strSelectedUtilityName = "LEVITATOR";	break;
		case ENUM_CLASS(Client::UI_TAB_UTILITY::NOTHING):			strSelectedUtilityName = "NOTHING";		break;
		}

		std::cout << "[CLevel_Test::Testing_UI] : Tab Utility Returned : " << strSelectedUtilityName << std::endl;
	}

#pragma endregion


#pragma region [NUMPAD 5] KSTA_UITEST_OVERFLOWINGPALETTE 

	static _bool isOpenOverflowingPalette = false;
	static _uint iTargetLevel = 0;
	const _uint iMaxNumLevel = 5;

	if		(m_pGameInstance->Get_DIKeyState(DIK_LEFTARROW) == KEYSTATE::DOWN)
	{
		iTargetLevel = (iTargetLevel <= 0) ? iTargetLevel : iTargetLevel - 1;
		std::cout << "[Level_Test::Testing_UI] TargetLevel : " << iTargetLevel << std::endl;
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_RIGHTARROW) == KEYSTATE::DOWN)
	{	
		iTargetLevel = (iTargetLevel >= iMaxNumLevel - 1) ? iTargetLevel : iTargetLevel + 1;
		std::cout << "[Level_Test::Testing_UI] TargetLevel : " << iTargetLevel << std::endl;
	}

	if (!isOpenOverflowingPalette &&
		m_pGameInstance->Get_DIKeyState(DIK_NUMPAD5) == KEYSTATE::DOWN)
	{
		m_pGameSystem->Open_Game_OverflowPalette(iTargetLevel);
		isOpenOverflowingPalette = true;
	}
	else if (isOpenOverflowingPalette &&
		m_pGameInstance->Get_DIKeyState(DIK_NUMPAD5) == KEYSTATE::DOWN)
	{
		m_pGameSystem->Close_Game_OverflowPalette();
		isOpenOverflowingPalette = false;
	}

#pragma endregion


#pragma region [NUMPAD 0] KSTA_UITEST_GRAFFLEPOINT

	static _bool isInitialized_GrafflePoint = false;

	_uint iNumGraffleUI = 50;

	if (!isInitialized_GrafflePoint)
	{
		for (_uint i = 0; i < iNumGraffleUI; i++)
		{
			m_pGameInstance->Spawn_PoolingObject(L"Pool_Custom_GrafflePoint", _fmatrix(), nullptr);
		}

		isInitialized_GrafflePoint = true;
	}

#pragma endregion

}

#ifdef _DEBUG
void CLevel_Test::Shader_Gui()
{

}
#endif

void CLevel_Test::Toggle_HUD()
{
	static _bool isToggled_HUD = false;
	static _bool isToggled_BOSSHP = false;

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD9) == KEYSTATE::DOWN)
	{
		isToggled_HUD = !isToggled_HUD;

		if (isToggled_HUD)
		{
			CGameSystem::GetInstance()->HUD_FadeOut();
		}
		else
		{
			CGameSystem::GetInstance()->HUD_FadeIn();
		}
	}


	//if (m_pGameInstance->Get_DIKeyState(DIK_NUMPADENTER) == KEYSTATE::DOWN)
	//{
	//	isToggled_BOSSHP = !isToggled_BOSSHP;
	//	CGameSystem::GetInstance()->HUD_Toggle_BossStatusUI(isToggled_BOSSHP);
	//}
}

HRESULT CLevel_Test::Ready_Layer_Map(const _char* pFilePath)
{
    _char FileDrive[MAX_PATH] = {};
    _char FileDir[MAX_PATH] = {};
    _char FileName[MAX_PATH] = {};
    _char FileExt[MAX_PATH] = {};

    _splitpath_s(pFilePath, FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

    _string PasingDir = FileDir;
    if (strlen(FileName) > 0)
    {
        PasingDir += FileName;
        PasingDir += FileExt;
        Read_Map_Dat(pFilePath);
    }
    else
    {
        for (const auto& entry : filesystem::recursive_directory_iterator(FileDir)) {
            if (!entry.is_regular_file())
                continue;

            if (entry.path().string().find("Prototype") != std::string::npos)
                continue;

            if (entry.path().extension() != ".dat")
                continue;

            _string strFilePath = entry.path().string();
            Read_Map_Dat(strFilePath);
        }
    }
    return S_OK;
}

void CLevel_Test::Read_Map_Dat(const _string pFilePath)
{
    ifstream File(pFilePath, ios::binary);

    if (!File.is_open())
    {
        MSG_BOX("Load Failed");
    }
    if (pFilePath.find("Instance") != std::string::npos)
    {
        return;

        //CMapObject::MAP_LOAD Desc{};

        //_matrix PreTransformMatrix = XMMatrixIdentity();
        //_float fSize = 0.01f;
        //PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

        //CMapObject_Instance::MAP_LOAD Desc{};
        //Desc.iNumInstance;
        //Desc.ModelName;
        //Desc.m_WolrdPos;
        //Desc.WorldMatrix;

        //while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
        //{
        //    memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
        //    File.read(Desc.ModelName, NameLength);

        //    File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
        //    File.read(reinterpret_cast<char*>(&Desc.m_WolrdPos), sizeof(_float4));

        //    File.read(reinterpret_cast<char*>(&Desc.iNumInstance), sizeof(_uint));
        //    _float4x4* pMatrix = new _float4x4[Desc.iNumInstance];
        //    File.read(reinterpret_cast<char*>(pMatrix), sizeof(_float4x4) * Desc.iNumInstance);

        //    Safe_Delete_Array(pMatrix);

        //    _wstring PrototypeName = TEXT("Prototype_Component_Model_Instance_");

        //    //프로토타입은 제일 큰 놈으로 들어옴. => 0번까지 계속 생성.
        //    _wstring ModelName = StringToWString(Desc.ModelName);


        //    ModelName.pop_back();
        //    _string ProjectPath = filesystem::current_path().parent_path().parent_path().string();
        //    ProjectPath += "/Client/Bin/Resource/Map";
        //    for (const auto& entry : filesystem::recursive_directory_iterator(ProjectPath)) {
        //        if (entry.is_regular_file()) {
        //            if (entry.path().string().find("json") != std::string::npos)
        //                continue;
        //            if (entry.path().string().find(WStringToString(ModelName)) != std::string::npos)
        //            {
        //                _string ModelPath = entry.path().string();

        //                m_pGameInstance->Add_Work([=, Model = PrototypeName + StringToWString(entry.path().stem().string()), Path = ModelPath]() {
        //                    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), Model,
        //                        CModel_Instance::Create(pDevice, pContext, PreTransformMatrix, Path.c_str()))))
        //                        CRASH("Failed");
        //                    });
        //            }
        //        }
        //    }
        //}

    }
    else
    {
        _uint NameLength;

        _matrix PreTransformMatrix = XMMatrixIdentity();
        _float fSize = 0.01f;
        PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

        CMapObject::MAP_LOAD Desc{};

        _wstring PrototypeName = TEXT("Prototype_Component_Model_");

		while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
		{
			memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
			File.read(Desc.ModelName, NameLength);

			File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
			File.read(reinterpret_cast<char*>(&Desc.eObjectType), sizeof(OBJECTTYPE));
			_float4x4 Matrix = {};
			File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
			Desc.WorldMatrix = &Matrix;

			//프로토타입은 제일 큰 놈으로 들어옴. => 0번까지 계속 생성.
			_wstring ModelName = StringToWString(Desc.ModelName);

			m_pGameInstance->Add_Work([&, ModelName = string(Desc.ModelName), ShaderPass = Desc.iShaderPassIndex, eObjectType = Desc.eObjectType, Matrix = *Desc.WorldMatrix]() mutable {
				CMapObject::MAP_LOAD pDesc{};
				strcpy_s(pDesc.ModelName, ModelName.c_str());
				pDesc.iShaderPassIndex = ShaderPass;
				pDesc.eObjectType = eObjectType;
				pDesc.WorldMatrix = &Matrix;

				CMapObject* pMapObject = static_cast<CMapObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject")
					, PROTOTYPE::GAMEOBJECT, &pDesc));

				//m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Layer_Map"), pMapObject);
				//Safe_AddRef(pMapObject);
				//m_pGameInstance->Add_To_OctoTree(pMapObject)

				//if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject"), ENUM_CLASS(m_eCurLevel), TEXT("Layer_Map"), &pDesc)))
				//	CRASH("Map Object");
				//});
				});
        }
    }
    m_pGameInstance->Wait_Thread_End();
    File.close();
}

CLevel_Test* CLevel_Test::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Test* pInstance = new CLevel_Test(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Test");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Test::Free()
{
    __super::Free();
    Safe_Release(m_pGameSystem);
	
}
