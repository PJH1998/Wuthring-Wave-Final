#include "EditorPch.h"
#include "AnimationTool.h"
#include "ModelLoader.h"
#include "AnimationActor.h"


#pragma region 기본 함수들
CAnimationTool::CAnimationTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance { CGameInstance::GetInstance()}
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CAnimationTool::Initialize(LEVEL eLevel)
{
    m_eCurLevel = eLevel;

    // 1. FBX 파일을 Dat화 해주는 Loader 생성.
    m_pLoader = CModelLoader::Create();

   

    return S_OK;
}

void CAnimationTool::Render()
{
    Render_Editor();
}

void CAnimationTool::Render_Editor()
{
    // 0. Debug Render => 현재 상태를 출력.
    Render_DebugWindow();

    // 1. 알파값 조절해서 투명하게 만들 수 있음.
    ImGui::Begin(u8"Editor", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar
        | ImGuiWindowFlags_AlwaysVerticalScrollbar);

    
    ImGui::Text("Settings");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 WindowColor = style.Colors[ImGuiCol_WindowBg];
    ImGui::SliderFloat("Editor Opacity", &m_fEditorAlpha, 0.0f, 1.0f);
    const ImVec4 NewColor = ImVec4(WindowColor.x, WindowColor.y, WindowColor.z, m_fEditorAlpha);
    style.Colors[ImGuiCol_WindowBg] = NewColor;
    ImGui::NewLine();

    // 2. MenuTabBar 구현
    Render_Menu();

    ImGui::End();
}

void CAnimationTool::Render_DebugWindow()
{
    ImGuiIO& io = ImGui::GetIO();

    ImVec2 windowPos = ImVec2(300.f, g_iWinSizeY - 300.f);
    ImVec2 windowSize = ImVec2(300.f, 300.f);

    // Cond_Once: 최초 한 번만 위치 적용 → 이후 드래그 가능
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);

    // NoCollapse만 유지, 이동 가능하게
    ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoCollapse);

    _float4 camPos = {};
    camPos = *m_pGameInstance->Get_CamPos();
    //XMStoreFloat3(&camPos, m_pCameraTransformCom->Get_State(STATE::POSITION));
    ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    // 현재 선택된 파일 타입 표시
    /*const char* typeNames[] = { "CONVERT_FBX", "LOAD_DAT", "EDIT_ANIMATION", "END"};
    ImGui::Text("MODE : %s", typeNames[ENUM_CLASS(m_eMode)]);*/

    ImGui::End();
}

// Mode 지정.
void CAnimationTool::Render_SelectMode()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Mode"))
        {
            // MenuItem을 사용하면 더 깔끔한 메뉴를 만들 수 있습니다.
            if (ImGui::MenuItem("Convert FBX to DAT")) { m_eMode = MODE::CONVERT_FBX_TO_DAT; }
            if (ImGui::MenuItem("View DAT")) { m_eMode = MODE::VIEW_DAT; }
            if (ImGui::MenuItem("Edit Animation")) { m_eMode = MODE::EDIT_ANIMATION; }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void CAnimationTool::Render_Menu()
{
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Save_LoadFBX"))
        {
            m_pLoader->Update();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Load_DAT"))
        {
            LoadDat();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Model_DAT"))
        {
            RenderUI_ViewDat();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

}

void CAnimationTool::RenderUI_ConvertFbx()
{
    // 1. 직접 하나 선택해서 FBX 파일을 DAT화 한다. => 일부.
    m_pLoader->Update();
}

void CAnimationTool::RenderUI_ViewDat()
{
    // 1. 선택한 Dat 파일을 Load하기. => Prototype 생성.
    // 우선. GameObject를 새로 만들고 Prototype 등록.

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("Prototype", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Model"))
        {
            // 2. 현재 생성된 프로토타입 목록을 보여주기.
            RenderUI_Prototype();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    
    
   
}

void CAnimationTool::RenderUI_EditAnimation()
{

}

void CAnimationTool::LoadDat()
{
    // 1. Load하고 Load된 프로토타입 이름을 저장해두기.
    _wstring wStrModelName = {};

    _string strModelName = "Prototype_Component_Model_";

    _string strModelPath = {};

    // 2. ImGui에서 파일을 오픈해서 해당 파일을 이용해서 Prototype Model 동적으로 생성
    CModel* pModelCom = { nullptr };

    IGFD::FileDialogConfig config;

    config.path = "../../Client/Bin/Resource/";
    config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

    ImGuiFileDialog::Instance()->OpenDialog("DAT File Load", "Import File", ".dat", config);

    if (ImGuiFileDialog::Instance()->Display("DAT File Load")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
            strModelPath = ImGuiFileDialog::Instance()->GetCurrentFileName();

            // .dat 잘라내기.
            size_t last_dot_pos = strModelPath.find_last_of('.');
            if (last_dot_pos != std::string::npos) {
                // 0번째 위치부터 '.' 위치까지 문자열을 잘라냅니다.
                strModelName += strModelPath.substr(0, last_dot_pos);
                
            }
            else
            {
                MSG_BOX("경로 잘못됨");
                return;
            }

            _uint iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };

            _matrix		PreTransformMatrix = XMMatrixIdentity();
            PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XM_PI);

            wStrModelName = StringToWstring(strModelName);

            // Model Prototype 생성.
            HRESULT hr = Add_Prototype_AnimModel(wStrModelName, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str());
            if (FAILED(hr))
            {
                MSG_BOX("경로 잘못되었거나, 중복 생성.");
                return;
            }
            // 3. 생성이 완료되었으면 m_ModelNames에 추가. 
            m_ModelNames.emplace_back(strModelName);
        }
        ImGuiFileDialog::Instance()->Close();
    }

}

void CAnimationTool::RenderUI_Prototype()
{
    // 1. 생성된 Prototype 목록들을 확인하기.

    _wstring objTag = {};
    _wstring modelTag = {};

    ImGui::BeginChild("left pane", ImVec2(500, 0), true);


    static int iSelectedIndex = -1;
    _uint id = 0;

    for (auto& modelName : m_ModelNames)
    {
        if (ImGui::Selectable(modelName.c_str(), id == iSelectedIndex))
        {
            iSelectedIndex = id;
            m_Selected_PrototypeModelTag = modelName;
            m_wSelected_PrototypeModelTag = StringToWstring(modelName);
        }
    }

    ImGui::EndChild();

    // 다음 위젯을 같은 줄에 그리도록 설정합니다.
    ImGui::SameLine();

    if (iSelectedIndex >= 0 && iSelectedIndex < m_ModelNames.size())
        Render_Model_Inspector();

    
}

HRESULT CAnimationTool::Add_Prototype_AnimModel(_wstring strPrototypeName, MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath)
{
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
        , strPrototypeName
        , CModel::Create(m_pDevice, m_pContext, eType, PreTransformMatrix, pFilePath))))
        return E_FAIL;

    return S_OK;
}

