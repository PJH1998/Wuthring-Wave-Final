#include "EditorPch.h"
#include "AnimationTool.h"
#include "ModelLoader.h"
#include "AnimationActor.h"
#include "AnimNotifyTool.h"


#pragma region 湲곕낯 ?⑥닔??
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

    // 1. FBX ?뚯씪??Dat???댁＜??Loader ?앹꽦.
    m_pLoader = CModelLoader::Create();

    // 2. Animation Notify瑜??깅줉 諛?愿由ы븯???대옒??
    m_pAnimNotifyTool = CAnimNotifyTool::Create(m_pDevice, m_pContext, m_eCurLevel);
   

    return S_OK;
}

void CAnimationTool::Render()
{
    Render_Editor();
    
    // Animation Notify媛 Visible ?곹깭?쇰㈃?
    if (m_IsVisibleNotify)
    {
        // ?????紐⑤뜽 Tag濡???ν븷 ??Folder留???ν븷源?
        _string strModelDirPath = m_ModelDirPaths[m_wSelected_PrototypeModelTag];
        ASSERT_CRASH(m_pAnimNotifyTool);
        m_pAnimNotifyTool->Process_Notify(m_AnimationActors[m_wSelected_AnimActorTag], m_Selected_AnimationTag, strModelDirPath, m_fDuration);
        m_pAnimNotifyTool->Render();
    }
        
}

void CAnimationTool::Render_Editor()
{
    // 0. Debug Render => ?꾩옱 ?곹깭瑜?異쒕젰.
    Render_DebugWindow();

    // 1. ?뚰뙆媛?議곗젅?댁꽌 ?щ챸?섍쾶 留뚮뱾 ???덉쓬.
    ImGui::Begin(u8"Editor", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar
        | ImGuiWindowFlags_AlwaysVerticalScrollbar);

    
    ImGui::Text("Settings");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 WindowColor = style.Colors[ImGuiCol_WindowBg];
    ImGui::SliderFloat("Editor Opacity", &m_fEditorAlpha, 0.0f, 1.0f);
    const ImVec4 NewColor = ImVec4(WindowColor.x, WindowColor.y, WindowColor.z, m_fEditorAlpha);
    style.Colors[ImGuiCol_WindowBg] = NewColor;
    ImGui::NewLine();

    // 2. MenuTabBar 援ы쁽
    Render_Menu();

    ImGui::End();
}

void CAnimationTool::Render_DebugWindow()
{
    ImGuiIO& io = ImGui::GetIO();

    ImVec2 windowPos = ImVec2(300.f, g_iWinSizeY - 300.f);
    ImVec2 windowSize = ImVec2(300.f, 300.f);

    // Cond_Once: 理쒖큹 ??踰덈쭔 ?꾩튂 ?곸슜 ???댄썑 ?쒕옒洹?媛??
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);

    // NoCollapse留??좎?, ?대룞 媛?ν븯寃?
    ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoCollapse);

    _float4 camPos = {};
    camPos = *m_pGameInstance->Get_CamPos();
    ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);


    // ?꾩옱 ?좏깮???뚯씪 ????쒖떆
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
    // 1. 吏곸젒 ?섎굹 ?좏깮?댁꽌 FBX ?뚯씪??DAT???쒕떎. => ?쇰?.
    m_pLoader->Update();
}

void CAnimationTool::RenderUI_CreateActor()
{
    // 1. ?좏깮??Dat ?뚯씪??Load?섍린. => Prototype ?앹꽦.
    // ?곗꽑. GameObject瑜??덈줈 留뚮뱾怨?Prototype ?깅줉.
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("Prototype", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Model"))
        {
            // 2. ?꾩옱 ?앹꽦???꾨줈?좏???紐⑸줉??蹂댁뿬二쇨린.
            RenderUI_ModelPrototype();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    
}

//?좏깮??媛앹껜???좊땲硫붿씠???꾩껜 紐⑸줉???뺤씤?섍퀬 ?좊땲硫붿씠?섏뿉 ????묒뾽??吏꾪뻾.
void CAnimationTool::RenderUI_EditAnimation()
{
     // 1. ?앹꽦??Prototype 紐⑸줉?ㅼ쓣 ?뺤씤?섍린.
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
            // ?좏깮 ?뺣낫???
            m_Selected_AnimActorTag = actorName;
            m_wSelected_AnimActorTag = StringToWString(actorName);
        }
    }
    ImGui::EndChild();

    // ?좊땲硫붿씠??紐⑸줉李쎄퉴吏??媛숈? ?먯떇 媛쒖껜濡??앹꽦.
    ImGui::SameLine();
    if (iSelectedIndex >= 0 && iSelectedIndex < m_ActorNames.size())
        RenderUI_AnimationList();
}

