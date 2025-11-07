#include "ClientPch.h"
#include "Level_GamePlay.h"
#include"GameSystem.h"
#include "MonsterTest.h"
#include "Ggobul.h"
#include "FS_Scythe.h"

#include "Player.h"
#include "SkyBox.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CLevel(pDevice,pContext), m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CLevel_GamePlay::Initialize()
{
	m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096, 4096, 4096));

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
	
	m_pGameSystem->Clone_MapObjects(m_eCurLevel, 0);
	//m_pGameSystem->Clone_MapObjects(m_eCurLevel, 1);

	LIGHT_DESC LightDesc{};
	LightDesc.eType = LIGHT_DESC::DIRECTION;
	LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
	LightDesc.vDiffuse = _float4(0.8f, 0.8f, 0.65f, 1.f);
	LightDesc.vDirection = _float4(0.f, -1.f, 0.5f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
	m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
	m_pGameInstance->SetUp_ShadowNF();


	Ready_UI();
	Ready_Layer_Player();
	Ready_MonsterTest();
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
	CMonsterTest::MONSTERTEST_DESC MobDesc{};
	MobDesc.eCurLevel = LEVEL::GAMEPLAY;
	MobDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	MobDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
	MobDesc.modelData = make_pair(LEVEL::GAMEPLAY, TEXT("Prototype_Component_Model_FalseSovereign"));
	MobDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	MobDesc.fRotationPerSec = XMConvertToRadians(90.f);
	MobDesc.fSpeedPerSec = 10.f;
	MobDesc.vInitPosition = _float3(3497.f, 147.84f, 3267.5f);
	MobDesc.pAnimationTag = "Born1";
	MobDesc.strFolderPath = "../Bin/Resource/Model/FalseSovereign/Notify";
	MobDesc.fHP = 100.f;
	MobDesc.fAttackDmg = 1.f;
	if(FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_MonsterTest"),
		ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Layer_MonsterTest"), &MobDesc)))
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
	ImGui::Begin("Test");
	
	
	if (ImGui::CollapsingHeader("SSAO"))
	{
				ImGui::InputFloat("RADIUS", &m_fRadius);
		
				ImGui::DragFloat("MAX_DISTANCE", &m_fMaxDistance, 1.f, 1.f, 50.f, "%.1f");
		
		#ifdef _DEBUG
				m_pGameInstance->Setting_SSAO(m_fRadius, m_fMaxDistance);
		#endif // _DEBUG
	}
	ImGui::End();

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
