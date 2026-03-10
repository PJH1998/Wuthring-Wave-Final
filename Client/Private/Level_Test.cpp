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
#include "Levi_Alter.h"
#include "Levi_Ray.h"
#include "Levi_Anchor.h"
#include "Levi_Drop.h"
#include "Levi_Wave.h"
#pragma endregion

#include "Player.h"
#include "SequencePlayer.h"
#include "SequenceLupa.h"

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
#include "UI_GrapplePoint.h"
#include "UI_QTE.h"
#include "UI_CurveTrace.h"

#include "NPC_Griffin.h"
#include "DummyNPC.h"
#include "NPC_Hiding.h"
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
    Ready_Layer_SequnecePlayer();
	//Ready_Dummy();
	//Ready_MonsterTest();
	Ready_CoroSaurus();
	//Ready_HavocWarrior();
	//Ready_ElectroPredator();
	//Ready_Spawner();
	//Ready_AnimInstanceTest();
	
	//Ready_NPC();

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

	//Ready_Leviatan();

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
	if (m_pGameInstance->Get_DIKeyState(DIK_F2) == KEYSTATE::DOWN)
	{
		m_pGameInstance->Set_LightActive(TEXT("Test"), true);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_F3) == KEYSTATE::DOWN)
	{
		m_pGameInstance->Spawn_PoolingObject_ForStatic(TEXT("Pooling_Excute_Prefab"), XMMatrixIdentity(), nullptr);
	}
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

void CLevel_Test::Ready_Layer_SequnecePlayer()
{
	_float3 vScale{}, vRotation{}, vPosition{};
	vScale = { 1.f, 1.f, 1.f };
	vRotation = { 0.f, 0.f, 0.f };
	vPosition = { 0.f, -10.f, 50.f };

	CSequencePlayer::SEQUENCEPLAYER_DESC Desc{};
	Desc.eCurLevel = m_eCurLevel;
	Desc.vScale = vScale;
	Desc.vRotation = vRotation;
	Desc.vPosition = vPosition;
	Desc.iPlayerCount = CSequencePlayer::SEQUENCECHARACTER::SEQUENCE_END;

	// 0. vector 크기 정의
	Desc.PlayerSpecs.resize(CSequencePlayer::SEQUENCECHARACTER::SEQUENCE_END);

	// 1. Yuno(주인공) 캐릭터 정의
	Desc.PlayerSpecs[CSequencePlayer::SEQUENCECHARACTER::YUNO].CharacterDesc = SeqPlayerData::GetYunoCloneData(vScale, vRotation, vPosition, m_eCurLevel);
	Desc.PlayerSpecs[CSequencePlayer::SEQUENCECHARACTER::YUNO].strActorTag = TEXT("Prototype_GameObject_Actor_Yuno");

	Desc.PlayerSpecs[CSequencePlayer::SEQUENCECHARACTER::AUGUSTA].CharacterDesc = SeqPlayerData::GetSequenceAugustaCloneData(vScale, vRotation, vPosition, m_eCurLevel);
	Desc.PlayerSpecs[CSequencePlayer::SEQUENCECHARACTER::AUGUSTA].strActorTag = TEXT("Prototype_GameObject_Actor_SequenceAugusta");

	Desc.PlayerSpecs[CSequencePlayer::SEQUENCECHARACTER::LUPA].CharacterDesc = SeqPlayerData::GetSequenceLupaCloneData(vScale, vRotation, vPosition, m_eCurLevel);
	Desc.PlayerSpecs[CSequencePlayer::SEQUENCECHARACTER::LUPA].strActorTag = TEXT("Prototype_GameObject_Actor_SequenceLupa");

	// 2. Sequence Player(Character 모음) 생성.
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_SequencePlayer"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_SequencePlayers"), &Desc)))
		CRASH("Failed Ready Sequence Player");
}

