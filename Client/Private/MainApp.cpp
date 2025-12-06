#include "ClientPch.h"
#include "MainApp.h"
#include "GameSystem.h"

#include "Event_Level.h"

#include "Level_Loading.h"

#include "Level_Logo.h"
#include "Level_GamePlay.h"
#include "Level_Heaven.h"
#include "Level_Test.h"

#include "Level_Test_UI.h"

#include "SpringCamera.h"
#include "SkyBox.h"
#include "Effect_Prefab.h"
#include "Ability.h"
#include "SceneCamera.h"

#include "CustomFont.h"

#include "Mouse.h"

#include "Scan.h"
#include "MotionTrail.h"

CMainApp::CMainApp()
	: m_pGameInstance { CGameInstance::GetInstance() },
	m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMainApp::Initialize()
{
	srand(static_cast<_uint>(time(nullptr)));

	ENGINE_DESC EngineDesc = {};
	EngineDesc.hWnd = g_hWnd;
	EngineDesc.hInst = g_hInst;
	EngineDesc.eMode = WINMODE::WIN;
	EngineDesc.iSizeX = g_iWinSizeX;
	EngineDesc.iSizeY = g_iWinSizeY;
	EngineDesc.iNumLevel = ENUM_CLASS(LEVEL::END);
	EngineDesc.iNumChannel = FMOD_CHANNEL_MAX;
	EngineDesc.iNumCollisionLayer = ENUM_CLASS(COLLISIONLAYER::END);

	if (FAILED(m_pGameInstance->Ready_Engine(EngineDesc, &m_pDevice, &m_pContext)))
		return E_FAIL;

	// ImGui Context Setting
	ImGui::SetCurrentContext(m_pGameInstance->Get_ImGuiContext());

	// Jolt Collision Layer SetUp
	SetUp_CollisionLayer();
	// Jolt PhysicsSystem SetUp
	m_pGameInstance->SetUp_PhysicsSystem();

	// Game System Ready
	m_pGameSystem->Ready_GameSystem(m_pDevice, m_pContext);

	Ready_Prototype_ForStatic();
	Ready_Sequence();
	Ready_Sequence_Item();
	Ready_Event();
	Start_Level();

	return S_OK;
}

void CMainApp::Post_Update()
{
	// Level Change
	if (true == m_isChangeLevel)
	{
		// Wait Thread End
		m_pGameInstance->Wait_Thread_End();

		m_isChangeLevel = false;

		if (true == m_isLoad)
		{
			// PhysicX Update시 Remove ID 수집 중지
			m_pGameInstance->IsChangeLevel_ForPhysicX(true);

			// Memory Clear (Sound, Camera, Light, ETC)
			if (FAILED(m_pGameInstance->Clear_Memory()))
				CRASH("Clear");

			// GameSystem Clear
			m_pGameSystem->Clear_Resource();

			if (FAILED(m_pGameInstance->Clear_CurrentLevel_Resources(ENUM_CLASS(LEVEL::LOADING))))
				CRASH("Clear Resource");

			m_pGameInstance->Open_Level(ENUM_CLASS(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, m_eNextLevel));
		}
		else
		{
			if (FAILED(m_pGameInstance->Clear_CurrentLevel_Resources(ENUM_CLASS(m_eNextLevel))))
				CRASH("Clear Resource");

			CLevel* pLevel = { nullptr };

			switch (m_eNextLevel)
			{
			case LEVEL::LOGO:
				pLevel = CLevel_Logo::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::GAMEPLAY:
				pLevel = CLevel_GamePlay::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::HEAVEN:
				pLevel = CLevel_Heaven::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::TEST:
				pLevel = CLevel_Test::Create(m_pDevice, m_pContext);
				break;
			//case LEVEL::TEST_UI:
			//	pLevel = CLevel_Test_UI::Create(m_pDevice, m_pContext);
			//	break;
			}
			ASSERT_CRASH(pLevel);

			m_pGameInstance->Open_Level(ENUM_CLASS(m_eNextLevel), pLevel);

			m_pGameInstance->IsChangeLevel_ForPhysicX(false);
		}
	}
}

