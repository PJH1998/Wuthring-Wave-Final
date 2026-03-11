#include "ClientPch.h"
#include "Level_GamePlay.h"
#include"GameSystem.h"
#include "MonsterTest.h"
#include "Ggobul.h"
#include "FS_Scythe.h"
#include "HavocWarrior.h"
#include "ElectroPredator.h"
#include "Corosaurus.h"
#include "Projectile.h"
#include "AoEDoT.h"
#include "NPCInstancing.h"
#include "Napal.h"
#include "CoroProduction.h"
#include "NPC_Hiding.h"

#include "Player.h"
#include "SkyBox.h"
#include "UI_Text_Damage.h"
#include "UI_Parry.h"
#include "UI_QTE.h"

#include "Event_Level.h"
#include"NPC_Griffin.h"
#include"Potal.h"

#include "PatternDummy.h"

//SFX
#ifdef _DEBUG
#include "SonoraChange.h"
#endif

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CLevel(pDevice,pContext), m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CLevel_GamePlay::Initialize()
{
	m_pGameInstance->SetUp_OctoTree(_float3(3164.29f, 159.2f, 2618.3f), _float3(4096.f, 4096.f, 4096.f));
	//m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096.f, 4096.f, 4096.f));

//	m_pGameInstance->Add_Probe(_float3(2375.42f, 317.92f, 1645.60f), 2000.f);

	//m_pGameInstance->Setting_LUT(0, 0.25f, false);
	m_pGameInstance->Setting_LUT(1, 0.22f, false);

	//TEST
	SHADOW_MAP_DESC ShadowMapDesc = {};
	ShadowMapDesc.iNumSectorX = 8;
	ShadowMapDesc.iNumSectorZ = 8;
	ShadowMapDesc.iSectorSizeX = 2048;
	ShadowMapDesc.iSectorSizeZ = 2048;

	ShadowMapDesc.vStartPos = _float3(2200.f, 60.f, 1200.f);
	ShadowMapDesc.vExtents = _float3(160.f, 300.f, 160.f);
	ShadowMapDesc.vLightDir = _float3(0.f, -1.f, 0.5f);

	if (FAILED(m_pGameInstance->Setting_ShadowMap(ShadowMapDesc)))
		CRASH("Test");

	m_pGameSystem->Clone_MapObjects(m_eCurLevel);

	m_pGameInstance->Render_ShadowMap();

	m_pGameInstance->Begin_DownSampleShadowMap();

	LIGHT_DESC LightDesc{};
	LightDesc.eType = LIGHT_DESC::DIRECTION;
	LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);
	LightDesc.vDiffuse = _float4(0.5f, 0.55f, 0.85f, 1.f);
	LightDesc.vDirection = _float4(0.f, -1.f, 0.5f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
	m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
	m_pGameInstance->SetUp_CameraNF();
	m_pGameInstance->SettingFog(true);

	m_pGameSystem->Set_Sonora_LightDesc(SONORA::NONE, LightDesc);

	LIGHT_DESC SonoraLightDesc = {};
	SonoraLightDesc.eType = LIGHT_DESC::DIRECTION;
	SonoraLightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);
	SonoraLightDesc.vDiffuse = _float4(0.85f, 0.55f, 0.4f, 1.f);
	SonoraLightDesc.vDirection = _float4(0.f, -1.f, 0.5f, 0.f);
	SonoraLightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	m_pGameSystem->Set_Sonora_LightDesc(SONORA::SONORA, SonoraLightDesc);

	Ready_Potal();
	Ready_UI();
	Ready_Layer_Player();
	Ready_MonsterTest();
	Ready_HavocWarrior();
	Ready_ElectroPredator();
	Ready_CoroSaurus();
	Ready_NPC();
	Ready_Production();
	//허수아비 복제
	Ready_Dummy();
	m_pGameSystem->Clone_Spawners(m_eCurLevel);
	// Test
	_uint iLevel = m_pGameInstance->Get_CurrentLevel();

	Ready_Effect();
	Ready_Skybox();
	Ready_SFX();

	m_pGameInstance->Set_FogDistanceFallOff(0.1f);
	m_pGameInstance->Set_FogMaxHeight(230.f);
	m_pGameInstance->Set_FogMaxDistance(100.f);
	m_pGameInstance->Set_FogRayDensityScale(0.4f);
	m_pGameInstance->Set_FogScatterWeight(0.5f);
	m_pGameInstance->Set_FogFarRatioToCameraFar(0.3f);
	m_pGameInstance->Set_FogRayIntensity(3.f);

	m_pGameInstance->Begin_VF();

