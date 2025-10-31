#include "ClientPch.h"
#include "Loader_GamePlay.h"
#include"GameSystem.h"
#include"MapObject.h"

#pragma region FALSE_SOVEREIGN
#include "MonsterTest.h"
#pragma endregion

// UI
#include "Custom_UI.h"
#include "UI_Button.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "Animator_UI.h"
#include "UI_HUD.h"
// _UI

CLoader_GamePlay::CLoader_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_GamePlay::Initialize()
{
	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });
	//m_pGameInstance->Add_Work([this]() {Load_MonsterTest(); Complete_Load(); });

	m_pGameInstance->Add_Work([this]() {Load_Augusta(); Complete_Load(); });
	//m_pGameInstance->Add_Work([this]() {Load_Rover(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Player(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_MonsterTest(); Complete_Load(); });

	m_pGameInstance->Add_Work([this]() {Load_UI(); Complete_Load(); });

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Texture()
{
	cout << "Texture" << endl;

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Model()
{
	m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/Asphodel_Barrens_1030_second_final/", m_eCurLevel);
	//m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/The_False_Sovereign_1031_Final/", m_eCurLevel);
	cout << "Model" << endl;

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Shader()
{
	cout << "Shader" << endl;

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Object()
{
	cout << "Object" << endl;

	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject"),
		CMapObject::Create(m_pDevice, m_pContext));

    return S_OK;
}

HRESULT CLoader_GamePlay::Load_Player()
{
	return S_OK;
}

HRESULT CLoader_GamePlay::Load_Augusta()
{
	return S_OK;
}

HRESULT CLoader_GamePlay::Load_Rover()
{
	return S_OK;
}

HRESULT CLoader_GamePlay::Load_UI()
{
	const   _uint       iDestLevel = ENUM_CLASS(LEVEL::GAMEPLAY);


	// ==============================
	cout << "[CLoader_Test_UI] Texture" << endl;
	// ==============================

	vector<CCustom_UI::CUSTOM_UITREE_DESC> vecDescs = {};       // parsed data from json

	// * Json Parse                 // for pre-loading textures
	// UI_HUD
	//_string strFilePath_UI_HUD = "../../Client/Bin/Resource/UI/FJson/UITree/TestHUD.json"; // ksta
	_string strFilePath_UI_HUD = "../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD_251030_2037.json"; // ksta
	vecDescs.push_back(Load_UITree(strFilePath_UI_HUD));

	for (auto& treeDesc : vecDescs)
	{
		for (auto& infoDesc : treeDesc.vecUIInfoDescs)
		{
			const   _wstring    strFilePath = infoDesc.tUIDesc.strFilePath;
			const   _wstring	strFileName = infoDesc.tUIDesc.strFileName;
			const   _uint       iNumFiles = infoDesc.tUIDesc.iNumFiles;

			if (strFileName == L"T_JiabeilinaEnergyBgCombined")
				int i = 10;

			infoDesc.tUIDesc.strFilePath;
			if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Texture_Custom_") + strFileName,
				CTexture::Create(m_pDevice, m_pContext, strFilePath.c_str(), iNumFiles))))
				OutputDebugString(L"[CLoader_Test_UI::Ready_Prototypes] Texture Load Failed. The texture may have already been loaded.\n");
		}
	}

	// ==============================
	cout << "[CLoader_Test_UI] Model" << endl;
	// ==============================
	// 
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


	// ==============================
	cout << "[CLoader_Test_UI] Shader" << endl;
	// ==============================

	// Shader
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		OutputDebugString(L"[Loader_Test_UI::Load_Shader] Shader Load Failed. The Shader may have already been loaded.\n");

	// Shader_Instance
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxInstance.hlsl"), VTXUIINSTANCE::Elements, VTXUIINSTANCE::iNumElements))))
		OutputDebugString(L"[Loader_Test_UI::Load_Shader] Shader_Instance Load Failed. The Shader_Instance may have already been loaded.\n");

	// ==============================
	cout << "[CLoader_Test_UI] Object" << endl;
	// ==============================

	// * Components Load
	// Animator_UI
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_Component_Animator_UI",
		CAnimator_UI::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[CCustom_UI::Load_Shader] Animator_UI Load Failed. The Animator_UI may have already been loaded.\n");



	// * Objects Load
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


	// ==============================
	cout << "[CLoader_Test_UI][UI Custom] Prototype" << endl;
	// ==============================

	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_HUD",
		CUI_HUD::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Test_UI::Load_Prototype] UI_HUD Load Failed. The UI_HUD may have already been loaded.\n");

	return S_OK;
}

HRESULT CLoader_GamePlay::Load_MonsterTest()
{
	//cout << "MonsterTest" << endl;

	// Prototype_Component_BehaviorTree_Test
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_BehaviorTree_FalseSovereign"),
		CBehavior_Tree::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/FalseSovereign/FalseSovereign_BT.json"))))
		CRASH("BehaviorTree Create Failed");

	// Prototype_Component_AnimMachine_Test
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_AnimMachine_FalseSovereign"),
		CAnimMachine::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Model/FalseSovereign/Animation/FalseSovereign_StateMachine.json"))))
		CRASH("Monster AnimMachine Create Failed");

	// Prototype_Component_Model_FalseSovereign
	//_fmatrix PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	_fmatrix PreTransformMatrix = XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) * XMMatrixRotationY(XMConvertToRadians(180.f));
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_FalseSovereign"),
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, "../../Client/Bin/Resource/Model/FalseSovereign/FalseSovereignTest.dat"))))
		CRASH("Prototype Create Failed");

	// Prototype_GameObject_MonsterTest
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_MonsterTest"),
		CMonsterTest::Create(m_pDevice, m_pContext))))
		CRASH("MonsterTest Prototype Create Failed");
	return S_OK;
}







CCustom_UI::CUSTOM_UITREE_DESC CLoader_GamePlay::Load_UITree(_string strFilePath)
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



CLoader_GamePlay* CLoader_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLoader_GamePlay* pInstance = new CLoader_GamePlay(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Loader_GamePlay");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader_GamePlay::Free()
{
    __super::Free();
}
