#include "EditorPch.h"
#include "AnimationTool.h"
#include "ModelLoader.h"
#include "AnimationActor.h"
#include "AnimNotifyTool.h"


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

    // 2. Animation Notify를 등록 및 관리하는 클래스
    m_pAnimNotifyTool = CAnimNotifyTool::Create(m_pDevice, m_pContext, m_eCurLevel);
   

    return S_OK;
}

void CAnimationTool::Render()
{
    Render_Editor();
    
    // Animation Notify가 Visible 상태라면?
    if (m_IsVisibleNotify)
    {
        // 저장 시 모델 Tag로 저장할 때 Folder만 저장할까?
        _string strModelDatPath = m_ModelDatPaths[m_wSelected_PrototypeModelTag];
        ASSERT_CRASH(m_pAnimNotifyTool);
        m_pAnimNotifyTool->Process_Notify(m_Selected_AnimationTag, strModelDatPath, m_fDuration);
        m_pAnimNotifyTool->Render();
    }
        
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
    ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);


    // 현재 선택된 파일 타입 표시
    const char* typeNames[] = { "CONVERT_FBX", "LOAD_DAT", "CREATE_ACTOR","EDIT_ANIMATION", "NONE"};
    ImGui::Text("MODE : %s", typeNames[ENUM_CLASS(m_eMode)]);

    switch (m_eMode)
    {
    case MODE::CREATE_ACTOR:
    {
        if (!m_Selected_PrototypeModelTag.empty())
            ImGui::Text("Select Model : %s", m_Selected_PrototypeModelTag.c_str());
        else
            ImGui::Text("Select Model : None");
    }
        break;
    case MODE::EDIT_ANIMATION:
    {
        if (!m_Selected_AnimActorTag.empty())
            ImGui::Text("Select Actor : %s", m_Selected_AnimActorTag.c_str());
        else
            ImGui::Text("Select Actor : None");

        if(!m_Selected_AnimationTag.empty())
            ImGui::Text("Select Animation : %s", m_Selected_AnimationTag.c_str());
        else
            ImGui::Text("Select Model : None");
    }
        break;
    default:
        break;
    }

    ImGui::End();
}


void CAnimationTool::Render_Menu()
{
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("ConvertFBX"))
        {
            m_pLoader->Update();
            ImGui::EndTabItem();

            m_eMode = MODE::CONVERT_FBX_TO_DAT;
        }

        if (ImGui::BeginTabItem("LoadDAT"))
        {
            LoadDat();
            ImGui::EndTabItem();

            m_eMode = MODE::LOAD_DAT;
        }

        if (ImGui::BeginTabItem("CreateActor"))
        {
            RenderUI_CreateActor();
            ImGui::EndTabItem();

            m_eMode = MODE::CREATE_ACTOR;
        }

        if (ImGui::BeginTabItem("EditAnimation"))
        {
            RenderUI_EditAnimation();
            ImGui::EndTabItem();

            m_eMode = MODE::EDIT_ANIMATION;
        }

        ImGui::EndTabBar();
    }

}

void CAnimationTool::RenderUI_ConvertFbx()
{
    // 1. 직접 하나 선택해서 FBX 파일을 DAT화 한다. => 일부.
    m_pLoader->Update();
}

