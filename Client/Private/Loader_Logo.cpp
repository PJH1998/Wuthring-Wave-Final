#include "ClientPch.h"
#include "Loader_Logo.h"

#include "Dummy.h"
#include "ShadowDummy.h"
#include "Parser.h"
#include "GameSystem.h"
#include "LogoMaleRover.h"
#include "LogoFemaleRover.h"
//#include "Custom_UI.h"

#include "UI_Logo.h"
#include "UI_Button.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "UI_Text_Damage.h"
#include "UI_Button_Interact.h"
#include "Animator_UI.h"

#include "SceneCamera.h"
#include "MapObject.h"

CLoader_Logo::CLoader_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_Logo::Initialize()
{
	m_iNumLoadingThread = 8;
	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_MonsterTable(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_LogoMaleRover(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_LogoFeMaleRover(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_UI(); Complete_Load(); });

    return S_OK;
}

HRESULT CLoader_Logo::Load_Texture()
{
	cout << "Texture" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_Model()
{
	m_pGameInstance->Load_Resource("../Bin/Resource/Map/Logo/");
	//m_pGameSystem->Ready_Prototype_Map("../../Client/Bin/Resource/Map/MapData/Logo/", m_eCurLevel);
	m_pGameSystem->Ready_Prototype_Map("../../Client/Bin/Resource/Map/MapData/Logo_Test/", m_eCurLevel);

	cout << "Model" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_Shader()
{
	cout << "Shader" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_Object()
{
	m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject"),
		CMapObject::Create(m_pDevice, m_pContext));

	cout << "Object" << endl;

    return S_OK;
}

HRESULT CLoader_Logo::Load_LogoMaleRover()
{
	_wstring wStrModelTag = L"Prototype_Component_Model_MaleRover";
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/Logo/Male/LogoMaleRover.dat";
	_matrix	PreTransformMatrix = XMMatrixIdentity();
	//_float fSize = 0.01f;
	_float fSize = 0.0001f;

	//PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(90.f));// * XMMatrixRotationY(XMConvertToRadians(180.f));
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(45.f));// * XMMatrixRotationY(XMConvertToRadians(180.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");


	// 2. StateMachine 초기화
	_wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_MaleRover";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrStateMachineTag,
		CStateMachine::Create(m_pDevice, m_pContext))))
		CRASH("PlayerState Machine");


	// 3. 객체 초기화
	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_LogoMaleRover");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrActorTag
		, CLogoMaleRover::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	cout << "Logo Male Rover" << endl;
	return S_OK;
}

HRESULT CLoader_Logo::Load_LogoFeMaleRover()
{
	_wstring wStrModelTag = L"Prototype_Component_Model_FemaleRover";
	_string strFilePath = "../../Client/Bin/Resource/Model/Player/Logo/Female/LogoFemaleRover.dat";
	_matrix	PreTransformMatrix = XMMatrixIdentity();
	//_float fSize = 0.01f;
	_float fSize = 0.0001f;

	//PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(90.f));// * XMMatrixRotationZ(XMConvertToRadians(90.f));// *  XMMatrixRotationY(XMConvertToRadians(180.f));
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(45.f));// * XMMatrixRotationZ(XMConvertToRadians(90.f));// *  XMMatrixRotationY(XMConvertToRadians(180.f));

	// 1. 모델 초기화.
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrModelTag,
		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
		CRASH("Prototype Create Failed");


	// 2. StateMachine 초기화
	_wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_FemaleRover";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wStrStateMachineTag,
		CStateMachine::Create(m_pDevice, m_pContext))))
		CRASH("PlayerState Machine");


	// 3. 객체 초기화
	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_LogoFemaleRover");
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
		, wStrActorTag
		, CLogoFemaleRover::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	cout << "Logo FeMale Rover" << endl;

	return S_OK;
}