void CMainApp::Update(_float fTimeDelta)
{
	m_pGameInstance->Update_Engine(fTimeDelta);

	ImGuiID DockingID = ImGui::GetID("Dock");
	ImGui::DockSpaceOverViewport(DockingID, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::Begin("Frame");
	_char szFrame[MAX_PATH] = {};
	sprintf_s(szFrame, MAX_PATH, "Frame : %d", m_iFrame);
	ImGui::Text(szFrame);
	ImGui::End();

	m_fTimeAcc += fTimeDelta;
	++m_iCnt;
	if (m_fTimeAcc > 1.f)
	{
		m_fTimeAcc = 0.f;
		m_iFrame = m_iCnt;
		m_iCnt = 0;
	}

}

void CMainApp::Render()
{
	_float4 vClearColor = _float4(0.f, 0.f, 1.f, 1.f);
	m_pGameInstance->Render_Begin(&vClearColor);
	m_pGameInstance->Draw();
	m_pGameInstance->Render_End();
}

void CMainApp::SetUp_CollisionLayer()
{
	// Object To BroadPhase
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::NONE), ENUM_CLASS(BPLAYER::NONE));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::MAP), ENUM_CLASS(BPLAYER::NON_MOVE));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::QTE), ENUM_CLASS(BPLAYER::SENSOR));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::ENEMY), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::NPC), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::ALTER), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::SEQUENCE), ENUM_CLASS(BPLAYER::MOVE));

	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::DETECT), ENUM_CLASS(BPLAYER::SENSOR));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::ATTACK), ENUM_CLASS(BPLAYER::SENSOR)); // Player
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::KNOCKBACK), ENUM_CLASS(BPLAYER::SENSOR));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::PARRY), ENUM_CLASS(BPLAYER::SENSOR));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::SKILL), ENUM_CLASS(BPLAYER::SENSOR));
	
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK), ENUM_CLASS(BPLAYER::SENSOR));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::ENEMY_HARDATTACK), ENUM_CLASS(BPLAYER::SENSOR));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::ENEMY_SKILL), ENUM_CLASS(BPLAYER::SENSOR));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::PARRY), ENUM_CLASS(BPLAYER::SENSOR));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::GRAB), ENUM_CLASS(BPLAYER::SENSOR));

	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::INTERACTION), ENUM_CLASS(BPLAYER::SENSOR)); // 상호 작용할 INTERACTION
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::GRAPPLE), ENUM_CLASS(BPLAYER::SENSOR));		 // PULL할 INTERACTION
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::SLIDE), ENUM_CLASS(BPLAYER::SENSOR));		 // 땅바닥 Slide


	// Object VS Object
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::QTE), ENUM_CLASS(COLLISIONLAYER::MAP));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::ENEMY));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::MAP));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::NPC));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::ATTACK), ENUM_CLASS(COLLISIONLAYER::ENEMY)); // Player
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::KNOCKBACK), ENUM_CLASS(COLLISIONLAYER::ENEMY));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PARRY), ENUM_CLASS(COLLISIONLAYER::ATTACK));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PARRY), ENUM_CLASS(COLLISIONLAYER::SKILL));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PARRY), ENUM_CLASS(COLLISIONLAYER::KNOCKBACK));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::SKILL), ENUM_CLASS(COLLISIONLAYER::ENEMY));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::DETECT), ENUM_CLASS(COLLISIONLAYER::ENEMY));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::DETECT), ENUM_CLASS(COLLISIONLAYER::PLAYER));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::DETECT), ENUM_CLASS(COLLISIONLAYER::GRAPPLE));
	//m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::DETECT), ENUM_CLASS(COLLISIONLAYER::INTERACTION));

	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::ENEMY), ENUM_CLASS(COLLISIONLAYER::MAP));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::ENEMY_HARDATTACK));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::ENEMY_SKILL));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::GRAB));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::GRAPPLE));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::SLIDE));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::NPC), ENUM_CLASS(COLLISIONLAYER::INTERACTION));

	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::NPC), ENUM_CLASS(COLLISIONLAYER::MAP));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::ALTER), ENUM_CLASS(COLLISIONLAYER::MAP));





	// Object VS BroadPhase
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::QTE), ENUM_CLASS(BPLAYER::NON_MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(BPLAYER::NON_MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(BPLAYER::SENSOR));

	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::ENEMY), ENUM_CLASS(BPLAYER::NON_MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::ENEMY), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::ENEMY), ENUM_CLASS(BPLAYER::SENSOR));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::ALTER), ENUM_CLASS(BPLAYER::NON_MOVE));

	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::NPC), ENUM_CLASS(BPLAYER::NON_MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::NPC), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::NPC), ENUM_CLASS(BPLAYER::SENSOR));

	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::DETECT), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::DETECT), ENUM_CLASS(BPLAYER::SENSOR));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::ATTACK), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::KNOCKBACK), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::PARRY), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::SKILL), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::ENEMY_HARDATTACK), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::ENEMY_SKILL), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::GRAB), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::PARRY), ENUM_CLASS(BPLAYER::SENSOR));

	//m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::INTERACTION), ENUM_CLASS(BPLAYER::SENSOR)); // 어떤 범위랑 할것인지? => SENSOR
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::INTERACTION), ENUM_CLASS(BPLAYER::MOVE));

	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::GRAPPLE), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::GRAPPLE), ENUM_CLASS(BPLAYER::SENSOR)); 
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::SLIDE), ENUM_CLASS(BPLAYER::MOVE)); 
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::SLIDE), ENUM_CLASS(BPLAYER::SENSOR)); 
}