//	m_pGameInstance->Bake_EnvMaps();

	m_pGameSystem->Create_MapEffects(m_pGameInstance->Get_CurrentLevel());

	//TEST
	m_pGameSystem->Change_Level(m_pGameInstance->Get_CurrentLevel());
	return S_OK;
}

void CLevel_GamePlay::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("GamePlay"));

	if (m_pGameInstance->Get_DIKeyState(DIK_F3) == KEYSTATE::DOWN)
	{
		CHANGE_LEVEL_EVENT event{ LEVEL::HEAVEN, true };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_N) == KEYSTATE::DOWN)
		m_pGameSystem->Set_MouseFix(false);

	m_pGameSystem->Update(fTimeDelta);

#ifdef _DEBUG
	DEBUG_FUNCTION();
#endif
	// 임시 Mouse 고정
}

void CLevel_GamePlay::Render()
{

}

void CLevel_GamePlay::Ready_Layer_Player()
{
	_float3 vScale{}, vRotation{}, vPosition{};
	vScale = { 1.f, 1.f, 1.f };
	vRotation = { 0.f, 0.f, 0.f };
	//vPosition = { 0.f, -10.f, 50.f };
	//vPosition = { 3455.f, 160.f, 2951.f }; => 신왕 광장 정중앙 좌표
	//vPosition = { 2375.42f, 317.92f, 1645.60f }; => 구 좌표
	vPosition = { 2464.6f, 317.2f, 1832.9f };
	

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

void CLevel_GamePlay::Ready_Dummy()
{
	CPatternDummy::PAT_DUMMYDESC DummyDesc{};
	DummyDesc.eLevel = m_eCurLevel;
	DummyDesc.eType = CPatternDummy::MODEL_TYPES::DEFAULT;
	DummyDesc.strModelTag = TEXT("Prototype_Component_Model_HavocWarrior");					// 근거리 잡몹
	DummyDesc.strInitAnimTag = "Stand1";		
	DummyDesc.strFolderPath = "../Bin/Resource/Model/Monster/HavocWarrior/Notify";			
	DummyDesc.isCollide = true;
	//DummyDesc.eType = CPatternDummy::MODEL_TYPES::DEFAULT;
	//DummyDesc.strPartTag = TEXT("Prototype_Component_Model_Leviatan_Bayonet");
	//DummyDesc.strBoneName = "WeaponProp02";
	//DummyDesc.vOffsetPos = _float3(0.f, 0.f, 0.f);
	//DummyDesc.vOffsetRot = _float3(XMConvertToRadians(90.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));

	//DummyDesc.vInitPosition = _float3(2464.6f, 317.2f, 184.9f);
	DummyDesc.vInitPosition = _float3(2480.6f, 317.2f, 1826.5f);
	//{ 2464.6f, 317.2f, 1832.9f };
	DummyDesc.vInitRotation = _float3(XMConvertToRadians(0.f), XMConvertToRadians(-90.f), XMConvertToRadians(0.f));
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_PatternDummy"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Dummy"), &DummyDesc)))
		CRASH("Failed Ready Monster");
}

void CLevel_GamePlay::Ready_MonsterTest()
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
#ifdef _DEBUG
	MobDesc.fHP = 150.f;
#endif
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

void CLevel_GamePlay::Ready_HavocWarrior()
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

void CLevel_GamePlay::Ready_ElectroPredator()
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

void CLevel_GamePlay::Ready_CoroSaurus()
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
	CoroDesc.pAnimationTag = "burst01_5";
	CoroDesc.strFolderPath = "../Bin/Resource/Model/Monster/Corrosaurus/Notify";
	CoroDesc.fHP = pInfo->fMaxHp;
	CoroDesc.fAttackDmg = pInfo->fAttack;
	CoroDesc.fMaxStamina = pInfo->fMaxStamina;
	CoroDesc.vDetectRange = _float3(55.f, 15.f, 55.f);
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_CoroSaurus"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Enemy"), &CoroDesc)))
		CRASH("Failed Ready Monster");
}

void CLevel_GamePlay::Ready_Effect()
{
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/Common", m_eCurLevel, 20);
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/Common_Plus", m_eCurLevel, 70);
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/WeiZuoShenWang", m_eCurLevel, 15);
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/Corro", m_eCurLevel, 10);

	m_pGameSystem->Create_Spertrum("../../Client/Bin/Resource/Effect/Spectrums/GamePlay/SpectrumOB", m_eCurLevel, 15);

	if(FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Effect_Rope"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Effect"), TEXT("Rope"), 3, nullptr)))
	{
		MSG_BOX("Rope Load Fail");
		return;
	}
}