void CAnimationTool::LoadDat()
{
    // 1. Load?섍퀬 Load???꾨줈?좏????대쫫????ν빐?먭린.
    _wstring wStrModelName = {};

    _string strModelName = "Prototype_Component_Model_";

    _string strModelPath = {};
    string basePathString = {};

    // 2. ImGui?먯꽌 ?뚯씪???ㅽ뵂?댁꽌 ?대떦 ?뚯씪???댁슜?댁꽌 Prototype Model ?숈쟻?쇰줈 ?앹꽦
    CModel* pModelCom = { nullptr };


    if (ImGui::Button("Load DAT File"))
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/Model/";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

        basePathString = config.path;

        ImGuiFileDialog::Instance()->OpenDialog("DAT File Load", "Import File", ".dat", config);
    }
  
    ImVec2 vMinSize = ImVec2(600, 400);  // 理쒖냼 ?ш린
    ImVec2 vMaxSize = ImVec2(800, 400); // 理쒕? ?ш린

    if (ImGuiFileDialog::Instance()->Display(
        "DAT File Load", ImGuiWindowFlags_NoCollapse
        , vMinSize
        , vMaxSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
            strModelPath = ImGuiFileDialog::Instance()->GetCurrentFileName();

            // Dir ?곷? 寃쎈줈濡????


            // .dat ?섎씪?닿린.
            size_t lastDotPos = strModelPath.find_last_of('.');
            if (lastDotPos != string::npos) {
                // 0踰덉㎏ ?꾩튂遺??'.' ?꾩튂源뚯? 臾몄옄?댁쓣 ?섎씪?낅땲??
                strModelName += strModelPath.substr(0, lastDotPos);
                
            }
            else
            {
                MSG_BOX("寃쎈줈 ?섎せ??");
                return;
            }

            _matrix		PreTransformMatrix = XMMatrixIdentity();
            _float fSize = 0.01f;
            PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(XM_PI));

            wStrModelName = StringToWString(strModelName);

            // Model Prototype ?앹꽦.
            HRESULT hr = Add_Prototype_AnimModel(wStrModelName, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str());
            if (FAILED(hr))
            {
                MSG_BOX("寃쎈줈 ?섎せ?섏뿀嫄곕굹, 以묐났 ?앹꽦.");
                return;
            }

            // 3. ?앹꽦???꾨즺?섏뿀?쇰㈃ ?꾩슂???뺣낫?ㅼ쓣 ???
            
            size_t lastSlashPos = strFilePath.find_last_of("/\\");
            string directoryPath = "";
            if (lastDotPos != string::npos)
            {
                directoryPath = strFilePath.substr(0, lastSlashPos);
                m_ModelDirPaths.emplace(wStrModelName, directoryPath);
            }
            
            m_ModelNames.emplace_back(strModelName);
        }
        ImGuiFileDialog::Instance()->Close();
    }

}

