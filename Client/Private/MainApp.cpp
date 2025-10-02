#include "ClientPch.h"
#include "MainApp.h"

#include "Event_Level.h"

#include "Level_Loading.h"

CMainApp::CMainApp()
	: m_pGameInstance { CGameInstance::GetInstance() }
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

	return S_OK;
}

void CMainApp::Post_Update()
{
	// Level 전환
	if (true == m_isChangeLevel)
	{
		m_isChangeLevel = false;
		if (true == m_isLoad)
		{
			// Level에 속하지 않은 객체들 Release
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
				// TODO
				break;
			case LEVEL::GAMEPLAY:
				// TODO
				break;
			}

			if (nullptr == pLevel)
				return;

			m_pGameInstance->Open_Level(ENUM_CLASS(m_eNextLevel), pLevel);
		}
	}
}

void CMainApp::Update(_float fTimeDelta)
{
	m_pGameInstance->Update_Engine(fTimeDelta);
}

HRESULT CMainApp::Render()
{
	_float4 vClearColor = _float4(0.f, 0.f, 1.f, 1.f);
	m_pGameInstance->Render_Begin(&vClearColor);
	m_pGameInstance->Draw();
	m_pGameInstance->Render_End();

	return S_OK;
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
	Safe_Release(m_pGameInstance);
}
