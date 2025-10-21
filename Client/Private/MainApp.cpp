#include "ClientPch.h"
#include "MainApp.h"
#include "GameSystem.h"

#include "Event_Level.h"

#include "Level_Loading.h"

#include "Level_Logo.h"
#include "Level_GamePlay.h"
#include "Level_Test.h"

CMainApp::CMainApp()
	: m_pGameInstance { CGameInstance::GetInstance() },
	m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
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
			// Level???랁븯吏 ?딆? 媛앹껜??Release
			if (FAILED(m_pGameInstance->Clear_Memory()))
				return;
			m_pGameInstance->Open_Level(ENUM_CLASS(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, m_eNextLevel));
		}
		else
		{
			CLevel* pLevel = { nullptr };

			switch (m_eNextLevel)
			{
			case LEVEL::LOGO:
				pLevel = CLevel_Logo::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::GAMEPLAY:
				pLevel = CLevel_GamePlay::Create(m_pDevice, m_pContext);
				break;
			case LEVEL::TEST:
				pLevel = CLevel_Test::Create(m_pDevice, m_pContext);
				break;
			}
			ASSERT_CRASH(pLevel);

			m_pGameInstance->Open_Level(ENUM_CLASS(m_eNextLevel), pLevel);
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
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::MAP), ENUM_CLASS(BPLAYER::NON_MOVE));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::ENEMY), ENUM_CLASS(BPLAYER::MOVE));

	// Object VS Object
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::ENEMY));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::MAP));
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::PLAYER));

	// Object VS BroadPhase
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(BPLAYER::NON_MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(BPLAYER::MOVE));

	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::ENEMY), ENUM_CLASS(BPLAYER::MOVE));
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
	// Shader_VtxMesh
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		CRASH("Shader_VtxMesh");

	// Shader_VtxAnimMesh
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		CRASH("Shader_VtxAnimMesh");

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

	// Rigidbody
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		CRigidbody::Create(m_pDevice, m_pContext))))
		CRASH("Rigidbody");

	// Collider
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Collider"),
		CCollider::Create(m_pDevice, m_pContext))))
		CRASH("Collider");
}

void CMainApp::Start_Level()
{
	CHANGE_LEVEL_EVENT event{ LEVEL::LOGO, true };
	m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Event_Change_Level"), event);
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

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	m_pGameInstance->Release_Engine();
	Safe_Release(m_pGameSystem);
	Safe_Release(m_pGameInstance);
}