void CAnimationTool::RenderUI_CreateActor()
{
    // 1. 선택한 Dat 파일을 Load하기. => Prototype 생성.
    // 우선. GameObject를 새로 만들고 Prototype 등록.
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("Prototype", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Model"))
        {
            // 2. 현재 생성된 프로토타입 목록을 보여주기.
            RenderUI_ModelPrototype();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    
}

//선택된 객체의 애니메이션 전체 목록을 확인하고 애니메이션에 대한 작업을 진행.
void CAnimationTool::RenderUI_EditAnimation()
{
     // 1. 생성된 Prototype 목록들을 확인하기.
    _wstring objTag = {};
    _wstring modelTag = {};

    ImGui::BeginChild("left pane", ImVec2(500, 0), true);

    static int iSelectedIndex = -1;
    _uint id = 0;

    for (auto& actorName : m_ActorNames)
    {
        if (ImGui::Selectable(actorName.c_str(), id == iSelectedIndex))
        {
            iSelectedIndex = id;
            // 선택 정보저장.
            m_Selected_AnimActorTag = actorName;
            m_wSelected_AnimActorTag = StringToWstring(actorName);
        }
    }
    ImGui::EndChild();

    // 애니메이션 목록창까지는 같은 자식 개체로 생성.
    ImGui::SameLine();
    if (iSelectedIndex >= 0 && iSelectedIndex < m_ActorNames.size())
        RenderUI_AnimationList();
}

void CAnimationTool::LoadDat()
{
    // 1. Load하고 Load된 프로토타입 이름을 저장해두기.
    _wstring wStrModelName = {};

    _string strModelName = "Prototype_Component_Model_";

    _string strModelPath = {};

    // 2. ImGui에서 파일을 오픈해서 해당 파일을 이용해서 Prototype Model 동적으로 생성
    CModel* pModelCom = { nullptr };


    if (ImGui::Button("Load DAT File"))
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

        ImGuiFileDialog::Instance()->OpenDialog("DAT File Load", "Import File", ".dat", config);
    }
  
    ImVec2 vMinSize = ImVec2(600, 400);  // 최소 크기
    ImVec2 vMaxSize = ImVec2(800, 400); // 최대 크기

    if (ImGuiFileDialog::Instance()->Display(
        "DAT File Load", ImGuiWindowFlags_NoCollapse
        , vMinSize
        , vMaxSize)) {
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

            _matrix		PreTransformMatrix = XMMatrixIdentity();
            _float fSize = 0.1f;
            PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(XM_PI));

            wStrModelName = StringToWstring(strModelName);

            // Model Prototype 생성.
            HRESULT hr = Add_Prototype_AnimModel(wStrModelName, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str());
            if (FAILED(hr))
            {
                MSG_BOX("경로 잘못되었거나, 중복 생성.");
                return;
            }

            // 3. 생성이 완료되었으면 m_ModelNames에 추가. 
            m_ModelDatPaths.emplace(wStrModelName, strFilePath);
            m_ModelNames.emplace_back(strModelName);
        }
        ImGuiFileDialog::Instance()->Close();
    }

}

void CAnimationTool::RenderUI_ModelPrototype()
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

    ImGui::SameLine();

    if (iSelectedIndex >= 0 && iSelectedIndex < m_ModelNames.size())
        Render_Model_Detail();

    
}

void CAnimationTool::RenderUI_AnimationList()
{
    ImGui::BeginChild("Right pane", ImVec2(500, 0), true);

    // 1. Animation 목록.
    static int iSelectedIndex = -1;
    _uint id = 0;

    for (auto& animName : m_AnimationActors[m_wSelected_AnimActorTag]->Get_AnimationNames())
    {
        if (ImGui::Selectable(animName.c_str(), id == iSelectedIndex))
        {
            iSelectedIndex = id;
            // 현재 선택한 애니메이션 이름 저장.
            m_Selected_AnimationTag = animName;

            // 선택될때만 저장.
            m_fDuration = m_AnimationActors[m_wSelected_AnimActorTag]->Get_Duration(m_Selected_AnimationTag);
            m_AnimationActors[m_wSelected_AnimActorTag]->Change_CurrentAnimation(m_Selected_AnimationTag);

            // Animation Tool이 켜져있는 상태로 NotifyTool을 작업할 예정이므로
            // Animation이 변경될때마다? => NotifyTool에 해당 정보를 전달해주어야합니다. NotifyTool이 켜져있다면?
            if (m_IsVisibleNotify)
            {
                m_pAnimNotifyTool->Process_Notify(m_Selected_AnimActorTag, "", m_fDuration);
                // 그리고 Animation이 바뀌면 현재 설정된 Notify 정보를 날려야한다.
                m_pAnimNotifyTool->Clear();
            }
                
        }
    }
    ImGui::EndChild();

    // 2. 선택한 Animation Detail 처리를 위한 기능 추가.
    Render_Animation_Detail();
}


