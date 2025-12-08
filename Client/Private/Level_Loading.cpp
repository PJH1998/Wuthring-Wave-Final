#include "ClientPch.h"
#include "Level_Loading.h"
#include "GameSystem.h"
#include "Event_Level.h"

// ========Loader========
#include "Loader_Logo.h"
#include "Loader_GamePlay.h"
#include "Loader_Heaven.h"
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
    : CLevel { pDevice, pContext }, m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
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
	Update_LoadingScreen(fTimeDelta);
	if (false == m_isFinished)
	{
		m_pLoader->Update(fTimeDelta);
	}

	//this_thread::sleep_for(chrono::milliseconds(1));
	if (false == m_isFinished && true == m_pGameInstance->IsWorkFinish())
	{
		cout << "Finish" << endl;

		_float fProgress = m_pLoader->Get_Progress();
		if(1.01f < fProgress)
			m_isFinished =  true;

		//// ui test
		//CCustom_UI* pLoadRootUI = dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(L"UI_Loading"));
		//static_cast<CAnimator_UI*>(pLoadRootUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(0);
	}

    if (	true == m_isFinished)
    {
		cout << "Loading End" << endl;
        CHANGE_LEVEL_EVENT event{ m_eNextLevel, false };
        m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
    }
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
	case LEVEL::HEAVEN:
		m_pLoader = CLoader_Heaven::Create(m_pDevice, m_pContext);
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
			OutputDebugString(L"[Level_Loading::Ready_LoadingScreen] Texture Load Failed. The texture may have already been loaded.\n");
	}

	// VIBuffer_Rect
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Level_Loading::Ready_LoadingScreen] VIBuffer_Rect Load Failed. The VIBuffer_Rect may have already been loaded.\n");

	// VIBuffer_Rect_Instance_UI
	CVIBuffer_Rect_Instance_UI::RECT_INSTANCE_UI_DESC tRectInstDesc = {};
	tRectInstDesc.iNumInstance = 500U;
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect_Instance_UI"),
		CVIBuffer_Rect_Instance_UI::Create(m_pDevice, m_pContext, &tRectInstDesc))))
		OutputDebugString(L"[Level_Loading::Ready_LoadingScreen] VIBuffer_Rect_Instance_UI Load Failed. The VIBuffer_Rect_Instance_UI may have already been loaded.\n");

	// Shader
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		OutputDebugString(L"[Level_Loading::Ready_LoadingScreen] Shader Load Failed. The Shader may have already been loaded.\n");

	// Shader_Instance
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxInstance.hlsl"), VTXUIINSTANCE::Elements, VTXUIINSTANCE::iNumElements))))
		OutputDebugString(L"[Level_Loading::Ready_LoadingScreen] Shader_Instance Load Failed. The Shader_Instance may have already been loaded.\n");

	// Text Shader Instance
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_Text_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_TextInstance.hlsl"), VTXUIINSTANCE::Elements, VTXUIINSTANCE::iNumElements))))
		OutputDebugString(L"[Level_Loading::Ready_LoadingScreen] Shader Load Failed. The Shader may have already been loaded.\n");

	// Animator_UI
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_Component_Animator_UI",
		CAnimator_UI::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Level_Loading::Ready_LoadingScreen] Animator_UI Load Failed. The Animator_UI may have already been loaded.\n");

	// Custom UI
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Button",
		CUI_Button::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Level_Loading::Ready_LoadingScreen] UI_Button Load Failed. The CUI_Button may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Image",
		CUI_Image::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Level_Loading::Ready_LoadingScreen] UI_Image Load Failed. The UI_Image may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Text",
		CUI_Text::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Level_Loading::Ready_LoadingScreen] UI_Text Load Failed. The CUI_Text may have already been loaded.\n");

	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_Loading",
		CUI_Loading::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Level_Loading::Ready_LoadingScreen] UI_HUD Load Failed. The UI_HUD may have already been loaded.\n");

	// Font
	if (FAILED(m_pGameInstance->Add_Font(L"WW_Bold", "../../Client/Bin/Resource/Font/Font_SUITE/SUITE-Bold.ttf")))
		OutputDebugString(L"[Level_Loading::Ready_LoadingScreen] Font Load Failed. The Font may have already been loaded.\n");


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



	// Create Text..
	_wstring strLoadingText = L"";
	CUI_Text* pLoadingText = m_pGameSystem->Create_FontToScreen(_float2{1760.f, 920.f}, strLoadingText, TEXT_COLOR_TYPE::TT_PROGRESS, 0.35f, L"UI_Text_ProgressTest");
	m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, L"Layer_UI_Text", pLoadingText);

	_wstring strPercentText = L"%";
	CUI_Text* pPercentText = m_pGameSystem->Create_FontToScreen(_float2{1780.f, 920.f}, strPercentText, TEXT_COLOR_TYPE::TT_NORMAL, 0.4f, L"UI_Text_PercentTest");
	m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, L"Layer_UI_Text", pPercentText);

	m_pGameInstance->Add_RootUI(L"UI_Text_LoadingTest", pLoadingText);
	m_pGameInstance->Add_RootUI(L"UI_Text_PercentTest", pPercentText);
}

