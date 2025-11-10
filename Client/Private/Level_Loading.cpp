#include "ClientPch.h"
#include "Level_Loading.h"

#include "Event_Level.h"

// ========Loader========
#include "Loader_Logo.h"
#include "Loader_GamePlay.h"
#include "Loader_Test.h"
#include "Loader_Test_UI.h"
//#include "Loader_Lord.h"
// =====================
//#include "BackGround.h"
//#include "LoadingBar.h"

#pragma region UI
#include "Custom_UI.h"
#include "UI_Button.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "Animator_UI.h"
#include "UI_Loading.h"
#pragma endregion


CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Loading::Initialize(LEVEL eNextLevel)
{
    m_eNextLevel = eNextLevel;

	Ready_LoadingScreen();

    if (FAILED(Ready_Prototype()))
        return E_FAIL;

    if (FAILED(Ready_Event()))
        return E_FAIL;

    if (FAILED(Ready_LoadingThread()))
        return E_FAIL;

    if (FAILED(Ready_GameObject()))
        return E_FAIL;

    return S_OK;
}

void CLevel_Loading::Update(_float fTimeDelta)
{


	//this_thread::sleep_for(chrono::milliseconds(1));
	if (false == m_isFinished && true == m_pGameInstance->IsWorkFinish())
	{
		cout << "Finish" << endl;
		m_isFinished =  true;
	}

    if (	true == m_isFinished)
    {
		cout << "Loading End" << endl;
        CHANGE_LEVEL_EVENT event{ m_eNextLevel, false };
        m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Event_Change_Level"), event);
    }

	if (false == m_isFinished)
		Update_LoadingScreen();
}

void CLevel_Loading::Render()
{
	//static _uint i = 0;
	//i++;
	//cout << "[CLevel_Loading::Render] Render Called! : " << i << endl;
}

HRESULT CLevel_Loading::Ready_Prototype()
{
    return S_OK;
}

HRESULT CLevel_Loading::Ready_Event()
{
    m_pGameInstance->Subscribe<LOADING_END_EVENT>(ENUM_CLASS(STATIC::NONE), TEXT("Event_Loading_End"), [this](const LOADING_END_EVENT& event) {
            if(false == m_isFinished)
                m_isFinished = event.isFinish;
        });

    return S_OK;
}

HRESULT CLevel_Loading::Ready_LoadingThread()
{
    switch (m_eNextLevel)
    {
    case LEVEL::LOGO:
        m_pLoader = CLoader_Logo::Create(m_pDevice, m_pContext);
        break;
	case LEVEL::GAMEPLAY:
		m_pLoader = CLoader_GamePlay::Create(m_pDevice, m_pContext);
		break;
	case LEVEL::TEST:
		m_pLoader = CLoader_Test::Create(m_pDevice, m_pContext);
		break;
	//case LEVEL::TEST_UI:
	//	m_pLoader = CLoader_Test_UI::Create(m_pDevice, m_pContext);
	//	break;
    }

	ASSERT_CRASH(m_pLoader);

	this_thread::sleep_for(chrono::seconds(1));

    return S_OK;
}

HRESULT CLevel_Loading::Ready_GameObject()
{
    return S_OK;
}

