#include "ClientPch.h"
#include "Level_Heaven.h"

#include "GameSystem.h"
#include "MonsterTest.h"
#include "Ggobul.h"
#include "FS_Scythe.h"
#include "HavocWarrior.h"
#include "ElectroPredator.h"
#include "Corosaurus.h"
#include "Projectile.h"
#include "AoEDoT.h"
#include "Leviatan.h"
#include "Levi_Alter.h"
#include "Levi_Ray.h"
#include "Levi_Anchor.h"
#include "Levi_Drop.h"
#include "Levi_Wave.h"

#include "Player.h"
#include "SequencePlayer.h"

#include "SkyBox.h"
#include "UI_Text_Damage.h"
#include "UI_QTE.h"

//SFX
#ifdef _DEBUG
#include "SonoraChange.h"
#endif

CLevel_Heaven::CLevel_Heaven(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CLevel(pDevice,pContext), m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CLevel_Heaven::Initialize()
{
	m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096.f, 4096.f, 4096.f));
	//m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096.f, 4096.f, 4096.f));

	m_pGameInstance->Setting_LUT(0, 0.25f, false);

	//TEST
	SHADOW_MAP_DESC ShadowMapDesc = {};
	ShadowMapDesc.iNumSectorX = 4;
	ShadowMapDesc.iNumSectorZ = 12;
	ShadowMapDesc.iSectorSizeX = 2048;
	ShadowMapDesc.iSectorSizeZ = 2048;

	ShadowMapDesc.vCenterPos = _float3(-910.f, 0.f, -1870.f);
	ShadowMapDesc.vExtents = _float3(200.f, 750.f, 160.f);
	ShadowMapDesc.vLightDir = _float3(0.f, -1.f, 0.5f);

	// Left Bottom : -910 / -1870
	// Right Bottom : 600 / -2100
	// Left Top : 1542

	if (FAILED(m_pGameInstance->Setting_ShadowMap(ShadowMapDesc)))
		CRASH("Test");
	

	m_pGameSystem->Clone_MapObjects(m_eCurLevel);

	m_pGameInstance->Render_ShadowMap();

	m_pGameInstance->Begin_DownSampleShadowMap();

	LIGHT_DESC LightDesc{};
	LightDesc.eType = LIGHT_DESC::DIRECTION;

	LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);
//	LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
//	LightDesc.vDiffuse = _float4(0.6f, 0.6f, 0.8f, 1.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 0.8f, 1.f);
//LightDesc.vDiffuse = _float4(0.8f, 0.8f, 0.65f, 1.f);
	LightDesc.vDirection = _float4(0.f, -1.f, 0.5f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
	m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
	m_pGameInstance->SetUp_CameraNF();

	m_pGameInstance->SettingFog(true);
	
	Ready_UI();
	Ready_Layer_Player();
	Ready_Layer_SequnecePlayer();

	Ready_Leviatan();

	//m_pGameSystem->Clone_Spawners(m_eCurLevel);
	// Test
	_uint iLevel = m_pGameInstance->Get_CurrentLevel();

	Ready_Effect();
	Ready_Skybox();
	//Ready_SFX();

	m_pGameInstance->Set_FogDistanceFallOff(0.001f);
	m_pGameInstance->Set_FogMaxHeight(300.f);
	m_pGameInstance->Set_FogRayDensityScale(0.f);

	m_pGameInstance->Begin_VF();

	m_pGameSystem->Create_MapEffects();
	return S_OK;
}

void CLevel_Heaven::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("Heaven"));


	m_pGameSystem->Update(fTimeDelta);
	

#ifdef _DEBUG
	DEBUG_FUNCTION();
#endif
}

void CLevel_Heaven::Render()
{

}

