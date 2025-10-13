#include "EditorPch.h"
#include "EditorApp.h"

#include "Event_Level.h"

#include "Level_Edit.h"
#include "Level_Shader.h"
#include "Level_Animation.h"
#include "Level_Effect.h"
#include "Level_Map.h"
#include "Level_UI.h"

CEditorApp::CEditorApp()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CEditorApp::Initialize()
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

	// ImGui Context 연동
	ImGui::SetCurrentContext(m_pGameInstance->Get_ImGuiContext());

	// Jolt Collision Layer SetUp
	SetUp_CollisionLayer();
	// Jolt PhysicsSystem SetUp
	m_pGameInstance->SetUp_PhysicsSystem();

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		CRigidbody::Create(m_pDevice, m_pContext))))
		CRASH("Rigidbody");

	Ready_Event();
	Start_Level();

	return S_OK;
}

void CEditorApp::Post_Update()
{
	// Level 전환
	if (true == m_isChangeLevel)
	{
		m_isChangeLevel = false;
		if (FAILED(m_pGameInstance->Clear_Memory()))
			return;

		CLevel* pLevel = { nullptr };

		switch (m_eNextLevel)
		{
		case LEVEL::EDIT:
			pLevel = CLevel_Edit::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::SHADER:
			pLevel = CLevel_Shader::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::ANIMATION:
			pLevel = CLevel_Animation::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::EFFECT:
			pLevel = CLevel_Effect::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::MAP:
			pLevel = CLevel_Map::Create(m_pDevice, m_pContext);
			break;
		case LEVEL::UI:
			pLevel = CLevel_UI::Create(m_pDevice, m_pContext);
			break;
		}

		if (nullptr == pLevel)
			CRASH("Level Create Fail");

		m_pGameInstance->Open_Level(ENUM_CLASS(m_eNextLevel), pLevel);
		
	}
}

void CEditorApp::Update(_float fTimeDelta)
{
	m_pGameInstance->Update_Engine(fTimeDelta);

	ImGui::Begin("Level");


	if (ImGui::Button("Shader", ImVec2(100.f, 50.f)))
	{
		CHANGE_LEVEL_EVENT event{ LEVEL::SHADER, true };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
	}
	if (ImGui::Button("Animation", ImVec2(100.f, 50.f)))
	{
		CHANGE_LEVEL_EVENT event{ LEVEL::ANIMATION, true };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
	}
	if (ImGui::Button("Effect", ImVec2(100.f, 50.f)))
	{
		CHANGE_LEVEL_EVENT event{ LEVEL::EFFECT, true };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
	}
	if (ImGui::Button("Map", ImVec2(100.f, 50.f)))
	{
		CHANGE_LEVEL_EVENT event{ LEVEL::MAP, true };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
	}
	if (ImGui::Button("UI", ImVec2(100.f, 50.f)))
	{
		CHANGE_LEVEL_EVENT event{ LEVEL::UI, true };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
	}

	
	ImGui::End();

	//if (m_pGameInstance->Get_DIKeyState(DIK_F1) == KEYSTATE::DOWN)
	//{
	//	CHANGE_LEVEL_EVENT event{ LEVEL::ANIMATION, true };
	//	m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
	//}
}

void CEditorApp::Render()
{
	_float4 vClearColor = _float4(0.f, 0.f, 1.f, 1.f);
	m_pGameInstance->Render_Begin(&vClearColor);
	m_pGameInstance->Draw();
	m_pGameInstance->Render_End();
}

void CEditorApp::SetUp_CollisionLayer()
{
	// Object To BroadPhase
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::MAP), ENUM_CLASS(BPLAYER::NON_MOVE));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectToBP(ENUM_CLASS(COLLISIONLAYER::ENEMY), ENUM_CLASS(BPLAYER::MOVE));

	// Object VS Object
	m_pGameInstance->SetUp_ObjectFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(COLLISIONLAYER::ENEMY));

	// Object VS BroadPhase
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::PLAYER), ENUM_CLASS(BPLAYER::MOVE));
	m_pGameInstance->SetUp_ObjectVsBPFilter(ENUM_CLASS(COLLISIONLAYER::ENEMY), ENUM_CLASS(BPLAYER::MOVE));
}

void CEditorApp::Ready_Event()
{
	m_pGameInstance->Subscribe<CHANGE_LEVEL_EVENT>(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), [this](const CHANGE_LEVEL_EVENT& event) {
			m_isChangeLevel = true;
			m_eNextLevel = event.eNextLevel;
			m_isLoad = event.isLoad;
		});
}

void CEditorApp::Start_Level()
{
	CHANGE_LEVEL_EVENT event{ LEVEL::EDIT, true };
	m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Event_Change_Level"), event);
}

CEditorApp* CEditorApp::Create()
{
	CEditorApp* pInstance = new CEditorApp();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : MainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEditorApp::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	m_pGameInstance->Release_Engine();
	Safe_Release(m_pGameInstance);
}