void CLevel_Test::Ready_Dummy()
{

	//m_pGameSystem->Create_MonsterDummy(LEVEL::TEST, _float3(0.f, 5.f, 50.f), XMMatrixScaling(0.0001f, 0.0001f, 0.0001f));
	
	CPatternDummy::PAT_DUMMYDESC DummyDesc{};
	DummyDesc.eLevel = m_eCurLevel;
	DummyDesc.eType = CPatternDummy::MODEL_TYPES::DEFAULT;
	//DummyDesc.strModelTag = TEXT("Prototype_Component_Model_FalseSovereign");				//신왕
	//DummyDesc.strInitAnimTag = "Stand1";
	//DummyDesc.strFolderPath = "../Bin/Resource/Model/Monster/FalseSovereign/Notify";
	
	//DummyDesc.strModelTag = TEXT("Prototype_Component_Model_CoroSaurus");					//코로
	//DummyDesc.strInitAnimTag = "Stand";
	//DummyDesc.strFolderPath = "../Bin/Resource/Model/Monster/CorroSaurus/Notify";
	//DummyDesc.eType = CPatternDummy::MODEL_TYPES::WEAPON;
	//DummyDesc.strPartTag = TEXT("Prototype_Component_Model_CoroRock");
	//DummyDesc.strBoneName = "Bone_WeaponProp001";
	//DummyDesc.vOffsetPos = _float3(2.5f, 0.f, 0.f);
	//DummyDesc.vOffsetRot = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(-90.f));
	 
	//DummyDesc.strModelTag = TEXT("Prototype_Component_Model_Ggobul");						//꼬불이
	//DummyDesc.strInitAnimTag = "SAttack01_1";
	//DummyDesc.strFolderPath = "../Bin/Resource/Model/Monster/Ggobul/Notify";
	// 
	//DummyDesc.strModelTag = TEXT("Prototype_Component_Model_Scythe");						//촉수
	//DummyDesc.strFolderPath = "../Bin/Resource/Model/Monster/FS_Scythe/Notify";
	//DummyDesc.strInitAnimTag = "Stand1";

	DummyDesc.strModelTag = TEXT("Prototype_Component_Model_Levi_Alter");					// 레비아탄(분신 모델, 본체와 애니메이션 동일)
	DummyDesc.strInitAnimTag = "Stand2";
	//DummyDesc.strFolderPath = "../Bin/Resource/Model/Monster/Levi_Alter/Notify";			// 레비아탄(분신 Notify)
	DummyDesc.strFolderPath = "../Bin/Resource/Model/Monster/Leviatan/Notify";				// 레비아탄(본체 Notify)
	DummyDesc.eType = CPatternDummy::MODEL_TYPES::WEAPON;
	DummyDesc.strPartTag = TEXT("Prototype_Component_Model_Leviatan_Bayonet");
	DummyDesc.strBoneName = "WeaponProp02";
	DummyDesc.vOffsetPos = _float3(0.f, 0.f, 0.f);
	DummyDesc.vOffsetRot = _float3(XMConvertToRadians(90.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));

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
	tDesc.pAnimationTag = "PatrolToFight";
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
	CoroDesc.pAnimationTag = "burst01_5";
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
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/Leviatan", m_eCurLevel, 10);
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
	

	CNPC_Griffin::GRIFFIN_DESC Desc{};
	Desc.eCurLevel = m_eCurLevel;
	Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	Desc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_NPCGriffin"));
	Desc.pAnimMachineTag = TEXT("Prototype_Component_AnimMachine_NPCGriffin");
	Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));

	m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Griffin"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Z_Test"), &Desc);

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
		CRASH("Failed Ready Leviatan");

	CLevi_Alter::ALTER_DESC AlterDesc{};
	AlterDesc.eCurLevel = m_eCurLevel;
	AlterDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	AlterDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	AlterDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_Levi_Alter"));
	AlterDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	AlterDesc.fRotationPerSec = XMConvertToRadians(90.f);
	AlterDesc.fSpeedPerSec = 10.f;
	AlterDesc.strFolderPath = "../Bin/Resource/Model/Monster/Levi_Alter/Notify";
	AlterDesc.fAttackDmg = pInfo->fAttack;
	AlterDesc.vDetectRange = _float3(35.f, 20.f, 35.f);

	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Alter"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), TEXT("Pool_LeviAlter"), 8, &AlterDesc)))
		CRASH("Failed Ready Alter");

	CLevi_Ray::LEVIRAY_DESC RayDesc{};
	RayDesc.eType = TEXT_COLOR_TYPE::DARK;
	RayDesc.fLifeTime = 1.f;
	RayDesc.fAttackDamage = pInfo->fAttack;
	RayDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	RayDesc.iTargetLayer = ENUM_CLASS(COLLISIONLAYER::PLAYER);
	RayDesc.vExtent = _float3(1.f, 1.f, 100.f);
	//RayDesc.wstrEffectTag = ;

	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Ray"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), TEXT("Pool_LeviRay_S"), 16, &RayDesc)))
		CRASH("Failed Ready Ray");

	RayDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_SKILL);
	RayDesc.fAttackDamage = pInfo->fAttack * 1.5f;
	RayDesc.vExtent = _float3(2.f, 2.f, 100.f);
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Ray"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), TEXT("Pool_LeviRay_B"), 2, &RayDesc)))
		CRASH("Failed Ready Ray");

	CProjectile::PROJECTILEDESC Projectile{};
	Projectile.fAttackDamage = pInfo->fAttack;
	Projectile.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	Projectile.iTargetLayers = { ENUM_CLASS(COLLISIONLAYER::PLAYER),ENUM_CLASS(COLLISIONLAYER::MAP) };
	Projectile.fRadius = 0.7f;
	Projectile.fSpeedPerSec = 15.f;
	Projectile.wstrModelTag = TEXT("Prototype_Component_Model_Leviatan_Projectile");
	Projectile.eType = TEXT_COLOR_TYPE::DARK;
	Projectile.isCollisionDestroy = false;
	Projectile.wstrEffectTag = TEXT("Leviatan_Dg2");
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Projectile"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Projectile"), TEXT("Pool_Projectile_LeviSword"), 4, &Projectile)))
		CRASH("Failed Ready Projectile (Leviatan)");

	Projectile.wstrModelTag = TEXT("Prototype_Component_Model_Leviatan_SwordAura");
	Projectile.wstrEffectTag = TEXT("Leviatan_Dg");
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Projectile"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Projectile"), TEXT("Pool_Projectile_LeviAura"), 4, &Projectile)))
		CRASH("Failed Ready Projectile (Leviatan)");

	CLevi_Anchor::ANCHORDESC Anchor{};
	Anchor.fSpeedPerSec = 10.f;
	Anchor.fAttackDamage = pInfo->fAttack;
	//Anchor.wstrEffectTag = ;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Anchor"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Projectile"), TEXT("Pool_LeviAnchor"), 4, &Anchor)))
		CRASH("Failed Ready Projectile (Leviatan)");

	CLevi_Drop::DROPDESC Drop{};
	Drop.fAttackDamage = pInfo->fAttack * 0.5f;
	Drop.fSpeedPerSec = 10.f;
	//Drop.wstrEffectTag = ;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Drop"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Projectile"), TEXT("Pool_LeviDrop"), 8, &Drop)))
		CRASH("Failed Ready Projectile (Leviatan)");

	CLevi_Wave::WAVEDESC Wave{};
	Wave.fAttackDamage = pInfo->fAttack;
	Wave.fSpeedPerSec = 15.f;
	//Wave.wstrEffectTag = ;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Wave"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Projectile"), TEXT("Pool_LeviWave"), 4, &Wave)))
		CRASH("Failed Ready Projectile (Leviatan)");
}

