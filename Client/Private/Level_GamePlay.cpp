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

#include "Player.h"
#include "SkyBox.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CLevel(pDevice,pContext), m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CLevel_GamePlay::Initialize()
{
	m_pGameInstance->SetUp_OctoTree(_float3(3164.29f, 159.2f, 2618.3f), _float3(4096.f, 4096.f, 4096.f));
	//m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096.f, 4096.f, 4096.f));

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
	LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
	LightDesc.vDiffuse = _float4(0.8f, 0.8f, 0.65f, 1.f);
	LightDesc.vDirection = _float4(0.f, -1.f, 0.5f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
	m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
	m_pGameInstance->SetUp_CameraNF();

	m_pGameInstance->SettingFog(true);
	m_pGameInstance->Set_LUT_Index(0);


	Ready_UI();
	Ready_Layer_Player();
	Ready_MonsterTest();
	Ready_HavocWarrior();
	Ready_ElectroPredator();
	Ready_CoroSaurus();

	m_pGameSystem->Clone_Spawners(m_eCurLevel);
	// Test
	_uint iLevel = m_pGameInstance->Get_CurrentLevel();

	Ready_Effect();
	Ready_Skybox();

	return S_OK;
}

void CLevel_GamePlay::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("GamePlay"));

	//소노라 올라가는 거 테스트. 추후 시스템의 업데이트 방식과 UI연동 후 삭제함.
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
			m_pGameSystem->Change_Sonoro(m_SonoroTest = !m_SonoroTest);

		m_pGameSystem->Update(fTimeDelta);
	}
}

void CLevel_GamePlay::Render()
{
#ifdef _DEBUG
	Shader_Gui();
#endif
}

void CLevel_GamePlay::Ready_Layer_Player()
{
	_float3 vScale{}, vRotation{}, vPosition{};
	vScale = { 1.f, 1.f, 1.f };
	vRotation = { 0.f, 0.f, 0.f };
	//vPosition = { 0.f, -10.f, 50.f };
	//vPosition = { 3455.f, 160.f, 2951.f }; => 신왕 광장 정중앙 좌표
	vPosition = { 2787.4f, 320.f, 1647.f };
	

	CPlayer::PLAYER_DESC Desc{};
	Desc.eCurLevel = m_eCurLevel;
	Desc.vScale = vScale;
	Desc.vRotation = vRotation;
	Desc.vPosition = vPosition;
	Desc.iPlayerCount = CPlayer::CHARACTERTYPE::TYPE_END;
	Desc.wStrInputControllerTag = TEXT("Prototype_Component_PlayerController");

	// 0. vector 크기 정의
	Desc.PlayerSpecs.resize(CPlayer::CHARACTERTYPE::TYPE_END);

	// 1. Augusta 정의.
	Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::AUGUSTA].CharacterDesc = PlayerData::GetAugustaCloneData(vScale, vRotation, vPosition, m_eCurLevel);
	Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::AUGUSTA].strActorTag = TEXT("Prototype_GameObject_Actor_Augusta");
	//Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::AUGUSTA].strActorTag = PlayerData::AUGUSTA_ACTOR_TAG;


	// 2. Galbrena 정의

	// 3. Rover(주인공) 캐릭터 정의
	Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::ROVER].CharacterDesc = PlayerData::GetRoverCloneData(vScale, vRotation, vPosition, m_eCurLevel);
	Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::ROVER].strActorTag = TEXT("Prototype_GameObject_Actor_Rover");

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
	MobDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
	MobDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_FalseSovereign"));
	MobDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	MobDesc.fRotationPerSec = XMConvertToRadians(90.f);
	MobDesc.fSpeedPerSec = 10.f;
	MobDesc.vInitPosition = _float3(3497.f, 147.84f, 3267.5f);
	MobDesc.vInitRotate = _float3(0.f, 180.f, 0.f);
	MobDesc.pAnimationTag = "Born1";
	MobDesc.strFolderPath = "../Bin/Resource/Model/FalseSovereign/Notify";
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
	Ggobul.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
	Ggobul.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_Ggobul"));
	Ggobul.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	Ggobul.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
	Ggobul.strFolderPath = "../Bin/Resource/Model/Ggobul/Notify";
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
	Tantacle.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
	Tantacle.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_Scythe"));
	Tantacle.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	Tantacle.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
	Tantacle.strFolderPath = "../Bin/Resource/Model/FS_Scythe/Notify";
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
	tDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
	tDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_HavocWarrior"));
	tDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	tDesc.strFolderPath = "../Bin/Resource/Model/HavocWarrior/Notify";
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
	ADesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
	ADesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_ElectroPredator"));
	ADesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	ADesc.strFolderPath = "../Bin/Resource/Model/ElectroPredator/Notify";
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
	//Projectile.wstrEffectTag = ;
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Projectile"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_EnemyAD"), TEXT("Pool_Projectile_Electro"), 10, &Projectile)))
		CRASH("Failed Ready Projectile (Electro Predatror)");

	CAoEDoT::AOEDOT_DESC AoEDesc{};
	AoEDesc.fAttackDamage = ADesc.fAttackDmg * 0.25f;
	AoEDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	AoEDesc.iTargetLayers = { ENUM_CLASS(COLLISIONLAYER::PLAYER) };
	AoEDesc.vExtent = _float3(1.f, 1.f, 1.f);
	AoEDesc.vOffset = _float3(0.f, 1.f, 0.f);
	//AoEDesc.wstrEffectTag
	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_AOEDOT"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_EnemyAD"), TEXT("Pool_AOEDOT_Electro"), 5, &AoEDesc)))
		CRASH("Failed Ready AoEDot (Electro Predatror)");
}