void CAnimationTool::Render_Model_Detail()
{
    ImGui::BeginChild("Right pane", ImVec2(500, 0), true);

    static float fPosition[3] = { 0.f, 180.f, -100.f };
    ImGui::InputFloat3("Position", fPosition);

    static float fRotation[3] = { 0.f, 0.f, 0.f };
    ImGui::InputFloat3("Rotation", fRotation);

    static float fScale[3] = { 1.f, 1.f, 1.f };
    ImGui::InputFloat3("Scale", fScale);

    static float fSpeedPerSec = { 10.f };
    ImGui::InputFloat("Speed", &fSpeedPerSec);

    static float fRotationPerSec = { 90.f };
    ImGui::InputFloat("RotationSpeed", &fRotationPerSec);

    static unsigned int iShaderPath = {};
    static const unsigned int min_val = static_cast<_uint>(SHADER_ANIMPATH::DEFAULT_NORMAL);
    static const unsigned int max_val = static_cast<_uint>(SHADER_ANIMPATH::NORMAL_TEXTURE);
    ImGui::SliderScalar("Shader Path", ImGuiDataType_U32, &iShaderPath, &min_val, &max_val);

    if (ImGui::Button("Create Instance"))
    {
        CAnimationActor::ANIMATION_ACTOR_DESC Desc{};
        Desc.fSpeedPerSec = fSpeedPerSec;
        Desc.fRotationPerSec = XMConvertToRadians(fRotationPerSec);
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
        CAnimationActor* pActor = dynamic_cast<CAnimationActor*>(
            m_pGameInstance->Clone_Prototype(ENUM_CLASS(m_eCurLevel)
            , wstrObjTag, PROTOTYPE::GAMEOBJECT, &Desc));
        ASSERT_CRASH(pActor);


        // 3. 생성한 객체 레이어에 추가
        if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel)
            , TEXT("Layer_Actor"), pActor)))
        {
            MSG_BOX("Animation Actor 추가. 생성 실패");
            return;
        }

        // 4. 생성이 완료되었으면 관리할 수 있게 해야함. 생성할 때 저장.
        m_ActorNames.emplace_back(WstringToString(wstrObjTag));

        Safe_AddRef(pActor);
        m_AnimationActors.emplace(wstrObjTag, pActor);
        

        
    }

    ImGui::EndChild();
}

// 선택한 애니메이션에 대한 디테일한 정보를 가져오기.
void CAnimationTool::Render_Animation_Detail()
{
    // 1. 현재 TrackPosition 저장.
    if (!m_Selected_AnimationTag.empty())
        m_fTrackPosition = *m_AnimationActors[m_wSelected_AnimActorTag]->Get_TrackPositionPtr(m_Selected_AnimationTag);


    // 2. TrackBar 조절 UI 만들기?
    _float minTrackPos = 0.f;
    _float maxTrackPos = m_fDuration;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowPos = ImVec2(0.f, g_iWinSizeY - 100.f); // 아래에 고정?
    ImVec2 windowSize = ImVec2(600.f, 120.f);
    
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);
    
    ImGui::Begin("Animation Detail", nullptr, ImGuiWindowFlags_NoCollapse);

    if (ImGui::SliderFloat("Track Position", &m_fTrackPosition, minTrackPos, maxTrackPos))
    {
        // 설정된 TrackPosition을 전달합니다.
        if (!m_Selected_AnimationTag.empty())
            m_AnimationActors[m_wSelected_AnimActorTag]->Set_TrackPosition(m_fTrackPosition);
    }

    _bool IsChanged = { false };
    
    if (KEYSTATE::DOWN == m_pGameInstance->Get_DIKeyState(DIK_SPACE))
    {
        IsChanged = true;
        m_IsPlayAnimation = !m_IsPlayAnimation;
    }
        

    if (ImGui::Button("Stop"))
    {
        IsChanged = true;
        m_IsPlayAnimation = false;
    }
        

    ImGui::SameLine();
    if (ImGui::Button("Play"))
    {
        IsChanged = true;
        m_IsPlayAnimation = true;
    }

    if (IsChanged)
        m_AnimationActors[m_wSelected_AnimActorTag]->Set_PlayAnimation(m_IsPlayAnimation);

    if (ImGui::Button("Notify Visible"))
        m_IsVisibleNotify = !m_IsVisibleNotify;


    

    ImGui::End();
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

string CAnimationTool::WstringToString(const std::wstring& wstr)
{
    if (wstr.empty())
        return "";

    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (len == 0)
        return "";

    std::string str(len - 1, 0);  // -1로 null terminator 제외
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], len, NULL, NULL);

    return str;
}


HRESULT CAnimationTool::Add_Prototype_AnimModel(_wstring strPrototypeName, MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath)
{
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
        , strPrototypeName
        , CModel::Create(m_pDevice, m_pContext, eType, PreTransformMatrix, pFilePath))))
        return E_FAIL;

    return S_OK;
}


CAnimationTool* CAnimationTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel)
{
    CAnimationTool* pInstance = new CAnimationTool(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eLevel)))
    {
        MSG_BOX("Failed to Create : CAnimationTool");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAnimationTool::Free()
{
    CBase::Free();
    Safe_Release(m_pLoader);
    Safe_Release(m_pAnimNotifyTool);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    for (auto& pair : m_AnimationActors)
        Safe_Release(pair.second);
    m_AnimationActors.clear();

    m_ModelDatPaths.clear();
    
    m_ModelNames.clear();
    m_ActorNames.clear();
    
}

