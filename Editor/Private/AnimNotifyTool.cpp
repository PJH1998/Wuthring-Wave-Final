#include "EditorPch.h"
#include "AnimNotifyTool.h"
#include "AnimationActor.h"

#pragma region 기본 함수들
CAnimNotifyTool::CAnimNotifyTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext { pContext }
    , m_pGameInstance { CGameInstance::GetInstance()}
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}
HRESULT CAnimNotifyTool::Initialize(LEVEL eLevel)
{
    m_eCurLevel = eLevel;

    if (FAILED(Ready_Sound()))
    {
        CRASH("Failed Ready_Sound");
        return E_FAIL;
    }

    // Test
    //m_pGameInstance->Play_Sound(TEXT("Augusta_Attack01"), 0, 0.3f);

	return S_OK;
}

void CAnimNotifyTool::Update()
{

}

void CAnimNotifyTool::Render()
{
    RenderUI_EditNotify();
}

// Notify 등록 시 무조건적으로 필요한 정보.
void CAnimNotifyTool::Process_Notify(CAnimationActor* pActor, const _string& strAnimName, _float fDuration)
{
    ASSERT_CRASH(pActor);
    m_pCurrentActor = pActor;
    m_CurrentAnimName = strAnimName;
    m_fCurrentDuration = fDuration;
}


void CAnimNotifyTool::RenderUI_EditNotify()
{
    ImGuiIO& io = ImGui::GetIO();

    // 오른쪽 위 위치 계산 (창 크기 300x250 고려)
    ImVec2 vPos = ImVec2(g_iWinSizeX * 0.75f, 0.f); 
    ImGui::SetNextWindowPos(vPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(g_iWinSizeX * 0.25f, g_iWinSizeY), ImGuiCond_Once);

    ImGui::Begin("Edit_Notify");
    

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Sound"))
        {
            RenderUI_EditSound();
            m_eType = NOTIFYTYPE::SOUND;
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    


    ImGui::End();
}


void CAnimNotifyTool::RenderUI_EditSound()
{
    // 1. 사운드 파일 목록 FileDialog로 선택?
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Load"))
        {
            Load_SoundFiles();
            ImGui::EndTabItem();
        }

        // 2. Load된 Sound File을 이용 Notify를 추가한다. 
        if (ImGui::BeginTabItem("Edit"))
        {
            Edit_SoundNotify();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    
    

}

void CAnimNotifyTool::RenderUI_EditEffect()
{
}

void CAnimNotifyTool::RenderUI_EditCollider()
{
}

void CAnimNotifyTool::Load_SoundFiles()
{

    if (ImGui::Button("Load Sound File"))
    {
        IGFD::FileDialogConfig config;
        config.path = "../../Client/Bin/Resource/";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

        ImGuiFileDialog::Instance()->OpenDialog("Load Sound File", "Import Sound", ".wav", config);
    }

    ImGui::SameLine();

    if (ImGui::Button("Load Sound Folder"))
    {
        IGFD::FileDialogConfig config;
        config.path = "../../Client/Bin/Resource/";
        config.flags = ImGuiFileDialogFlags_Modal;

        ImGuiFileDialog::Instance()->OpenDialog("Load Sound Folder", "Import Sound Foloder", nullptr, config);
    }

    ImVec2 vMinSize = ImVec2(600, 400);  // 최소 크기
    ImVec2 vMaxSize = ImVec2(800, 400); // 최대 크기

    if (ImGuiFileDialog::Instance()->Display(
        "Load Sound File", ImGuiWindowFlags_NoCollapse
        , vMinSize
        , vMaxSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _string strSoundPath = ImGuiFileDialog::Instance()->GetCurrentFileName();
            Load_SoundsFromFile(strFilePath, strSoundPath);
        }
        ImGuiFileDialog::Instance()->Close();
    }


    if (ImGuiFileDialog::Instance()->Display(
        "Load Sound Folder", ImGuiWindowFlags_NoCollapse
        , vMinSize
        , vMaxSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFolderPath = ImGuiFileDialog::Instance()->GetCurrentPath();
            Load_AllSoundsFromFolder(strFolderPath);
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

// 목록 확인 및 Sound 파일 선택.
void CAnimNotifyTool::Edit_SoundNotify()
{
    _wstring objTag = {};
    _wstring modelTag = {};

    ImGui::BeginChild("left pane", ImVec2(500, 0), true);

    static int iSelectedIndex = -1;
    _uint id = 0;

    for (auto& pair : m_SoundTags)
    {
        if (ImGui::Selectable(pair.first.c_str(), id == iSelectedIndex))
        {
            iSelectedIndex = id;
            // Sound Tag만 저장하자.
            m_CurrentSoundTag = pair.first;
        }
    }
    ImGui::EndChild();
}

void CAnimNotifyTool::Load_SoundsFromFile(const _string& strFilePath, const _string& strSoundPath)
{
    // .wav 잘라내기.
    size_t last_dot_pos = strSoundPath.find_last_of('.');
    if (last_dot_pos != std::string::npos) {
        // 0번째 위치부터 '.' 위치까지 문자열을 잘라냅니다.
        _string strSoundTag = strSoundPath.substr(0, last_dot_pos);
        _wstring wStrSoundTag = StringToWstring(strSoundTag);

        // 1. Sound Load
        m_pGameInstance->Load_Sound(wStrSoundTag, strFilePath.c_str());

        // 2. Sound 이름 관리
        m_SoundTags.emplace(strSoundTag, wStrSoundTag);
    }
    else
    {
        MSG_BOX("경로 잘못됨");
        return;
    }
}

void CAnimNotifyTool::Load_AllSoundsFromFolder(const _string& strFolderPath)
{

    for (const auto& entry : filesystem::directory_iterator(strFolderPath))
    {
        if (entry.is_regular_file())
        {
            _string filePath = entry.path().string();
            _string fileName = entry.path().filename().string();
            _string extension = entry.path().extension().string();

            // .wav 파일만 처리
            if (extension == ".wav" || extension == ".WAV")
            {
                _string soundTag = entry.path().stem().string(); // 확장자 제외한 파일명
                _wstring wSoundTag = StringToWstring(soundTag);

                m_pGameInstance->Load_Sound(wSoundTag, filePath.c_str());
                m_SoundTags.emplace(soundTag, wSoundTag);
            }
        }
    }
}


#pragma endregion


HRESULT CAnimNotifyTool::Ready_Sound()
{
    //m_pGameInstance->Load_Sound(TEXT("Augusta_Attack01"), "../../Client/Bin/Resource/Player/Augusta/Sound/Attack/Augusta_Attack01_01.wav");
    
    return S_OK;
}

wstring CAnimNotifyTool::StringToWstring(const std::string& str)
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

string CAnimNotifyTool::WstringToString(const std::wstring& wstr)
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

CAnimNotifyTool* CAnimNotifyTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel)
{
    CAnimNotifyTool* pInstance = new CAnimNotifyTool(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eLevel)))
    {
        MSG_BOX("Failed to Create : CAnimNotifyTool");
        Safe_Release(pInstance);
    }

    return pInstance;
}
void CAnimNotifyTool::Free()
{
    CBase::Free();
    Safe_Release(m_pGameInstance);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    
}