void CMainApp::Ready_Event()
{
	m_pGameInstance->Subscribe<CHANGE_LEVEL_EVENT>(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), [this](const CHANGE_LEVEL_EVENT& event) {
			m_isChangeLevel = true;
			m_eNextLevel = event.eNextLevel;
			m_isLoad = event.isLoad;
		});
}

void CMainApp::Ready_Prototype_ForStatic()
{
#pragma region SHADER
	// Shader_VtxMesh
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		CRASH("Shader_VtxMesh");

	// Shader_VtxMesh_Instance
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh_Instance.hlsl"), VTXMESHINSTANCE::Elements, VTXMESHINSTANCE::iNumElements))))
		CRASH("Shader_VtxMesh");

	// DeferredShader_Map
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_DeferredShader_Map"),
		CDeferredShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements, TEXT("Shader_Map")))))
		CRASH("DeferredShader_Map");

	// DeferredShader_Map
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_DeferredShader_Map_Instance"),
		CDeferredShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh_Instance.hlsl"), VTXMESHINSTANCE::Elements, VTXMESHINSTANCE::iNumElements, TEXT("Shader_Map_Instance")))))
		CRASH("DeferredShader_Map");

	
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_NonAnimMesh_Water"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh_Water.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		CRASH("Failed to Add Prototype Shader NonAnimMesh Water");

	// Shader_VtxPropAnimMesh
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPropAnimMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPropAnimMesh.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		CRASH("Shader_VtxAnimMesh");

	// Shader_VtxPropAnimMesh
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		CRASH("Shader_VtxPropAnimMesh");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMeshCharacter"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshCharacter.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		CRASH("Failed Load AnimMesh Shader");

	// Shader_UI_VtxPosTex ..Shader for UI
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		CRASH("Shader_UI_VtxPosTex");

	// Shader_VtxSkyBox
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxSkyBox"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxSkyBox.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		CRASH("Shader_VtxSkyBox");

	// Shader_VtxArrow
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MonsterProp"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh_MonsterProp.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		CRASH("Shader_VtxMesh");

	// Shader_VtxInstance_PointParticle
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Shader_VtxInstance_PointParticle"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstance_PointParticle.hlsl"), VTXPOINTPARTICLE::Elements, VTXPOINTPARTICLE::iNumElements))))
		CRASH("Shader_VtxInstance_PointParticle");

	//Shader_VtxTrailMesh
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Shader_VtxTrailMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxTrailMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		CRASH("Shader_VtxTrailMesh");

	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Shader_VtxFXRect"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxFXRect.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements));

	// Shader_SFX_Sonora
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Sonora"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_SFX_Sonora.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		CRASH("Shader_ScreenEffect");

	// Shader_SFX_AugustaUlti
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_SFX_Burst.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		CRASH("Shader_ScreenEffect");
	// Shader_VtxAnimMesh_Instance
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimMesh_Instance.hlsl"), VTXANIMMESH_INSTANCE::Elements, VTXANIMMESH_INSTANCE::iNumElements))))
		CRASH("Shader_VtxAnimMesh_Instance");

	// Shader_SFX_GalbrenaUlti_Instance
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_SFX_Burst_Instance.hlsl"), VTXRECTINSTANCE::Elements, VTXRECTINSTANCE::iNumElements))))
		CRASH("Shader_ScreenEffect");

	// Shader_Mouse
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Mouse"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		CRASH("Shader_Mouse");

	// Shader_Scan
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Scan"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxScan.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements))))
		CRASH("Failed to Add Prototype Shader Scan");

	// Shader_MotionTrail
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MotionTrail"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimMesh_MotionTrail.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		CRASH("Failed to Add Prototype Shader MotionTrail");


