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

#include "Player.h"
#include "SkyBox.h"
#include "UI_Text_Damage.h"
#include "UI_Parry.h"

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
	m_pGameInstance->Setting_LUT(1, 0.77f, false);

	//TEST
	SHADOW_MAP_DESC ShadowMapDesc = {};
	ShadowMapDesc.iNumSectorX = 8;
	ShadowMapDesc.iNumSectorZ = 8;
	ShadowMapDesc.iSectorSizeX = 2048;
	ShadowMapDesc.iSectorSizeZ = 2048;

	ShadowMapDesc.vCenterPos = _float3(2200.f, 60.f, 1200.f);
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
//	LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
//	LightDesc.vDiffuse = _float4(0.6f, 0.6f, 0.8f, 1.f);
	LightDesc.vDiffuse = _float4(0.5f, 0.55f, 0.85f, 1.f);
//LightDesc.vDiffuse = _float4(0.8f, 0.8f, 0.65f, 1.f);
	LightDesc.vDirection = _float4(0.f, -1.f, 0.5f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
	m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
	m_pGameInstance->SetUp_CameraNF();

	m_pGameInstance->SettingFog(true);
	

	Ready_UI();
	Ready_Layer_Player();
	Ready_MonsterTest();
	Ready_HavocWarrior();
	Ready_ElectroPredator();
	Ready_CoroSaurus();
	Ready_NPC();

	m_pGameSystem->Clone_Spawners(m_eCurLevel);
	// Test
	_uint iLevel = m_pGameInstance->Get_CurrentLevel();

	Ready_Effect();
	Ready_Skybox();
	Ready_Mouse();
	Ready_SFX();

//	m_pGameInstance->Bake_EnvMaps();

	return S_OK;
}

void CLevel_GamePlay::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("GamePlay"));

	//소노라 올라가는 거 테스트. 추후 시스템의 업데이트 방식과 UI연동 후 삭제함.
	{
		//if (m_pGameInstance->Get_DIKeyState(DIK_F) == KEYSTATE::DOWN)
		//{
		//	m_pGameSystem->Change_Sonoro(m_SonoroTest = !m_SonoroTest);
		//	m_pGameSystem->Play_Action(TEXT("Action_False_Sonora"), XMMatrixRotationY(1.6736f+3.14f) * XMMatrixTranslation(3546.f, 173.f, 2931.f), false);
		//}

		m_pGameSystem->Update(fTimeDelta);
	}

#ifdef _DEBUG
	DEBUG_FUNCTION();
#endif
	// 임시 Mouse 고정
	

	// UI Test. Delete it.
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
	auto pTargetMobHPBarUI = m_pGameSystem->Find_RootUI(L"UI_MobHPBar");
	_bool isTargetAlive = (pTargetMobHPBarUI) ? pTargetMobHPBarUI->IsActivate() : false;

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD4) == KEYSTATE::DOWN &&
		!isTargetAlive)
		m_pGameInstance->Spawn_PoolingObject(L"Pool_Image_MobHPBar", _fmatrix(), nullptr);
	else if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD4) == KEYSTATE::DOWN &&
		isTargetAlive)
		pTargetMobHPBarUI->SetActivate(false);
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

	if (!isOpenOverflowingPalette &&
		m_pGameInstance->Get_DIKeyState(DIK_NUMPAD5) == KEYSTATE::DOWN)
	{
		m_pGameSystem->Open_Game_OverflowPalette();
		isOpenOverflowingPalette = true;
	}
	else if (isOpenOverflowingPalette &&
		m_pGameInstance->Get_DIKeyState(DIK_NUMPAD5) == KEYSTATE::DOWN)
	{
		m_pGameSystem->Close_Game_OverflowPalette();
		isOpenOverflowingPalette = false;
	}

#pragma endregion



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
	vPosition = { 2375.42f, 317.92f, 1645.60f };
	

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

