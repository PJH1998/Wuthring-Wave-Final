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

    // 2. 기본 Animation Actor용 Prototype 생성?
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
        , TEXT("Prototype_GameObject_AnimationActor")
        , CAnimationActor::Create(m_pDevice, m_pContext))))
        return E_FAIL;

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

    // 1. Mode Selcet
    //Render_SelectMode();

    // 2. 알파값 조절해서 투명하게 만들 수 있음.
    ImGui::Begin(u8"Editor", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar
        | ImGuiWindowFlags_AlwaysVerticalScrollbar);

    _float fEditorAlpha = { 1.f };
    ImGui::Text("Settings");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 WindowColor = style.Colors[ImGuiCol_WindowBg];
    ImGui::SliderFloat("Editor Opacity", &fEditorAlpha, 0.0f, 1.0f);
    const ImVec4 NewColor = ImVec4(WindowColor.x, WindowColor.y, WindowColor.z, fEditorAlpha);
    style.Colors[ImGuiCol_WindowBg] = NewColor;
    ImGui::NewLine();

    // 3. MenuTabBar 구현
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

    _float3 camPos = {};
    //XMStoreFloat3(&camPos, m_pCameraTransformCom->Get_State(STATE::POSITION));
    //ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
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
        if (ImGui::BeginTabItem("Save_Load"))
        {
            if (ImGui::BeginTabBar("Save_Load", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Save_Load"))
                {
                    m_pLoader->Update();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }

       /* if (ImGui::BeginTabItem("Save_FBX"))
        {
            if (ImGui::BeginTabBar("Save_FBX", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Save_FBX"))
                {
                    RenderUI_ConvertFbx();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Load_DAT"))
        {
            if (ImGui::BeginTabBar("Load_DAT", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Load_DAT"))
                {
                    RenderUI_ConvertFbx();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }*/

        if (ImGui::BeginTabItem("Edit_DAT"))
        {
            if (ImGui::BeginTabBar("Edit_DAT", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Edit_DAT"))
                {
                    RenderUI_ViewDat();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
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
    LoadDat();
    
    // 2. 현재 생성된 프로토타입 목록을 보여주기.
    RenderUI_Prototype();
}

void CAnimationTool::RenderUI_EditAnimation()
{

}

void CAnimationTool::LoadDat()
{
    // 1. Load하고 Load된 프로토타입 이름을 저장해두기.
    
}

void CAnimationTool::RenderUI_Prototype()
{
}

#pragma endregion



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
}