void CLevel_Heaven::Ready_Layer_Player()
{
	_float3 vScale{}, vRotation{}, vPosition{};
	vScale = { 1.f, 1.f, 1.f };
	vRotation = { 0.f, 0.f, 0.f };
	//vPosition = { 0.f, -10.f, 50.f };
	//vPosition = { 3455.f, 160.f, 2951.f }; => 신왕 광장 정중앙 좌표
	vPosition = { 0.f, 2.f, -40.f };
	
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

void CLevel_Heaven::Ready_Layer_SequnecePlayer()
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

void CLevel_Heaven::Ready_Dummy()
{
}

void CLevel_Heaven::Ready_MonsterTest()
{
	MONSTER_INFO* const pInfo = m_pGameSystem->Get_MonsterInfo("FalseSovereign");
	CMonsterTest::MONSTERTEST_DESC MobDesc{};
	MobDesc.eCurLevel = m_eCurLevel;
	MobDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	MobDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	MobDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_FalseSovereign"));
	MobDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	MobDesc.fRotationPerSec = XMConvertToRadians(90.f);
	MobDesc.fSpeedPerSec = 10.f;
	MobDesc.vInitPosition = _float3(3497.f, 147.84f, 3267.5f);
	MobDesc.vInitRotate = _float3(0.f, 180.f, 0.f);
	MobDesc.pAnimationTag = "Born1";
	MobDesc.strFolderPath = "../Bin/Resource/Model/Monster/FalseSovereign/Notify";
	MobDesc.fHP = pInfo->fMaxHp;
	MobDesc.fAttackDmg = pInfo->fAttack;
	MobDesc.fMaxStamina = pInfo->fMaxStamina;
	MobDesc.vDetectRange = _float3(55.f, 15.f, 55.f);
	if(FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MonsterTest"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), &MobDesc)))
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
	Projectile.eType = TEXT_COLOR_TYPE::ELEC;
	//Projectile.wstrEffectTag = ;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Projectile"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_EnemyAD"), TEXT("Pool_Projectile_ShinWang"), 9, &Projectile)))
		CRASH("Failed Ready Projectile (False Sovereign)");
}

void CLevel_Heaven::Ready_HavocWarrior()
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
	tDesc.vInitPosition = _float3(0.f, 40.f, 0.f);
	tDesc.pAnimationTag = "Stand1";
	tDesc.fHp = pInfo->fMaxHp;
	tDesc.fAttackDmg = pInfo->fAttack;
	tDesc.fImpluseRate = pInfo->fImpluseRate;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_HavocWarrior"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), pInfo->wstrPoolTag, 8, &tDesc)))
		CRASH("Failed Ready Monster (Havoc Warrior)");
}

void CLevel_Heaven::Ready_ElectroPredator()
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
	ADesc.vInitPosition = _float3(3.f, 40.f, 3.f);
	ADesc.pAnimationTag = "Stand2";
	ADesc.fHp = pInfo->fMaxHp;
	ADesc.fAttackDmg = pInfo->fAttack;
	ADesc.fImpluseRate = pInfo->fImpluseRate;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_ElectroPredator"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), pInfo->wstrPoolTag, 8, &ADesc)))
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
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_EnemyAD"), TEXT("Pool_Projectile_Electro"), 50, &Projectile)))
		CRASH("Failed Ready Projectile (Electro Predatror)");

	CAoEDoT::AOEDOT_DESC AoEDesc{};
	AoEDesc.fAttackDamage = ADesc.fAttackDmg * 0.25f;
	AoEDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	AoEDesc.iTargetLayers = { ENUM_CLASS(COLLISIONLAYER::PLAYER) };
	AoEDesc.vExtent = _float3(1.f, 1.f, 1.f);
	AoEDesc.vOffset = _float3(0.f, 1.f, 0.f);
	AoEDesc.eType = TEXT_COLOR_TYPE::ELEC;
	AoEDesc.wstrEffectTag = TEXT("Electro_GroundAttack");
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_AOEDOT"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_EnemyAD"), TEXT("Pool_AOEDOT_Electro"), 15, &AoEDesc)))
		CRASH("Failed Ready AoEDot (Electro Predatror)");
}

void CLevel_Heaven::Ready_CoroSaurus()
{
	MONSTER_INFO* const pInfo = m_pGameSystem->Get_MonsterInfo("CoroSaurus");
	// Corosaurus
	CCorosaurus::CORROSAURUS_DESC CoroDesc{};
	CoroDesc.eCurLevel = m_eCurLevel;
	CoroDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	CoroDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	CoroDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_CoroSaurus"));
	CoroDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	CoroDesc.fRotationPerSec = XMConvertToRadians(90.f);
	CoroDesc.fSpeedPerSec = 10.f;
	CoroDesc.vInitPosition = _float3(3479.2f, 268.6f, 2098.8f);
	CoroDesc.vInitRotate = _float3(0.f, 180.f, 0.f);
	CoroDesc.pAnimationTag = "Idle1";
	CoroDesc.strFolderPath = "../Bin/Resource/Model/Monster/Corrosaurus/Notify";
	CoroDesc.fHP = pInfo->fMaxHp;
	CoroDesc.fAttackDmg = pInfo->fAttack;
	CoroDesc.fMaxStamina = pInfo->fMaxStamina;
	CoroDesc.vDetectRange = _float3(55.f, 15.f, 55.f);
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_CoroSaurus"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), &CoroDesc)))
		CRASH("Failed Ready Monster");
}