#pragma endregion

#pragma region COMPUTE_SHADER
	SHADER_MACRO eShaderMacro = {
	{"THREAD_X", "64" }
	,{"THREAD_Y", "1" }
	,{"THREAD_Z", "1" }
	, { NULL, NULL }
	};
	string strEntryPoint = "CSMain";

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"),
		CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMesh.hlsl")
			, eShaderMacro, strEntryPoint))))
		CRASH("Compute AnimMesh Shader");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshCharacter"),
		CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMeshCharacter.hlsl")
			, eShaderMacro, strEntryPoint))))
		CRASH("Failed Load AnimMesh Character Shader");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshFly"),
		CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMeshFly.hlsl")
			, eShaderMacro, strEntryPoint))))
		CRASH("Compute FlyAnimMesh Shader");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"),
		CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMeshNonRib.hlsl")
			, eShaderMacro, strEntryPoint))))
		CRASH("Compute NonRibAnimMesh Shader");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ComputeVtxInstance_AnimMesh"),
		CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxInstance_AnimMesh.hlsl")
			, eShaderMacro, strEntryPoint))))
		CRASH("Compute Instance_AnimMesh Shader");

	string strEntryParticle = "main";
	SHADER_MACRO eShaderMacroParticle = {
	{"THREAD_X", "64" }
	,{"THREAD_Y", "1" }
	,{"THREAD_Z", "1" }
	, { NULL, NULL }
	};
	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Shader_ComputeShader_Particle"),
		CComputeShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_ParticleUpdate_CS.hlsl"), eShaderMacroParticle, strEntryParticle));
	//Skinning Format
	eShaderMacro = {
		{"THREAD_X", "128" }
		,{"THREAD_Y", "1" }
		,{"THREAD_Z", "1" }
		, { NULL, NULL }
	};
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh_Skinning"),
		CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMesh_Skining.hlsl")
			, eShaderMacro, strEntryPoint))))
		CRASH("Compute Instance_AnimMesh Shader");
		
	eShaderMacro = {
		{"THREAD_X", "256" }
		,{"THREAD_Y", "1" }
		,{"THREAD_Z", "1" }
		, { NULL, NULL }
	};

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_ComputeVtxAnimMorph"),
		CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMorph.hlsl")
			, eShaderMacro, strEntryPoint))))
		CRASH("Failed Load AnimMorph Shader");
#pragma endregion

#pragma region COLLIDER

	// Rigidbody
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		CRigidbody::Create(m_pDevice, m_pContext))))
		CRASH("Rigidbody");

	// Collider
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Collider"),
		CCollider::Create(m_pDevice, m_pContext))))
		CRASH("Collider");