void CLevel_Loading::Update_LoadingScreen(_float fTimeDelta)
{
	// 로딩 진행상황 정보를 가져올 수 있다면, 그걸 통해 UI의 진행바 및 로딩 퍼센트 등 업데이트하기
	// 추가로 loading 과정 중에 텍스트의 로드도 필요

	// 그러면 텍스트를 어떻게 ui 위에 띄우는가? ui매니저 만들어뒀던거로 가져오면 될듯


	CCustom_UI* pLoadBarUI = dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(L"UI_Loading"))->Find_ChildObject(L"LoadBar");
	CUI_Text* pLoadingTextUI = dynamic_cast<CUI_Text*>(m_pGameInstance->Find_UIObject(L"UI_Text_LoadingTest"));


	auto loadBarUIDesc = pLoadBarUI->Get_UIDesc();
	auto& loadingTextUIDesc = pLoadingTextUI->Get_TextUIDesc();

	// 임시 테스트. 나중에 값 받아올 수 있으면 받아오긴
	const _float fTestLoadingTime = 3.f;

	//m_fElapsedTime += fTimeDelta * (1.f / fTestLoadingTime);
	m_fElapsedTime = m_pLoader->Get_Progress();
	if (m_fElapsedTime > 1.0f) m_fElapsedTime = 1.0f;

	loadBarUIDesc.vecInstanceDescs[0].vClipTexcoordX.y = m_fElapsedTime;
	pLoadBarUI->Set_UIDesc(loadBarUIDesc);

	loadingTextUIDesc.strText = to_wstring((int)(m_fElapsedTime * 100.f));

	_uint iAlignmentPixel = 0;	// 오른정렬을 위함
	static _uint iOriginPosX = UINT_MAX;
	if (iOriginPosX == UINT_MAX) iOriginPosX = loadingTextUIDesc.vScreenPos.x;

	for (auto& textInstDesc : loadingTextUIDesc.vecInstanceDescs)
		iAlignmentPixel += static_cast<_uint>(textInstDesc.vSInstRight.x);

	loadingTextUIDesc.vScreenPos.x = iOriginPosX - iAlignmentPixel * loadingTextUIDesc.fScale;
	pLoadingTextUI->Set_TextUIDesc(loadingTextUIDesc);

	//if (!m_isLoadFadeOut && m_fElapsedTime >= 1.0f)
	//{
	//	m_isLoadFadeOut = true;
	//	CCustom_UI* pLoadRootUI = dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(L"UI_Loading"))->Find_ChildObject(L"SubRoot_Loading");
	//	static_cast<CAnimator_UI*>(pLoadRootUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(0);
	//}
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

	//m_pGameInstance->Clear_RootUI();	// 이러니까 비동기 로드로 인하여 "다음 레벨에서 추가된 RootUI"도 로딩 중 컨테이너 내부에서 제거되어 문제 발생함. 개별로 직접 컨테이너로부터 제거.
	m_pGameInstance->Remove_RootUI(L"UI_Loading");
	m_pGameInstance->Remove_RootUI(L"UI_Text_LoadingTest");
	m_pGameInstance->Remove_RootUI(L"UI_Text_PercentTest");

    Safe_Release(m_pLoader);
	Safe_Release(m_pGameSystem);
}