void CLevel_Test::Ready_NPC()
{
	CNPC_Hiding::HIDINGDESC NPCDesc{};
	NPCDesc.eCurLevel = m_eCurLevel;
	NPCDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	NPCDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	NPCDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_FemaleS370437"));
	NPCDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	//NPCDesc.strFolderPath = "../Bin/Resource/Model/Monster/ElectroPredator/Notify";
	NPCDesc.isCollide = true;
	NPCDesc.fRotationPerSec = XMConvertToRadians(90.f);
	NPCDesc.fSpeedPerSec = 1.f;
	NPCDesc.vInitPos = _float3(3.f, -8.f, -33.f);
	NPCDesc.vInitRot = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	NPCDesc.pAnimMachineTag = TEXT("Prototype_Component_AnimMachine_NPC_Hiding");
	NPCDesc.pAnimationTag = "sing02_Loop";

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_NPC_Hiding"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_NPC"), &NPCDesc)))
		CRASH("Failed Ready NPC_Hiding");
}

void CLevel_Test::Ready_UI()
{
	// UI
	const   _uint       iDestLevel = ENUM_CLASS(m_eCurLevel);
	const _wstring strLayertag_UI = L"Layer_Custom_UI";
	const _wstring strPrototypeTag_UI[] = {
		 L"Prototype_GameObject_Custom_UI_Container_HUD",
		 L"Prototype_GameObject_Custom_UI_Container_HUD_Sector_Minimap",
		 L"Prototype_GameObject_Custom_UI_Container_HUD_Sector_FuncIcons",
		 L"Prototype_GameObject_Custom_UI_Container_FinalEnd",
		 L"Prototype_GameObject_Custom_UI_Container_QuestIndicator"
	};
	for (auto& strPrototypeTag : strPrototypeTag_UI)
	{
		CUIObject* pTargetUI = static_cast<CUIObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, strPrototypeTag, PROTOTYPE::GAMEOBJECT));
		
		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, strLayertag_UI, pTargetUI)))
			CRASH("Failed to Add RootUI to Object_Manager.");
	}

	CUI_Text_Damage::TEXT_UI_TIMED_DESC tTimedTextDesc = {};
	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_Text_Damage"),
		iDestLevel, TEXT("Layer_Custom_UI_Text_Damage"), TEXT("Pool_Text_Damage"), 50, &tTimedTextDesc)))
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

	//if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_GrapplePoint"),
	//	iDestLevel, TEXT("Layer_Custom_UI_GrapplePoint"), TEXT("Pool_Custom_GrapplePoint"), 50)))
	//	CRASH("Failed Ready GrapplePoint");

	CUI_QTE::UI_QTE_DESC tQTEDesc = {};
	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_QTE"),
		iDestLevel, TEXT("Layer_Custom_UI_QTE"), TEXT("Pool_Image_QTE"), 1, &tQTEDesc)))
		CRASH("Failed Ready QTE");

	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_Dialog"),
		iDestLevel, TEXT("Layer_Custom_UI_Dialog"), TEXT("Pool_Custom_Dialog"), 1)))
		CRASH("Failed Ready Dialog");



	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_UI_CurveTrace"),
		iDestLevel, TEXT("Layer_Custom_UI_CurveTrace"), TEXT("Pool_Custom_CurveTrace"), 1)))
		CRASH("Failed Ready CurveTrace");

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