void CLevel_Heaven::Ready_Leviatan()
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
	MobDesc.vInitPosition = _float3(0.f, 0.f, 0.f);
	MobDesc.vInitRotate = _float3(0.f, 180.f, 0.f);
	MobDesc.pAnimationTag = "Stand2";
	MobDesc.strFolderPath = "../Bin/Resource/Model/Monster/Leviatan/Notify";
	MobDesc.fHP = pInfo->fMaxHp;
	MobDesc.fAttackDmg = pInfo->fAttack;
	MobDesc.fMaxStamina = pInfo->fMaxStamina;
	MobDesc.vDetectRange = _float3(50.f, 25.f, 50.f);
	if(FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Leviatan"),
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

	if(FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Alter"),
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

	if(FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Ray"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), TEXT("Pool_LeviRay_S"), 16, &RayDesc)))
		CRASH("Failed Ready Ray");

	RayDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_SKILL);
	RayDesc.fAttackDamage = pInfo->fAttack * 1.5f;
	RayDesc.vExtent = _float3(2.f, 2.f, 100.f);
	if(FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Ray"),
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
	Projectile.fMaxDelay = 10.f;
	Projectile.wstrEffectTag = TEXT("Leviatan_Dg2");
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Projectile"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Projectile"), TEXT("Pool_Projectile_LeviSword"), 1, &Projectile)))
		CRASH("Failed Ready Projectile (Leviatan)");

	Projectile.wstrModelTag = TEXT("Prototype_Component_Model_Leviatan_SwordAura");
	Projectile.wstrEffectTag = TEXT("Leviatan_Dg");
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Projectile"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Projectile"), TEXT("Pool_Projectile_LeviAura"), 1, &Projectile)))
		CRASH("Failed Ready Projectile (Leviatan)");

	CLevi_Anchor::ANCHORDESC Anchor{};
	Anchor.fSpeedPerSec = 10.f;
	Anchor.fAttackDamage = pInfo->fAttack;
	//Anchor.wstrEffectTag = ;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Anchor"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Projectile"), TEXT("Pool_LeviAnchor"), 1, &Anchor)))
		CRASH("Failed Ready Projectile (Leviatan)");

	CLevi_Drop::DROPDESC Drop{};
	Drop.fAttackDamage = pInfo->fAttack * 0.5f;
	Drop.fSpeedPerSec = 10.f;
	//Drop.wstrEffectTag = ;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Drop"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Projectile"), TEXT("Pool_LeviDrop"), 16, &Drop)))
		CRASH("Failed Ready Projectile (Leviatan)");

	CLevi_Wave::WAVEDESC Wave{};
	Wave.fAttackDamage = pInfo->fAttack;
	Wave.fSpeedPerSec = 15.f;
	//Wave.wstrEffectTag = ;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Levi_Wave"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Projectile"), TEXT("Pool_LeviWave"), 4, &Wave)))
		CRASH("Failed Ready Projectile (Leviatan)");
}

void CLevel_Heaven::Ready_Effect()
{
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/Common", m_eCurLevel, 20);
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/Common_Plus", m_eCurLevel, 200);
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/Leviatan", m_eCurLevel, 15);
}

void CLevel_Heaven::Ready_Skybox()
{
	CSkyBox::SKYBOX_DESC SkyboxDesc = {};
	SkyboxDesc.iNumModel = 3;
	SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_Dome"));
	SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_Background"));
	SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_FX"));
	SkyboxDesc.vUVRate = _float2(1.f, 1.f);
	SkyboxDesc.fFXScaleRate = 0.3f;

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Skybox"), ENUM_CLASS(m_eCurLevel),
		TEXT("Layer_BackGround"), &SkyboxDesc)))
		CRASH("Skybox");
}

void CLevel_Heaven::Ready_UI()
{
	// UI
	const   _uint   iDestLevel = m_pGameInstance->Get_CurrentLevel();;
	const _wstring	strLayertag_UI = L"Layer_Custom_UI";
	const _wstring	strPrototypeTag_UI[] = {
		 L"Prototype_GameObject_Custom_UI_Container_HUD",
		 L"Prototype_GameObject_Custom_UI_Container_HUD_Sector_Minimap",
		 L"Prototype_GameObject_Custom_UI_Container_HUD_Sector_FuncIcons",
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

	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_GrapplePoint"),
		iDestLevel, TEXT("Layer_Custom_UI_GrapplePoint"), TEXT("Pool_Custom_GrapplePoint"), 50)))
		CRASH("Failed Ready GrapplePoint");

	CUI_QTE::UI_QTE_DESC tQTEDesc = {};
	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_QTE"),
		iDestLevel, TEXT("Layer_Custom_UI_QTE"), TEXT("Pool_Image_QTE"), 1, &tQTEDesc)))
		CRASH("Failed Ready QTE");


	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_UI_CurveTrace"),
		iDestLevel, TEXT("Layer_Custom_UI_CurveTrace"), TEXT("Pool_Custom_CurveTrace"), 1)))
		CRASH("Failed Ready CurveTrace");

	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_Ovfl_Palette"),
		iDestLevel, TEXT("Layer_Custom_UI_Ovfl_Palette"), TEXT("Pool_Custom_Ovfl_Palette"), 1)))
		CRASH("Failed Ready Ovfl_Palette");


	m_pGameSystem->PreAssign_TargetUIs();
	// _UI
}

