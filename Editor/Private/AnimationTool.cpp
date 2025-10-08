#include "EditorPch.h"
#include "AnimationTool.h"

#pragma region 기본 함수들
CAnimationTool::CAnimationTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CAnimationTool::Initialize()
{

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
    Render_SelectMode();

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
    const char* typeNames[] = { "SAVE_FBX", "LOAD_DAT", "EDIT_ANIMATION", "END"};
    ImGui::Text("MODE : %s", typeNames[ENUM_CLASS(m_eMode)]);

    ImGui::End();
}

// Mode 지정.
void CAnimationTool::Render_SelectMode()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("SAVE_FBX")) {
            m_eMode = MODE::SAVE_FBX;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("LOAD_DAT")) {
            m_eMode = MODE::LOAD_DAT;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("EDIT_ANIMATION")) {
            m_eMode = MODE::EDIT_ANIMATION;
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
        if (ImGui::BeginTabItem("Save_FBX"))
        {
            if (ImGui::BeginTabBar("Save_FBX", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Save_FBX"))
                {
                    Render_SaveFBX();
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
                    Render_LoadDAT();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Edit_DAT"))
        {
            if (ImGui::BeginTabBar("Edit_DAT", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Edit_DAT"))
                {
                    Render_EditDAT();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

}

void CAnimationTool::Render_SaveFBX()
{
    // 1. .json 파일을 읽어와서? FBX 파일을 DAT화 한다. => 대량

    // 2. 직접 하나 선택해서 FBX 파일을 DAT화 한다. => 일부.
}

void CAnimationTool::Render_LoadDAT()
{

}

void CAnimationTool::Render_EditDAT()
{

}

#pragma endregion



CAnimationTool* CAnimationTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAnimationTool* pInstance = new CAnimationTool(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Animation");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAnimationTool::Free()
{
    CBase::Free();
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}