#pragma region [NUMPAD +] KSTA_UITEST_INTERACT


	static _bool isInteractActivate = false;
	
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPADPLUS) == KEYSTATE::DOWN)
		isInteractActivate = !isInteractActivate;
	

	_bool isInteracted = false;
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPADMINUS) == KEYSTATE::DOWN)
		isInteracted = true;
	if (isInteractActivate)
		m_pGameSystem->Req_Render_InteractUI(L"테스트입니다.", isInteracted);

	

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


#pragma region [INSTANT] KSTA_UITEST_GRAPPLEPOINT

	static _bool isInitialized_GrapplePoint = false;
	
	_uint iNumGrappleUI = 1;
	
	if (!isInitialized_GrapplePoint)
	{
		for (_uint i = 0; i < iNumGrappleUI; i++)
		{
			UI_GRAPPLE_TYPE eType = UI_GRAPPLE_TYPE::PULL; /* static_cast<UI_GRAPPLE_TYPE>(m_pGameInstance->Rand(0.f, 1.999f));*/
			_float3 vBasePos = { 0.f, -10.f, 0.f };
			_float vRandRange= 0.f;
			_float3 vTmpPos = {
				m_pGameInstance->Rand(-vRandRange, vRandRange) + vBasePos.x,
				m_pGameInstance->Rand(-vRandRange, vRandRange) + vBasePos.y,
				m_pGameInstance->Rand(-vRandRange, vRandRange) + vBasePos.z
			};

			m_pGameSystem->Create_GrapplePoint(vTmpPos, eType);

			//CUI_GrapplePoint::UI_GRAPPLEPOINT_DESC tDesc = { _float3(), eType};
			//m_pGameInstance->Spawn_PoolingObject(L"Pool_Custom_GrapplePoint", _fmatrix(), &tDesc);
		}
	
		isInitialized_GrapplePoint = true;
	}

	_float3 vPlayerPos = {}; XMStoreFloat3(&vPlayerPos, m_pGameSystem->Get_PlayerPosition());
	//_float fDistance = FLT_MAX;
	//auto pNearestGrapple = m_pGameSystem->Find_NearGrapplePoint(vPlayerPos, UI_GRAPPLE_TYPE::ANCHOR, &fDistance);
	//_float3 vNearGrapplePos = {}; XMStoreFloat3(&vNearGrapplePos, static_cast<CTransform*>(pNearestGrapple->Get_Component(L"Com_Transform"))->Get_State(STATE::POSITION));
	//_float fPullDistance = FLT_MAX;
	//auto pNearestPull = m_pGameSystem->Find_NearGrapplePoint(vPlayerPos, UI_GRAPPLE_TYPE::PULL, &fPullDistance);
	//_float3 vNearPullPos = {}; XMStoreFloat3(&vNearPullPos, static_cast<CTransform*>(pNearestPull->Get_Component(L"Com_Transform"))->Get_State(STATE::POSITION));
	//
	//std::cout << "[CLevel_Test::Testing_UI] Nearest Grapple UI Distance : " << fDistance << std::endl;
	//std::cout << "[CLevel_Test::Testing_UI] Nearest Pull    UI Distance : " << fPullDistance << std::endl;
	//std::cout << "==================================================================" << std::endl;