void CLevel_GamePlay::Ready_Skybox()
{
	CSkyBox::SKYBOX_DESC SkyboxDesc = {};
	SkyboxDesc.iNumModel = 4;
	SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_Dome"));
	SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_Background"));
	SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_FX"));
	SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_Cloud"));
	SkyboxDesc.vUVRate = _float2(9.f, 12.f);
	SkyboxDesc.fFXScaleRate = 0.2f;

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Skybox"), ENUM_CLASS(m_eCurLevel),
		TEXT("Layer_BackGround"), &SkyboxDesc)))
		CRASH("Skybox");
}

void CLevel_GamePlay::Ready_UI()
{
	// UI
	const   _uint       iDestLevel = m_pGameInstance->Get_CurrentLevel();
	const _wstring strLayertag_UI = L"Layer_Custom_UI";
	const _wstring strPrototypeTag_UI[] = {
		 L"Prototype_GameObject_Custom_UI_Container_HUD",
		 L"Prototype_GameObject_Custom_UI_Container_HUD_Sector_Minimap",
		 L"Prototype_GameObject_Custom_UI_Container_HUD_Sector_FuncIcons",
		 L"Prototype_GameObject_Custom_UI_Container_QuestIndicator",
	};
	for (auto& strPrototypeTag : strPrototypeTag_UI)
	{
		CUIObject* pTargetUI = static_cast<CUIObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, strPrototypeTag, PROTOTYPE::GAMEOBJECT));
	
		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, strLayertag_UI, pTargetUI)))
			CRASH("Failed to Add RootUI to Object_Manager.");
	}

	CUI_Text_Damage::TEXT_UI_TIMED_DESC tDesc = {};
	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_Text_Damage"),
		iDestLevel, TEXT("Layer_Custom_UI_Text_Damage"), TEXT("Pool_Text_Damage"), 100, &tDesc)))
		CRASH("Failed Ready Text_Damage");

	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_Button_Interact"),
		iDestLevel, TEXT("Layer_Custom_UI_Button_Interact"), TEXT("Pool_Button_Interact"), 1, &tDesc)))
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

void CLevel_GamePlay::Ready_SFX()
{

}

void CLevel_GamePlay::Ready_NPC()
{
	CNPCInstancing::NPC_DESC NPCDesc{};
	NPCDesc.iNPCType = 0;
	NPCDesc.eCurLevel = m_eCurLevel;
	NPCDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh_Instance"));
	NPCDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxInstance_AnimMesh"));
	NPCDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_FemaleM"));
	NPCDesc.wstrObjectPrototypeTag = TEXT("Prototype_GameObject_NPCCell");
	NPCDesc.wstrCombiningPrototypeTag = TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh_Skinning");
	m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_NPCInstancing"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Z_NPCInstance"), &NPCDesc);

	NPCDesc.iNPCType = 1;
	NPCDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_MaleM"));
	m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_NPCInstancing"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Z_NPCInstance"), &NPCDesc);

	CNapal::NAPALDESC Napal{};
	Napal.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_Napal"));
	Napal.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	Napal.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	Napal.eCurLevel = m_eCurLevel;
	Napal.vInitPos = _float3(3206.12f, 350.9f, 1680.1f);
	Napal.vInitRot = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	Napal.strFolderPath = "../Bin/Resource/Model/NPC/Napal/Notify";
	m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Napal"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_NPC"), &Napal);

	CNPC_Griffin::GRIFFIN_DESC Desc{};
	Desc.eCurLevel = m_eCurLevel;
	Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	Desc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_NPCGriffin"));
	Desc.pAnimMachineTag = TEXT("Prototype_Component_AnimMachine_NPCGriffin");
	Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	Desc.strFolderPath = "../Bin/Resource/Model/NPC/Animals/Griffin/Notify";
	
	_float3 vRotDegree = _float3(-180.0f, 88.149f, -180.f);
	_vector vRot = XMVectorSet(XMConvertToRadians(vRotDegree.x), XMConvertToRadians(vRotDegree.y), XMConvertToRadians(vRotDegree.z), 0.f);
	_vector vTrans = XMVectorSet(3214.813f, 324.34f, 1727.473f, 1.f);
	
	XMStoreFloat4x4(&Desc.pTransformMatrix, XMMatrixRotationRollPitchYawFromVector(vRot) * XMMatrixTranslationFromVector(vTrans));
	m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Griffin"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Z_Test"), &Desc);

	vRotDegree = _float3(-180.f, -4.127f, -180.f);
	vRot = XMVectorSet(XMConvertToRadians(vRotDegree.x), XMConvertToRadians(vRotDegree.y), XMConvertToRadians(vRotDegree.z), 0.f);
	vTrans = XMVectorSet(3273.907f, 339.604f, 1650.809f, 1.f);
	XMStoreFloat4x4(&Desc.pTransformMatrix, XMMatrixRotationRollPitchYawFromVector(vRot) * XMMatrixTranslationFromVector(vTrans));
	m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Griffin"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Z_Test"), &Desc);
	
	vRotDegree = _float3(-180.f, -17.102f, 180.f);
	vRot = XMVectorSet(XMConvertToRadians(vRotDegree.x), XMConvertToRadians(vRotDegree.y), XMConvertToRadians(vRotDegree.z), 0.f);
	vTrans = XMVectorSet(3405.010f, 379.317f, 1625.268f, 1.f);
	XMStoreFloat4x4(&Desc.pTransformMatrix, XMMatrixRotationRollPitchYawFromVector(vRot) * XMMatrixTranslationFromVector(vTrans));
	m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Griffin"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Z_Test"), &Desc);

	CNPC_Hiding::HIDINGDESC Hiding{};
	vector<NPCINFO> HidingData = m_pGameSystem->Get_NpcData(3);
	vector<_wstring> ModelTag;
	ModelTag.push_back(TEXT("Prototype_Component_Model_FemaleS370437"));
	ModelTag.push_back(TEXT("Prototype_Component_Model_FemaleS370708"));
	ModelTag.push_back(TEXT("Prototype_Component_Model_FemaleS380101"));
	ModelTag.push_back(TEXT("Prototype_Component_Model_FemaleS371438"));
	ModelTag.push_back(TEXT("Prototype_Component_Model_FemaleS381221"));

	Hiding.eCurLevel = m_eCurLevel;
	Hiding.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	Hiding.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"));
	Hiding.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	Hiding.pAnimMachineTag = TEXT("Prototype_Component_AnimMachine_NPC_Hiding");
	Hiding.fRotationPerSec = XMConvertToRadians(90.f);
	Hiding.strFolderPath = "../Bin/Resource/Model/NPC/FemaleS/Notify";
	Hiding.fSpeedPerSec = 1.f;

	for (size_t i = 0; i < HidingData.size(); ++i)
	{
		Hiding.modelData = make_pair(m_eCurLevel, ModelTag[i].c_str());
		Hiding.isCollide = HidingData[i].isCollide;
		Hiding.vInitPos = HidingData[i].vPosition;
		Hiding.vInitRot = HidingData[i].vRotation;
		Hiding.pAnimationTag = HidingData[i].strAnimTag.c_str();

		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_NPC_Hiding"),
			ENUM_CLASS(m_eCurLevel), TEXT("Layer_NPC"), &Hiding)))
			CRASH("Failed Ready NPC_Hiding");
	}
}