void CLevel_GamePlay::Ready_CoroSaurus()
{
	MONSTER_INFO* const pInfo = m_pGameSystem->Get_MonsterInfo("CoroSaurus");
	// Corosaurus
	CCorosaurus::CORROSAURUS_DESC CoroDesc{};
	CoroDesc.eCurLevel = m_eCurLevel;
	CoroDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	CoroDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
	CoroDesc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_CoroSaurus"));
	CoroDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	CoroDesc.fRotationPerSec = XMConvertToRadians(90.f);
	CoroDesc.fSpeedPerSec = 10.f;
	CoroDesc.vInitPosition = _float3(3479.2f, 268.6f, 2098.8f);
	CoroDesc.vInitRotate = _float3(0.f, 180.f, 0.f);
	CoroDesc.pAnimationTag = "Idle1";
	CoroDesc.strFolderPath = "../Bin/Resource/Model/Corrosaurus/Notify";
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
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/Common", m_eCurLevel);
	m_pGameSystem->Create_Prefab("../../Client/Bin/Resource/Effect/Prefabs/WeiZuoShenWang", m_eCurLevel);
}

void CLevel_GamePlay::Ready_Skybox()
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
		if (FAILED(m_pGameInstance->Add_RootUI(L"UI_UHD", pTargetUI)))
			CRASH("Failed to Add RootUI to UI_Manager.");
		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, strLayertag_UI, pTargetUI)))
			CRASH("Failed to Add RootUI to Object_Manager.");
	}

	// _UI
}