void CAnimationTool::Render_Model_Inspector()
{
    ImGui::BeginChild("Right pane", ImVec2(500, 0), true);
    //ImGuiIO& io = ImGui::GetIO();

    // 오른쪽 위 위치 계산 (창 크기 300x250 고려)
    //ImVec2 vPos = ImVec2(g_iWinSizeX - g_iWinSizeX * 0.25f, 0.f); // 오른쪽에서 310픽셀, 위에서 10픽셀
    //ImGui::SetNextWindowPos(vPos, ImGuiCond_Always);
    //ImGui::SetNextWindowSize(ImVec2(g_iWinSizeX * 0.25f, g_iWinSizeY), ImGuiCond_Once);

    //ImGui::Begin("AnimationActor_Transform");
    //ImGui::Text(m_Selected_PrototypeModelTag.c_str());

    static float fPosition[3] = { 0.f, 0.f, 0.f };
    ImGui::InputFloat3("Position", fPosition);

    static float fRotation[3] = { 0.f, 0.f, 0.f };
    ImGui::InputFloat3("Rotation", fRotation);

    static float fScale[3] = { 1.f, 1.f, 1.f };
    ImGui::InputFloat3("Scale", fScale);

    static float fSpeedPerSec = {};
    ImGui::InputFloat("Speed", &fSpeedPerSec);

    static float fRotationPerSec = {};
    ImGui::InputFloat("RotationSpeed", &fRotationPerSec);

    static unsigned int iShaderPath = {};
    static const unsigned int min_val = 0;
    static const unsigned int max_val = 1;
    ImGui::SliderScalar("Shader Path", ImGuiDataType_U32, &iShaderPath, &min_val, &max_val);

    if (ImGui::Button("Create Instance"))
    {
        CAnimationActor::ANIMATION_ACTOR_DESC Desc{};
        Desc.strModelTag = m_wSelected_PrototypeModelTag;
        Desc.strShaderTag = TEXT("Prototype_Component_Shader_VtxAnimMesh"); // 일단 하드코딩..
        Desc.iShaderPath = iShaderPath;
        memcpy(&Desc.vPostion, fPosition, sizeof(_float3));
        memcpy(&Desc.vRotation, fRotation, sizeof(_float3));
        memcpy(&Desc.vScale, fScale, sizeof(_float3));
        Desc.eLevel = m_eCurLevel;

        _wstring wstrObjTag = TEXT("Prototype_GameObject_Actor_");
        

        // 마지막 모델 이름만 잘라내기.
        size_t last_dot_pos = m_wSelected_PrototypeModelTag.find_last_of('_');
        if (last_dot_pos != std::string::npos) {
            // 마지막 _ 위치로부터 생성.
            wstrObjTag += m_wSelected_PrototypeModelTag.substr(last_dot_pos + 1, m_wSelected_PrototypeModelTag.size());
        }
        else
        {
            MSG_BOX("경로 잘못됨");
            return;
        }


        // 1. Animation Actor용 Prototype 생성
        if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
            , wstrObjTag
            , CAnimationActor::Create(m_pDevice, m_pContext))))
        {
            MSG_BOX("Animation Actor Prototype 생성 실패");
            return;
        }

        // 2. 생성한 Prototype Clone
        if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel)
            , wstrObjTag
            , ENUM_CLASS(m_eCurLevel),TEXT("Layer_Actor"), &Desc)))
        {
            MSG_BOX("Animation Actor Clone 생성 실패");
            return;
        }
    }

    ImGui::EndChild();
    //ImGui::End();
}


#pragma endregion



wstring CAnimationTool::StringToWstring(const std::string& str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (len == 0) {
        return L"";
    }

    wstring wstr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);

    if (!wstr.empty() && wstr.back() == L'\0') {
        wstr.pop_back();
    }

    return wstr;
}

CAnimationTool* CAnimationTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel)
{
    CAnimationTool* pInstance = new CAnimationTool(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eLevel)))
    {
        MSG_BOX("Failed to Create : Level_Animation");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAnimationTool::Free()
{
    CBase::Free();
    Safe_Release(m_pLoader);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    m_ModelNames.clear();
    m_ActorNames.clear();
}