void CLevel_GamePlay::Ready_Production()
{
	CCoroProduction::COROPROD_DESC Production{};
	Production.shaderData = { LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh") };
	Production.computeShaderData = { LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib") };
	Production.modelData = { m_eCurLevel, TEXT("Prototype_Component_Model_CoroProduction") };
	Production.strFolderPath = "../Bin/Resource/Model/NPC/CoroProduction/Notify";
	Production.fSpeedPerSec = 10.f;
	Production.fRotationPerSec = XMConvertToRadians(90.f);
	m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_CoroProduction"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_NPC"), &Production);
}

void CLevel_GamePlay::Ready_Potal()
{
	CPotal::POTAL_DESC PotalDesc{};
	PotalDesc.iLevel = ENUM_CLASS(m_eCurLevel);
	PotalDesc.vExtent = _float3(15.1f, 15.1f, 0.5f);
	PotalDesc.vPos = _float4(3490.f, 164.8f, 3313.5f, 1.f);

	m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Potal"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Potal"), &PotalDesc);
}

#ifdef _DEBUG
void CLevel_GamePlay::DEBUG_FUNCTION()
{

	ImGui::Begin("SHADER");
	ImGui::InputFloat("BIAS", &m_fMapBias);

	m_pGameInstance->Bind_RawValue_Renderer("g_fShadowMapBais", &m_fMapBias, sizeof(_float));

	ImGui::End();
	//if (ImGui::CollapsingHeader("MOTION_BLUR"))
	//{
	//	ImGui::InputFloat("LIMIT_VELOCITY", &m_fLimitVelocity);

	//	ImGui::InputFloat("LIMIT_DEPTH", &m_fLimitDepth);

	//	ImGui::InputFloat("DISTANCE_SCALE", &m_fLengthScale);

	//	m_pGameInstance->Set_Motion(m_fLimitVelocity, m_fLimitDepth, m_fLengthScale);
	//}
}
#endif

CLevel_GamePlay* CLevel_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_GamePlay* pInstance = new CLevel_GamePlay(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_GamePlay");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_GamePlay::Free()
{
    __super::Free();
	Safe_Release(m_pGameSystem);
}