#ifdef _DEBUG
void CLevel_GamePlay::Shader_Gui()
{
//	ImGui::Begin("Test");
//	
//	
//	if (ImGui::CollapsingHeader("SSAO"))
//	{
//				ImGui::InputFloat("RADIUS", &m_fRadius);
//		
//				ImGui::DragFloat("MAX_DISTANCE", &m_fMaxDistance, 1.f, 1.f, 50.f, "%.1f");
//		
//		#ifdef _DEBUG
//				m_pGameInstance->Setting_SSAO(m_fRadius, m_fMaxDistance);
//		#endif // _DEBUG
//	}
//
////	if (ImGui::CollapsingHeader("LUT"))
////	{
////		ImGui::InputInt("INDEX", &m_iLUT_Index, 1, 1);
////		if (m_iLUT_Index < 0)
////			m_iLUT_Index = 0;
////		if (m_iLUT_Index >= 5)
////			m_iLUT_Index = 4;
////
////		ImGui::DragFloat("LUT_INTENSITY", &m_fLUT_Intensity, 0.01f, 0.f, 1.f);
////#ifdef _DEBUG
////		m_pGameInstance->Set_LUT_Index(m_iLUT_Index);
////		m_pGameInstance->Bind_RawValue_Renderer("g_fLutLerpIntensity", &m_fLUT_Intensity, sizeof(_float));
////#endif // _DEBUG
////	}
//
//		if (ImGui::CollapsingHeader("PBR"))
//	{
//
//		ImGui::DragFloat("DYNAMIC_ROUGHNESS", &m_fDebugRoughness[0], 0.01f, 0.f, 1.f);
//		ImGui::DragFloat("STATIC_ROUGHNESS", &m_fDebugRoughness[1], 0.01f, 0.f, 1.f);
//
//		ImGui::DragFloat("DYNAMIC_METALLIC", &m_fDebugMetallic[0], 0.01f, 0.f, 1.f);
//		ImGui::DragFloat("STATIC_METALLIC", &m_fDebugMetallic[1], 0.01f, 0.f, 1.f);
//#ifdef _DEBUG
//		m_pGameInstance->Set_Metallic(m_fDebugMetallic[0], m_fDebugMetallic[1]);
//		m_pGameInstance->Set_Roughness(m_fDebugRoughness[0], m_fDebugRoughness[1]);
//#endif // _DEBUG		 
//	}
//
//	ImGui::End();
	/*
	if (ImGui::CollapsingHeader("CASCADE"))
	{
		if (ImGui::CollapsingHeader("Base Bias"))
		{
			ImGui::InputFloat("CASCADE[0]", &m_fBias[0], 0.001f, 0.001f);
			ImGui::InputFloat("CASCADE[1]", &m_fBias[1], 0.001f, 0.001f);
			ImGui::InputFloat("CASCADE[2]", &m_fBias[2], 0.001f, 0.001f);
			ImGui::InputFloat("CASCADE[3]", &m_fBias[3], 0.001f, 0.001f);
		}

		if (ImGui::CollapsingHeader("Min Bias"))
		{

			ImGui::InputFloat("MIN_BIAS_CASCADE[0]", &m_fMinBias[0], 0.001f, 0.001f);
			ImGui::InputFloat("MIN_BIAS_CASCADE[1]", &m_fMinBias[1], 0.001f, 0.001f);
			ImGui::InputFloat("MIN_BIAS_CASCADE[2]", &m_fMinBias[2], 0.001f, 0.001f);
			ImGui::InputFloat("MIN_BIAS_CASCADE[3]", &m_fMinBias[3], 0.001f, 0.001f);
		}

		if (ImGui::CollapsingHeader("Map Bias"))
		{
			ImGui::InputFloat("MAP", &m_fMapBias, 0.001f, 0.001f);
		}

		if (ImGui::CollapsingHeader("SLOPE_SCALE"))
		{
			ImGui::InputFloat("SCALE", &m_fSlopeScale);
		}

		m_pGameInstance->Bind_RawValue_Renderer("g_fShadowBais", &m_fBias, sizeof(_float4));
		m_pGameInstance->Bind_RawValue_Renderer("g_fMinShadowBias", &m_fMinBias, sizeof(_float4));
		m_pGameInstance->Bind_RawValue_Renderer("g_DebugSlopeScale", &m_fSlopeScale, sizeof(_float));
		m_pGameInstance->Bind_RawValue_Renderer("g_fShadowMapBais", &m_fMapBias, sizeof(_float));
	}
	*/
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