#pragma endregion	

	
#pragma region [LCTRL + I / LCTRL + O] KSTA_UITEST_DIALOG
	static _bool isUITestDialogOn = false;

	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) == KEYSTATE::PRESS &&
		m_pGameInstance->Get_DIKeyState(DIK_I) == KEYSTATE::DOWN)
	{
		isUITestDialogOn = !isUITestDialogOn;
		std::cout << "[Level_Test::Testing_UI] Dialog Toggled to" << (_bool)isUITestDialogOn << std::endl;

		if (isUITestDialogOn)
			m_pGameSystem->Open_DialogUI("../../Client/Bin/Resource/UI/Dialog/testdialog2.csv", true);
		else
			m_pGameSystem->Close_DialogUI();
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) == KEYSTATE::PRESS &&
		m_pGameInstance->Get_DIKeyState(DIK_O) == KEYSTATE::DOWN)
	{
		m_pGameSystem->Req_Interact_DialogUI(false);
	}

#pragma endregion


#pragma region  [NUMPAD 0] KSTA_UITEST_FINAL
	static _bool isFinalImageOn = false;

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD0) == KEYSTATE::DOWN)
	{
		isFinalImageOn = !isFinalImageOn;

		if (isFinalImageOn)
			m_pGameSystem->Trigger_PlayEndImage();
#ifdef _DEBUG
		else
			m_pGameSystem->Trigger_StopEndImageForcely();
#endif // _DEBUG

	}
#pragma endregion




#pragma region [LCTRL + NUMPAD1, 2, 3] KSTA_UITEST_QUEST

	static _bool isQuestUIOn = false;

	if (!isQuestUIOn &&
		m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) == KEYSTATE::PRESS &&
		m_pGameInstance->Get_DIKeyState(DIK_NUMPAD3) == KEYSTATE::DOWN)
	{
		isQuestUIOn = true;
		std::cout << "[Level_Test::Testing_UI] Quest On" << std::endl;
		m_pGameSystem->Trigger_ActivateQuest();
	}

	if (isQuestUIOn &&
		m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) == KEYSTATE::PRESS &&
		m_pGameInstance->Get_DIKeyState(DIK_NUMPAD2) == KEYSTATE::DOWN)
	{
#ifdef _DEBUG
		m_pGameSystem->Trigger_ForceCompleteQuestProgress();
#else
		m_pGameSystem->Trigger_AddQuestProgress();
#endif  
	}



#ifdef _DEBUG
	if (isQuestUIOn &&
		m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) == KEYSTATE::PRESS &&
		m_pGameInstance->Get_DIKeyState(DIK_NUMPAD1) == KEYSTATE::DOWN)
	{
		m_pGameSystem->Trigger_AllReset();
	}
#endif // _DEBUG

	

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