HRESULT CLoader_Logo::Load_UI()
{
	const   _uint       iDestLevel = ENUM_CLASS(m_eCurLevel);

	// ==============================
	cout << "[Loader_Test] Texture" << endl;
	// ==============================

	vector<CCustom_UI::CUSTOM_UITREE_DESC> vecDescs = {};       // parsed data from json

	// * Json Parse                 // for pre-loading textures

	_string strFilePath_UI_Logo = "../../Client/Bin/Resource/UI/FJson/UITree/Root_Logo.json"; // ksta
	vecDescs.push_back(Load_UITree(strFilePath_UI_Logo));

	//_string strFilePath_UI_Interact = "../../Client/Bin/Resource/UI/FJson/UITree/Root_Interact.json"; // ksta
	//vecDescs.push_back(Load_UITree(strFilePath_UI_Interact));


	for (auto& treeDesc : vecDescs)
	{
		for (auto& infoDesc : treeDesc.vecUIInfoDescs)
		{
			const   _wstring    strFilePath = infoDesc.tUIDesc.strFilePath;
			const   _wstring	strFileName = infoDesc.tUIDesc.strFileName;
			const   _uint       iNumFiles = infoDesc.tUIDesc.iNumFiles;

			infoDesc.tUIDesc.strFilePath;
			if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Texture_Custom_") + strFileName,
				CTexture::Create(m_pDevice, m_pContext, strFilePath.c_str(), iNumFiles))))
				OutputDebugString(L"[Loader_Logo::Load_UI] Texture Load Failed. The texture may have already been loaded.\n");
		}
	}


	// ==============================
	cout << "[Loader_Test] Model" << endl;
	// ==============================
	// 
	// VIBuffer_Rect
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Logo::Load_UI] VIBuffer_Rect Load Failed. The VIBuffer_Rect may have already been loaded.\n");

	// VIBuffer_Rect_Instance_UI
	CVIBuffer_Rect_Instance_UI::RECT_INSTANCE_UI_DESC tRectInstDesc = {};
	tRectInstDesc.iNumInstance = 500U;
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect_Instance_UI"),
		CVIBuffer_Rect_Instance_UI::Create(m_pDevice, m_pContext, &tRectInstDesc))))
		OutputDebugString(L"[Loader_Logo::Load_UI] VIBuffer_Rect_Instance_UI Load Failed. The VIBuffer_Rect_Instance_UI may have already been loaded.\n");

	// ==============================
	cout << "[Loader_Test] Shader" << endl;
	// ==============================

	// Shader
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		OutputDebugString(L"[Loader_Logo::Load_UI] Shader Load Failed. The Shader may have already been loaded.\n");

	// Shader_Instance
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxInstance.hlsl"), VTXUIINSTANCE::Elements, VTXUIINSTANCE::iNumElements))))
		OutputDebugString(L"[Loader_Logo::Load_UI] Shader_Instance Load Failed. The Shader_Instance may have already been loaded.\n");

	// Shader_Font
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_Text_Instance"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_TextInstance.hlsl"), VTXUIINSTANCE::Elements, VTXUIINSTANCE::iNumElements))))
		OutputDebugString(L"[Loader_Logo::Load_UI] Shader_TextInstance Load Failed. The Shader_TextInstance may have already been loaded.\n");


	// ==============================
	cout << "[Loader_Test] Object" << endl;
	// ==============================

	// * Components Load
	// Animator_UI
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_Component_Animator_UI",
		CAnimator_UI::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Logo::Load_UI] Animator_UI Load Failed. The Animator_UI may have already been loaded.\n");



	// * Objects Load
	// Custom UI
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Button",
		CUI_Button::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Logo:Load_UI] UI_Button Load Failed. The CUI_Button may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Image",
		CUI_Image::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Logo::Load_UI] UI_Image Load Failed. The UI_Image may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Text",
		CUI_Text::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Logo::Load_UI] UI_Text Load Failed. The CUI_Text may have already been loaded.\n");

	// Custom Text
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Text_Damage",
		CUI_Text_Damage::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Logo::Load_UI] UI_Text_Damage Load Failed. The UI_Text_Damage may have already been loaded.\n");
	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Button_Interact",
		CUI_Button_Interact::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Logo::Load_UI] UI_Button_Interact Load Failed. The UI_Text_Damage may have already been loaded.\n");

	// ==============================
	cout << "[Loader_Test][UI Custom] Prototype" << endl;
	// ==============================

	if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_Logo",
		CUI_Logo::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[Loader_Logo::Load_UI] UI_Logo Load Failed. The UI_Logo may have already been loaded.\n");


	return S_OK;

}

HRESULT CLoader_Logo::Load_MonsterTable()
{
	if (FAILED(CGameSystem::GetInstance()->LoadMonsterTable("../Bin/Resource/Data/MonsterTable.csv")))
		return E_FAIL;

	cout << "Monster Table" << endl;

	return S_OK;
}

CCustom_UI::CUSTOM_UITREE_DESC CLoader_Logo::Load_UITree(_string strFilePath)
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

CLoader_Logo* CLoader_Logo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLoader_Logo* pInstance = new CLoader_Logo(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Loader_Logo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader_Logo::Free()
{
    __super::Free();
}
