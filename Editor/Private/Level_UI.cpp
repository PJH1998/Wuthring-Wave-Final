#include "EditorPch.h"
#include "Level_UI.h"

#include "Event_Level.h"
#include "Custom_UI.h"
#include "FreeCamera.h"

// 임시로 여기에 매크로로..
#define         STR2WSTR(str)           _wstring(str.begin(), str.end())
#define         STR_ONLYFILENAME(str)   std::filesystem::path(str).stem().string();





CLevel_UI::CLevel_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_UI::Initialize()
{
    // ==============================
    // * Add Prototypes
    // ==============================
    
    // Custom UI
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::UI), L"Prototype_GameObject_UI_Custom",
        CCustom_UI::Create(m_pDevice, m_pContext))))
        CRASH(Failed to add Custom_UI prototype.);

    // FreeCamera
    //if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::UI), L"Prototype_GameObject_Camera_Free",
    //    CFreeCamera::Create(m_pDevice, m_pContext))))
    //    CRASH(Failed to add FreeCamera prototype.);

    // ==============================
    // * Add GameObjects
    // ==============================
    
    // FreeCamera
    //CFreeCamera::CAMERA_DESC	CameraDesc{};
    //
    //CameraDesc.vEye = _float4(0.f, 20.f, -15.f, 1.f);
    //CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
    //CameraDesc.fFovy = XMConvertToRadians(60.0f);
    //CameraDesc.fNear = 0.1f;
    //CameraDesc.fFar = 500.f;
    //CameraDesc.fSpeedPerSec = 20.f;
    //CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);
    //CameraDesc.fMouseSensor = .2f;
    //
    //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::UI), TEXT("Prototype_GameObject_Camera_Free"),
    //    ENUM_CLASS(LEVEL::UI), L"Layer_Camera", &CameraDesc)))
    //    return E_FAIL;


    // ==============================
    // * Add Lights
    // ==============================
    LIGHT_DESC			LightDesc{};

    LightDesc.eType = LIGHT_DESC::TYPE::DIRECTION;
    LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);	// Light 방향
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);		// Light 색상 및 밝기의 세기
    LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);	// Light 환경광으로 가정. 최소 밝기 보장에 관여.
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);		// Light 반사광.

    if (FAILED(m_pGameInstance->Add_Light(L"Light_Default", LightDesc)))
        return E_FAIL;

    return S_OK;
}

void CLevel_UI::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("UI"));
    const _wstring strLayerTag = L"Layer_UI_Custom";


	ImGui::Begin("Editor Window..");


    // ===== [Button] Load Image =====
	if (ImGui::Button("Load Image..", ImVec2(100.f, 50.f)))
	{
		IGFD::FileDialogConfig config;

		config.path = "../../Client/Bin/Resource/UI/";
		config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;
		ImGuiFileDialog::Instance()->OpenDialog("UI_Image_Load","Select Image",	".png,.jpg,.dds,.tga", config);
	}
    // End ==============================


    // ===== [Logic] Load FilePath, Create & Store CustomUI =====
    _wstring strFilePath = {}, strFileName = {};

    if (ImGuiFileDialog::Instance()->Display("UI_Image_Load"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())    // 파일 선택 시
        {
            string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            string fileName = STR_ONLYFILENAME(ImGuiFileDialog::Instance()->GetCurrentFileName());

            strFilePath = STR2WSTR(filePath);
            strFileName = STR2WSTR(fileName);

            CCustom_UI::CUSTOM_UI_DESC tCustomUIDesc = {};
            tCustomUIDesc.fSizeX = 100;
            tCustomUIDesc.fSizeY = 100;
            tCustomUIDesc.fX = g_iWinSizeX / 2.f;
            tCustomUIDesc.fY = g_iWinSizeY / 2.f;
            tCustomUIDesc.strFilePath = strFilePath;
            tCustomUIDesc.strFileName = strFileName;

            if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::UI), L"Prototype_GameObject_UI_Custom",
                ENUM_CLASS(LEVEL::UI), L"Layer_UI_Custom", &tCustomUIDesc)))
                CRASH(Failed to add Custom_UI gameobject.);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // 게임오브젝트 찾아와서 로컬 컨테이너에 push_back..을 어떻게할까
    
    // End ==============================



	ImGui::End();
}

void CLevel_UI::Render()
{
    
}

CLevel_UI* CLevel_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_UI* pInstance = new CLevel_UI(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_UI::Free()
{
    __super::Free();

}