void CLevel_GamePlay::Ready_Effect()
{
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/Common", m_eCurLevel, 20);
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/Common_Plus", m_eCurLevel, 200);
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/WeiZuoShenWang", m_eCurLevel, 15);
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
		 L"Prototype_GameObject_Custom_UI_Container_HUD"
	};
	for (auto& strPrototypeTag : strPrototypeTag_UI)
	{
		CUIObject* pTargetUI = static_cast<CUIObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, strPrototypeTag, PROTOTYPE::GAMEOBJECT));
	//	if (FAILED(m_pGameInstance->Add_RootUI(L"UI_HUD", pTargetUI)))
	//		CRASH("Failed to Add RootUI to UI_Manager.");
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

	if (FAILED(m_pGameInstance->Add_PoolingObject(iDestLevel, TEXT("Prototype_GameObject_Custom_UI_Ovfl_Palette"),
		iDestLevel, TEXT("Layer_Custom_UI_Ovfl_Palette"), TEXT("Pool_Custom_Ovfl_Palette"), 1)))
		CRASH("Failed Ready Ovfl_Palette");


	m_pGameSystem->PreAssign_TargetUIs();
	// _UI
}

void CLevel_GamePlay::Ready_Mouse()
{
	// Mouse
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Mouse"), ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Layer_Mouse"))))
		CRASH("Mosue");
}

void CLevel_GamePlay::Ready_SFX()
{
	m_pGameSystem->Ready_SFX_Prefab("../Bin/Resource/Effect/SFX_Data/", ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_SFX_Prefab"), ENUM_CLASS(LEVEL::GAMEPLAY));

#pragma region SFX
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_SFX_SonoraChange"),
		ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Layer_SFX"), TEXT("Pooling_SFX_SonoraChange"), 1)))
		CRASH("Failed Add Pool SONORA_CHANGE");

	//if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_SFX_Galbrena_UltiSlash"),
	//	ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Layer_SFX"), TEXT("Pooling_SFX_Galbrena_UltiSlash"), 1)))
	//	CRASH("Failed Add Pool Galbrena_UltiSlash");

	//if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_SFX_Galbrena_UltiStar"),
	//	ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Layer_SFX"), TEXT("Pooling_SFX_Galbrena_UltiStar"), 1)))
	//	CRASH("Failed Add Pool Galbrena_UltiSlash");

#pragma endregion
}

void CLevel_GamePlay::Ready_NPC()
{
	CNPCInstancing::NPC_DESC NPCDesc{};
	NPCDesc.eCurLevel = m_eCurLevel;
	NPCDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh_Instance"));
	NPCDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxInstance_AnimMesh"));
	NPCDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_FemaleM"));
	NPCDesc.wstrObjectPrototypeTag = TEXT("Prototype_GameObject_NPCCell");
	NPCDesc.wstrCombiningPrototypeTag = TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh_Skinning");
	m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_NPCInstancing"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Z_NPCInstance"), &NPCDesc);
}

#ifdef _DEBUG
void CLevel_GamePlay::DEBUG_FUNCTION()
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
	if (ImGui::CollapsingHeader("SSR"))
	{
		ImGui::InputFloat("MIN_STEP", &m_fMinStep, 1.f, 2.f);
		ImGui::InputFloat("MAX_STEP", &m_fMaxStep, 1.f, 2.f);
		ImGui::InputFloat("STARTOFFSET", &m_fStart, 1.f, 2.f);

		m_pGameInstance->Set_SSR(m_fMinStep, m_fMaxStep, m_fStart);
	}
	//if (ImGui::CollapsingHeader("HDR"))
	//{

	//	ImGui::InputFloat("EXPOSURE", &m_fExposure, 0.01f, 0.1f);
	//	
	//	m_pGameInstance->SettingHDR(m_fExposure);
	//
	//}
	if (ImGui::CollapsingHeader("LUT"))
	{
		if (ImGui::BeginCombo("LUT_INDEX", "LUT"))
		{
			for (_uint i = 0; i < 7; ++i)
			{

				if (ImGui::Selectable(to_string(i).c_str()))
				{
					m_iLUT_Index = i;
				}
			}

			ImGui::EndCombo();
		}
	
		ImGui::Checkbox("IsDynamic", &m_IsDyanmicLUT);
		ImGui::DragFloat("LUT_INTENSITY", &m_fLUT_Intensity, 0.01f, 0.f, 1.f);
		m_pGameInstance->Setting_LUT(m_iLUT_Index, m_fLUT_Intensity, m_IsDyanmicLUT);
	}

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