void CLevel_Loading::Ready_LoadingScreen()
{
	// UI.. Load
	_uint iDestLevel = ENUM_CLASS(LEVEL::LOADING);


	_string strFilePath_UI_Loading = "../../Client/Bin/Resource/UI/FJson/UITree/Root_Loading.json";
	auto treeDesc = Load_UITree(strFilePath_UI_Loading);
	for (auto& infoDesc : treeDesc.vecUIInfoDescs)
	{
		const   _wstring    strFilePath = infoDesc.tUIDesc.strFilePath;
		const   _wstring	strFileName = infoDesc.tUIDesc.strFileName;
		const   _uint       iNumFiles	= infoDesc.tUIDesc.iNumFiles;

		infoDesc.tUIDesc.strFilePath;
		if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Texture_Custom_") + strFileName,
			CTexture::Create(m_pDevice, m_pContext, strFilePath.c_str(), iNumFiles))))
			OutputDebugString(L"[Loader_Test::Ready_Prototypes] Texture Load Failed. The texture may have already been loaded.\n");
	}

	// VIBuffer_Rect
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test_UI::Load_Model] VIBuffer_Rect Load Failed. The VIBuffer_Rect may have already been loaded.\n");

	// VIBuffer_Rect_Instance_UI
	CVIBuffer_Rect_Instance_UI::RECT_INSTANCE_UI_DESC tRectInstDesc = {};
	tRectInstDesc.iNumInstance = 500U;
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect_Instance_UI"),
		CVIBuffer_Rect_Instance_UI::Create(m_pDevice, m_pContext, &tRectInstDesc))))
		OutputDebugString(L"[Loader_Test_UI::Load_Model] VIBuffer_Rect_Instance_UI Load Failed. The VIBuffer_Rect_Instance_UI may have already been loaded.\n");

	// Shader
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		OutputDebugString(L"[Loader_Test_UI::Load_Shader] Shader Load Failed. The Shader may have already been loaded.\n");

	// Shader_Instance
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxInstance.hlsl"), VTXUIINSTANCE::Elements, VTXUIINSTANCE::iNumElements))))
		OutputDebugString(L"[Loader_Test_UI::Load_Shader] Shader_Instance Load Failed. The Shader_Instance may have already been loaded.\n");

	// Animator_UI
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_Component_Animator_UI",
		CAnimator_UI::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[CCustom_UI::Load_Shader] Animator_UI Load Failed. The Animator_UI may have already been loaded.\n");

	// Custom UI
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Button",
		CUI_Button::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test_UI:Load_Object] UI_Button Load Failed. The CUI_Button may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Image",
		CUI_Image::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test_UI::Load_Object] UI_Image Load Failed. The UI_Image may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Text",
		CUI_Text::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test_UI::Load_Object] UI_Text Load Failed. The CUI_Text may have already been loaded.\n");

	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_Loading",
		CUI_Loading::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test_UI::Load_Prototype] UI_HUD Load Failed. The UI_HUD may have already been loaded.\n");


	const _wstring strLayertag_UI = L"Layer_Custom_UI";
	const _wstring strPrototypeTag_UI[] = {
		 L"Prototype_GameObject_Custom_UI_Container_Loading"
	};
	for (auto& strPrototypeTag : strPrototypeTag_UI)
	{
		CUIObject* pTargetUI = static_cast<CUIObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, strPrototypeTag, PROTOTYPE::GAMEOBJECT));
		if (FAILED(m_pGameInstance->Add_RootUI(L"UI_Loading", pTargetUI)))
			CRASH("Failed to Add RootUI to UI_Manager.");
		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, strLayertag_UI, pTargetUI)))
			CRASH("Failed to Add RootUI to Object_Manager.");
	}


}

void CLevel_Loading::Update_LoadingScreen()
{
	// 로딩 진행상황 정보를 가져올 수 있다면, 그걸 통해 UI의 진행바 및 로딩 퍼센트 등 업데이트하기
	// 추가로 loading 과정 중에 텍스트의 로드도 필요

	// 그러면 텍스트를 어떻게 ui 위에 띄우는가?

}

CCustom_UI::CUSTOM_UITREE_DESC CLevel_Loading::Load_UITree(_string strFilePath)
{
	ifstream file(strFilePath);
	json jUIInfoData = {};
	if (file.is_open()) {
		file >> jUIInfoData;
	}
	else
		CRASH("File Open Failed.");

	CCustom_UI::CUSTOM_UITREE_DESC tDesc = {};
	from_json(jUIInfoData, tDesc);

	return tDesc;
}

CLevel_Loading* CLevel_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevel)
{
    CLevel_Loading* pInstance = new CLevel_Loading(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eNextLevel)))
    {
        MSG_BOX("Failed to Create : Level_Loading");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Loading::Free()
{
    __super::Free();

	m_pGameInstance->Clear_RootUI();
    Safe_Release(m_pLoader);
}