#pragma endregion

#pragma region VIBUFFER
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_FXRect"),
		CVIBuffer_Point::Create(m_pDevice, m_pContext))))
		CRASH("Failed to Add Prototype VIBuffer FXRect");

	if(FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
		CRASH("Failed to Add Prototype VIBuffer Rect");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Sphere"),
		CVIBuffer_Sphere::Create(m_pDevice, m_pContext))))
		CRASH("Failed to Add Prototype VIBuffer Sphere");

	if(FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect_Instance"),
		CVIBuffer_Rect_Instance::Create(m_pDevice, m_pContext, 10))))
		CRASH("Failed to Add Prototype VIBuffer Rect Instance");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Cube"),
		CVIBuffer_Cube::Create(m_pDevice, m_pContext))))
		CRASH("Failed to Add Prototype VIBuffer Cube");

	
#pragma endregion


	_fmatrix PreMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Wolf"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreMatrix, "../Bin/Resource/Dummy/Wolf/Wolf.dat"))))
		CRASH("Model Dummy");

	// SpringCamera
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_SpringCamera"),
		CSpringCamera::Create(m_pDevice, m_pContext))))
		CRASH("SpringCamera");

	//
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Font"),
	//	CCustomFont::Create(m_pDevice, m_pContext))))
	//	CRASH("Text Prototype Create Failed.");
		
	// Skybox
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Skybox"),
		CSkyBox::Create(m_pDevice, m_pContext))))
		CRASH("Skybox");

	// Prefab
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Prefab"),
		CEffect_Prefab::Create(m_pDevice, m_pContext))))
		CRASH("Prefab");

	// Ability 
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Ability"),
		CAbility::Create(m_pDevice, m_pContext))))
		CRASH("Ability");

	//Decal
	m_pGameSystem->Load_EffectDecalData_FromFolder("../Bin/Resource/Effect/Prefabs/Common/Decal");

#pragma region MOUSE
	// Mouse_Texture
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Mouse"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resource/UI/Mouse/CursorHi.png"), 1))))
		CRASH("Mouse_Texture");

	// Mouse
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Mouse"),
		CMouse::Create(m_pDevice, m_pContext))))
		CRASH("Mouse");
#pragma endregion

#pragma region SCAN

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Scan"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resource/Effect/Scan/T_Mask_18023.png"), 1))))
		CRASH("Failed to Add Prototype Texture Scan");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Scan"),
		CScan::Create(m_pDevice, m_pContext))))
		CRASH("Failed to Add Prototype GameObject Scan");

#pragma endregion

#pragma region MOTION_TRAIL

	if(FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_MotionTrail"),
		CMotionTrail::Create(m_pDevice, m_pContext))))
		CRASH("Failed to Add Prototype GameObject MotionTrail");

#pragma endregion
}

void CMainApp::Ready_Sequence_Item()
{
	// Sequence Item
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_SceneCamera"),
		CSceneCamera::Create(m_pDevice, m_pContext))))
		CRASH("Camera");
}

void CMainApp::Ready_Sequence()
{
	m_pGameSystem->Load_Sequence("../Bin/Resource/Sequence/Scene/");
}

void CMainApp::Start_Level()
{
	CHANGE_LEVEL_EVENT event{ LEVEL::LOGO, true };
	//CHANGE_LEVEL_EVENT event{ LEVEL::TEST, true };
	//CHANGE_LEVEL_EVENT event{ LEVEL::TEST_UI, true };
	//CHANGE_LEVEL_EVENT event{ LEVEL::TEST, true };

	m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
}

CMainApp* CMainApp::Create()
{
	CMainApp* pInstance = new CMainApp();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : MainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMainApp::Free()
{
	__super::Free();

	m_pGameSystem->Release_System();
	Safe_Release(m_pGameSystem);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	m_pGameInstance->Release_Engine();
	Safe_Release(m_pGameInstance);
}