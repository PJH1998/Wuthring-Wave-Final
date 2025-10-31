#include "ClientPch.h"
#include "Loader_Test_UI.h"



#include "Custom_UI.h"

#include "UI_Button.h"
#include "UI_Image.h"
#include "UI_Text.h"

#include "Animator_UI.h"

#include "UI_HUD.h"

// ������ ������Ʈ �Ŵ������� �����̳� ������ �Ǵ� UI_HUD, UI_EscMenu, UI_TabMenu �� ���� �Ű�,
// ���� Update�� �θ� ���� ����ɰǵ� ���� UI�Ŵ�����? 

CLoader_Test_UI::CLoader_Test_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader{ pDevice, pContext }
{
}

HRESULT CLoader_Test_UI::Initialize()
{
    CoInitializeEx(nullptr, 0);
    m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
    m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
    m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
    m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });

    m_pGameInstance->Add_Work([this]() {Load_Prototype(); Complete_Load(); });
    m_pGameInstance->Add_Work([this]() {Load_Font(); Complete_Load(); });
    m_pGameInstance->Wait_Thread_End();

    return S_OK;
}

HRESULT CLoader_Test_UI::Load_Texture()
{
    // ==============================
    cout << "[CLoader_Test_UI] Texture" << endl;
    // ==============================
    const   _uint       iDestLevel = ENUM_CLASS(LEVEL::TEST_UI);

    vector<CCustom_UI::CUSTOM_UITREE_DESC> vecDescs = {};       // parsed data from json

    // * Json Parse                 // for pre-loading textures
    // UI_HUD
    //_string strFilePath_UI_HUD = "../../Client/Bin/Resource/UI/FJson/UITree/TestHUD.json"; // ksta
    _string strFilePath_UI_HUD = "../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD_251030_2037.json"; // ksta
    vecDescs.push_back(Load_Tree(strFilePath_UI_HUD));

    for (auto& treeDesc : vecDescs)
    {
        for (auto& infoDesc : treeDesc.vecUIInfoDescs)
        {
            const   _wstring    strFilePath = infoDesc.tUIDesc.strFilePath;
            const   _wstring	strFileName = infoDesc.tUIDesc.strFileName;
            const   _uint       iNumFiles   = infoDesc.tUIDesc.iNumFiles;

            if (strFileName == L"T_JiabeilinaEnergyBgCombined")
                int i = 10;

            infoDesc.tUIDesc.strFilePath;
            if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Texture_Custom_") + strFileName,
                CTexture::Create(m_pDevice, m_pContext, strFilePath.c_str(), iNumFiles))))
                OutputDebugString(L"[CLoader_Test_UI::Ready_Prototypes] Texture Load Failed. The texture may have already been loaded.\n");
        }
    }

    return S_OK;
}

HRESULT CLoader_Test_UI::Load_Model()
{
    // ==============================
    cout << "[CLoader_Test_UI] Model" << endl;
    // ==============================
    const   _uint       iDestLevel = ENUM_CLASS(LEVEL::TEST_UI);

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

    return S_OK;
}

HRESULT CLoader_Test_UI::Load_Shader()
{
    // ==============================
    cout << "[CLoader_Test_UI] Shader" << endl;
    // ==============================
    const   _uint       iDestLevel = ENUM_CLASS(LEVEL::TEST_UI);

    // Shader
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
        OutputDebugString(L"[Loader_Test_UI::Load_Shader] Shader Load Failed. The Shader may have already been loaded.\n");

    // Shader_Instance
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex_Instance"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_UI_VtxInstance.hlsl"), VTXUIINSTANCE::Elements, VTXUIINSTANCE::iNumElements))))
        OutputDebugString(L"[Loader_Test_UI::Load_Shader] Shader_Instance Load Failed. The Shader_Instance may have already been loaded.\n");

    return S_OK;
}

HRESULT CLoader_Test_UI::Load_Object()
{
    // ==============================
    cout << "[CLoader_Test_UI] Object" << endl;
    // ==============================
    const   _uint       iDestLevel = ENUM_CLASS(LEVEL::TEST_UI);

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




    return S_OK;
}

HRESULT CLoader_Test_UI::Load_Prototype()
{
    // ==============================
    cout << "[CLoader_Test_UI][UI Custom] Prototype" << endl;
    // ==============================
    const   _uint       iDestLevel = ENUM_CLASS(LEVEL::TEST_UI);

    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_HUD",
        CUI_HUD::Create(m_pDevice, m_pContext))))
        OutputDebugString(L"[Loader_Test_UI::Load_Prototype] UI_HUD Load Failed. The UI_HUD may have already been loaded.\n");

    return S_OK;
}

HRESULT CLoader_Test_UI::Load_Font()
{
    cout << "[CLoader_Test_UI] .. " << endl;

    if (FAILED(m_pGameInstance->Add_Font(L"WW_Medium", "../../Client/Bin/Resource/Font/Font_SUITE/SUITE-Medium.ttf", 16)))
        OutputDebugString(L"[Loader_Test_UI::Load_Font] Font Load Failed. The Font may have already been loaded.\n");

    return S_OK;
}

CCustom_UI::CUSTOM_UITREE_DESC CLoader_Test_UI::Load_Tree(_string strFilePath)
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

CLoader_Test_UI* CLoader_Test_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLoader_Test_UI* pInstance = new CLoader_Test_UI(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Loader_Test_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader_Test_UI::Free()
{
    __super::Free();
}
