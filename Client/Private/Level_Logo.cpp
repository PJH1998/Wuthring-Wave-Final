#include "ClientPch.h"
#include "Level_Logo.h"

#include "Event_Level.h"

#include "Dummy.h"
#include "LogoMaleRover.h"
#include "SceneCamera.h"
#include "GameSystem.h"

CLevel_Logo::CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CLevel_Logo::Initialize()
{
	// SetUp OctoTree
	m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096, 4096, 4096));
	m_pGameSystem->Clone_MapObjects(m_eCurLevel);
	Ready_Layer_LogoMaleRover();
	Ready_Layer_LogoFemaleRover();
	Ready_UI();
	Ready_Camera();

	LIGHT_DESC LightDesc{};
	LightDesc.eType = LIGHT_DESC::DIRECTION;
	LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);
	LightDesc.vDiffuse = _float4(0.6f, 0.6f, 0.8f, 1.f);
	LightDesc.vDirection = _float4(0.f, -1.f, 0.5f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
	m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
	m_pGameInstance->SetUp_CameraNF();

	m_pGameInstance->SettingFog(true);
	m_pGameInstance->Set_LUT_Index(1);
    return S_OK;
}

void CLevel_Logo::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Logo"));

    if (m_pGameInstance->Get_DIKeyState(DIK_F1) == KEYSTATE::DOWN)
    {
        CHANGE_LEVEL_EVENT event{ LEVEL::GAMEPLAY, true };
        m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
    }
	if (m_pGameInstance->Get_DIKeyState(DIK_F2) == KEYSTATE::DOWN)
	{
		CHANGE_LEVEL_EVENT event{ LEVEL::TEST, true };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
	}
	//if (m_pGameInstance->Get_DIKeyState(DIK_F3) == KEYSTATE::DOWN)
	//{
	//	CHANGE_LEVEL_EVENT event{ LEVEL::TEST_UI, true };
	//	m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
	//}
}

void CLevel_Logo::Render()
{
}

void CLevel_Logo::Ready_Camera()
{
	// Camera
	CCamera::CAMERA_DESC CameraDesc = {};
	CameraDesc.fFovy = XMConvertToRadians(60.f);
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.f;
	CameraDesc.vEye = _float4(2.81f, 0.70f, -3.53f, 1.f);
	CameraDesc.vAt = _float4(0.96f, 0.30f, -1.74f, 1.f);
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.f);
	CameraDesc.fMouseSensor = 0.004f;
	
	if (FAILED(m_pGameInstance->Add_Camera(ENUM_CLASS(LEVEL::LOGO), TEXT("Camera_Scene"), ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_GameObject_SceneCamera"), &CameraDesc)))
		CRASH("Add Camera");

	if (FAILED(m_pGameInstance->Change_MainCamera(ENUM_CLASS(LEVEL::LOGO), TEXT("Camera_Scene"))))
		CRASH("Change Camera");
}

void CLevel_Logo::Ready_Layer_LogoMaleRover()
{
	_float3 vScale{}, vRotation{}, vPosition{};
	vScale = { 1.f, 1.f, 1.f };
	vRotation = { 0.f, 0.f, 0.f };
	vPosition = { 0.f, 0.f, 0.f };

	CCharacter::CHARACTER_DESC LogoMaleRoverDesc;
	LogoMaleRoverDesc = PlayerData::GetLogoMaleRoverCloneData(vScale, vRotation, vPosition, m_eCurLevel);
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
		ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Actor_LogoMaleRover")
		, ENUM_CLASS(m_eCurLevel), TEXT("Layer_LogoRover"), &LogoMaleRoverDesc)))
		CRASH("Prototype LogoMaleRover");

	cout << "Level Male Rover" << endl;
}

void CLevel_Logo::Ready_Layer_LogoFemaleRover()
{
	_float3 vScale{}, vRotation{}, vPosition{};
	vScale = { 1.f, 1.f, 1.f };
	vRotation = { 0.f, 0.f, 0.f };
	vPosition = { 0.f, 0.f, 0.f };

	CCharacter::CHARACTER_DESC LogoFemaleRoverDesc;
	LogoFemaleRoverDesc = PlayerData::GetLogoFemaleRoverCloneData(vScale, vRotation, vPosition, m_eCurLevel);
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
		ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Actor_LogoFemaleRover")
		, ENUM_CLASS(m_eCurLevel), TEXT("Layer_LogoRover"), &LogoFemaleRoverDesc)))
		CRASH("Prototype LogoFemaleRover");

	cout << "Level Female Rover" << endl;
}

void CLevel_Logo::Ready_UI()
{
	// UI
	const   _uint       iDestLevel = ENUM_CLASS(m_eCurLevel);
	const _wstring		strLayertag_UI = L"Layer_Custom_UI";
	const _wstring		strPrototypeTag_UI[] = {
		 L"Prototype_GameObject_Custom_UI_Container_Logo"
	};
	for (auto& strPrototypeTag : strPrototypeTag_UI)
	{
		CUIObject* pTargetUI = static_cast<CUIObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, strPrototypeTag, PROTOTYPE::GAMEOBJECT));
		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, strLayertag_UI, pTargetUI)))
			CRASH("Failed to Add RootUI to Object_Manager.");
	}

	cout << "[Level_Logo::Ready_UI] Logo UI Loaded!" << endl;
	// _UI
}


CLevel_Logo* CLevel_Logo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Logo* pInstance = new CLevel_Logo(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Logo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Logo::Free()
{
	//m_pGameInstance->Clear_RootUI();

    __super::Free();

	Safe_Release(m_pGameSystem);
}