void CAnimationTool::RenderUI_ModelPrototype()
{
    // 1. ?앹꽦??Prototype 紐⑸줉?ㅼ쓣 ?뺤씤?섍린.
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
            m_wSelected_PrototypeModelTag = StringToWString(modelName);
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

    // 1. Animation 紐⑸줉.
    static int iSelectedIndex = -1;
    _uint id = 0;

#ifdef _DEBUG
    for (auto& animName : m_AnimationActors[m_wSelected_AnimActorTag]->Get_AnimationNames())
    {
        if (ImGui::Selectable(animName.c_str(), id == iSelectedIndex))
        {
            iSelectedIndex = id;
            // ?꾩옱 ?좏깮???좊땲硫붿씠???대쫫 ???
            m_Selected_AnimationTag = animName;

            // ?좏깮?좊븣留????
            m_fDuration = m_AnimationActors[m_wSelected_AnimActorTag]->Get_Duration(m_Selected_AnimationTag);
            m_AnimationActors[m_wSelected_AnimActorTag]->Change_CurrentAnimation(m_Selected_AnimationTag);

            // Animation Tool??耳쒖졇?덈뒗 ?곹깭濡?NotifyTool???묒뾽???덉젙?대?濡?
            // Animation??蹂寃쎈맆?뚮쭏?? => NotifyTool???대떦 ?뺣낫瑜??꾨떖?댁＜?댁빞?⑸땲?? NotifyTool??耳쒖졇?덈떎硫?
            if (m_IsVisibleNotify)
            {
                m_pAnimNotifyTool->Process_Notify(m_AnimationActors[m_wSelected_AnimActorTag], m_Selected_AnimActorTag, "", m_fDuration);
                // 洹몃━怨?Animation??諛붾뚮㈃ ?꾩옱 ?ㅼ젙??Notify ?뺣낫瑜??좊젮?쇳븳??
                m_pAnimNotifyTool->Clear();
            }
                
        }
    }
    ImGui::EndChild();
#endif 

    // 2. ?좏깮??Animation Detail 泥섎━瑜??꾪븳 湲곕뒫 異붽?.
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
        Desc.strShaderTag = TEXT("Prototype_Component_Shader_VtxAnimMesh"); // ?쇰떒 ?섎뱶肄붾뵫..
        Desc.strComputeShaderTag = TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"); // ?쇰떒 ?섎뱶肄붾뵫..
        Desc.iShaderPath = iShaderPath;
        memcpy(&Desc.vPostion, fPosition, sizeof(_float3));
        memcpy(&Desc.vRotation, fRotation, sizeof(_float3));
        memcpy(&Desc.vScale, fScale, sizeof(_float3));
        Desc.eLevel = m_eCurLevel;

        _wstring wstrObjTag = TEXT("Prototype_GameObject_Actor_");
        

        // 留덉?留?紐⑤뜽 ?대쫫留??섎씪?닿린.
        size_t last_dot_pos = m_wSelected_PrototypeModelTag.find_last_of('_');
        if (last_dot_pos != std::string::npos) {
            wstrObjTag += m_wSelected_PrototypeModelTag.substr(last_dot_pos + 1, m_wSelected_PrototypeModelTag.size());
        }
        else
        {
            MSG_BOX("寃쎈줈 ?섎せ??");
            return;
        }


        // 1. Animation Actor??Prototype ?앹꽦
        if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
            , wstrObjTag
            , CAnimationActor::Create(m_pDevice, m_pContext))))
        {
            MSG_BOX("Animation Actor Prototype ?앹꽦 ?ㅽ뙣");
            return;
        }

        // 2. ?앹꽦??Prototype Clone
        CAnimationActor* pActor = dynamic_cast<CAnimationActor*>(
            m_pGameInstance->Clone_Prototype(ENUM_CLASS(m_eCurLevel)
            , wstrObjTag, PROTOTYPE::GAMEOBJECT, &Desc));
        ASSERT_CRASH(pActor);


        // 3. ?앹꽦??媛앹껜 ?덉씠?댁뿉 異붽?
        if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel)
            , TEXT("Layer_Actor"), pActor)))
        {
            MSG_BOX("Animation Actor 異붽?. ?앹꽦 ?ㅽ뙣");
            return;
        }

        // 4. ?앹꽦???꾨즺?섏뿀?쇰㈃ 愿由ы븷 ???덇쾶 ?댁빞?? ?앹꽦???????
        m_ActorNames.emplace_back(WStringToString(wstrObjTag));

        Safe_AddRef(pActor);
        m_AnimationActors.emplace(wstrObjTag, pActor);
        

        
    }

    ImGui::EndChild();
}

// ?좏깮???좊땲硫붿씠?섏뿉 ????뷀뀒?쇳븳 ?뺣낫瑜?媛?몄삤湲?
void CAnimationTool::Render_Animation_Detail()
{
#ifdef _DEBUG
    // 1. ?꾩옱 TrackPosition ???
    if (!m_Selected_AnimationTag.empty())
        m_fTrackPosition = *m_AnimationActors[m_wSelected_AnimActorTag]->Get_TrackPositionPtr(m_Selected_AnimationTag);
#endif

    // 2. TrackBar 議곗젅 UI 留뚮뱾湲?
    _float minTrackPos = 0.f;
    _float maxTrackPos = m_fDuration;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowPos = ImVec2(0.f, g_iWinSizeY - 100.f); // ?꾨옒??怨좎젙?
    ImVec2 windowSize = ImVec2(600.f, 120.f);
    
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);
    
    ImGui::Begin("Animation Detail", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Animation Name : %s", m_Selected_AnimationTag.c_str());

    if (ImGui::SliderFloat("Track Position", &m_fTrackPosition, minTrackPos, maxTrackPos))
    {
#ifdef _DEBUG
        // ?ㅼ젙??TrackPosition???꾨떖?⑸땲??
        if (!m_Selected_AnimationTag.empty())
            m_AnimationActors[m_wSelected_AnimActorTag]->Set_TrackPosition(m_fTrackPosition);
#endif
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

#ifdef _DEBUG
    if (IsChanged)
        m_AnimationActors[m_wSelected_AnimActorTag]->Set_PlayAnimation(m_IsPlayAnimation);
#endif

    if (ImGui::Button("Notify Visible"))
        m_IsVisibleNotify = !m_IsVisibleNotify;


    

    ImGui::End();
}



#pragma endregion



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

    m_ModelDirPaths.clear();
    
    m_ModelNames.clear();
    m_ActorNames.clear();
    
}