void CLevel_Heaven::Ready_SFX()
{
	m_pGameSystem->Ready_SFX_Prefab("../Bin/Resource/Effect/SFX_Data/", ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_SFX_Prefab"), ENUM_CLASS(LEVEL::HEAVEN));

#pragma region SFX
	//if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_SFX_SonoraChange"),
	//	ENUM_CLASS(LEVEL::HEAVEN), TEXT("Layer_SFX"), TEXT("Pooling_SFX_SonoraChange"), 1)))
	//	CRASH("Failed Add Pool SONORA_CHANGE");

	//if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_SFX_Galbrena_UltiSlash"),
	//	ENUM_CLASS(LEVEL::HEAVEN), TEXT("Layer_SFX"), TEXT("Pooling_SFX_Galbrena_UltiSlash"), 1)))
	//	CRASH("Failed Add Pool Galbrena_UltiSlash");

	//if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(LEVEL::HEAVEN), TEXT("Prototype_SFX_Galbrena_UltiStar"),
	//	ENUM_CLASS(LEVEL::HEAVEN), TEXT("Layer_SFX"), TEXT("Pooling_SFX_Galbrena_UltiStar"), 1)))
	//	CRASH("Failed Add Pool Galbrena_UltiSlash");

#pragma endregion
}

#ifdef _DEBUG
void CLevel_Heaven::DEBUG_FUNCTION()
{
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD0) == KEYSTATE::DOWN)
		m_pGameInstance->End_SFX();
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD1) == KEYSTATE::DOWN)
		m_pGameInstance->Begin_Toggle_SFX(SFX_TOGGLE::BLUR, 2.f);
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD2) == KEYSTATE::DOWN)
		m_pGameInstance->Begin_Toggle_SFX(SFX_TOGGLE::DOF, 5.f);
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD3) == KEYSTATE::DOWN)
		m_pGameInstance->Begin_Toggle_SFX(SFX_TOGGLE::MOTION);
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD4) == KEYSTATE::DOWN)
		m_pGameInstance->Begin_Toggle_SFX(SFX_TOGGLE::RADIAL);

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD9) == KEYSTATE::DOWN)
	{
		//CSonoraChange::SONORA_CHANGE_DESC Desc = {};
		//Desc.fEffectTime = 3.f;
		//Desc.fRadialTime = 1.f;
		//Desc.fFadeTime = 1.f;

		m_pGameInstance->Spawn_PoolingObject(TEXT("Pooling_Galbrena_Ulti_Prefab"), XMMatrixIdentity(), nullptr);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD8) == KEYSTATE::DOWN)
	{
		m_pGameInstance->Spawn_PoolingObject(TEXT("Pooling_Augusta_Ulti_Prefab"), XMMatrixIdentity(), nullptr);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD5) == KEYSTATE::DOWN)
	{
		m_IsSSS = !m_IsSSS;
		m_pGameInstance->SettingSSS(m_IsSSS);
	}


	ImGui::Begin("SHADER");

	if (ImGui::CollapsingHeader("HDR"))
	{

		ImGui::InputFloat("EXPOSURE", &m_fExposure, 0.01f, 0.1f);

		m_pGameInstance->SettingHDR(m_fExposure);

	}
	if (ImGui::CollapsingHeader("LUT"))
	{
		if (ImGui::BeginCombo("LUT_INDEX", "LUT"))
		{
			for (_uint i = 0; i < 7; ++i)
			{

#ifdef _DEBUG
				if (ImGui::Selectable(to_string(i).c_str()))
				{
					m_iLUT_Index = i;
				}
#endif // _DEBUG
			}

			ImGui::EndCombo();
		}

		ImGui::Checkbox("IsDynamic", &m_IsDyanmicLUT);
		ImGui::DragFloat("LUT_INTENSITY", &m_fLUT_Intensity, 0.01f, 0.f, 1.f);
		m_pGameInstance->Setting_LUT(m_iLUT_Index, m_fLUT_Intensity, m_IsDyanmicLUT);
	}

	ImGui::End();
}
#endif

CLevel_Heaven* CLevel_Heaven::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Heaven* pInstance = new CLevel_Heaven(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Heaven");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Heaven::Free()
{
    __super::Free();
	Safe_Release(m_pGameSystem);
}
