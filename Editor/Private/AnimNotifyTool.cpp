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
void CAnimNotifyTool::Process_Notify(const _string& strAnimName, const _string& strModelDatPath, _float fDuration)
{
    m_strCurrentAnimName = strAnimName;

    // 비어있지 않을 때만 저장할 폴더 경로를 받습니다.
    if (!strModelDatPath.empty())
    {
        m_strCurrentFolderPath = strModelDatPath;
    }
        

    m_fCurrentDuration = fDuration;
}



void CAnimNotifyTool::Clear()
{
    m_SoundNotifyes.clear();
    m_EffectNotifyes.clear();
    m_ColliderNotifyes.clear();
    m_LightNotifyes.clear();
}

void CAnimNotifyTool::RenderUI_EditNotify()
{
    ImGuiIO& io = ImGui::GetIO();

    // 오른쪽 위 위치 계산 (창 크기 300x250 고려)
    ImVec2 vPos = ImVec2(g_iWinSizeX * 0.75f, 0.f); 
    ImGui::SetNextWindowPos(vPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f), ImGuiCond_Once);

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

        if (ImGui::BeginTabItem("Effect"))
        {
            RenderUI_EditEffect();
            m_eType = NOTIFYTYPE::EFFECT;
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Collider"))
        {
            RenderUI_EditCollider();
            m_eType = NOTIFYTYPE::COLLIDER;
            ImGui::EndTabItem();
        }

        // 설정된 모든 정보를 저장 하면서, 설정된 정보도 확인 가능하게.
        if (ImGui::BeginTabItem("Save"))
        {
            RenderUI_SaveNotify();
            ImGui::EndTabItem();
        }

        // 설정된 정보를 불러와서 Notify를 확인하기
        if (ImGui::BeginTabItem("Load"))
        {
            RenderUI_LoadNotify();
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

        // 2. Load된 Sound File을 이용 Notify 설정을 추가한다. 
        if (ImGui::BeginTabItem("Edit"))
        {
            Select_SoundNotify();
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

void CAnimNotifyTool::RenderUI_SaveNotify()
{
    // 1. 툴에서 list에 등록된 Notify 전체를 확인할 수 있어야한다.
    Render_CurrentNotify();
    
    ImGui::Separator();

    // 2. Save를 누르면 현재 등록된 Notify 정보를 확인하고? Json에 기록한다.
    if (ImGui::Button("Save All Notifyes"))
    {
        Save_NotifyToJson();
    }
}

void CAnimNotifyTool::RenderUI_LoadNotify()
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
void CAnimNotifyTool::Select_SoundNotify()
{
    _wstring objTag = {};
    _wstring modelTag = {};

    ImGui::BeginChild("left pane", ImVec2(g_iWinSizeX * 0.25f, 0), true);

    static int iSelectedIndex = -1;
    _uint id = 0;

    // 1. 현재 Sound Tag를 저장.
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

    ImGui::SameLine();
    // 필요한 정보
    // 1. 현재 플레이 중인 애니메이션 정보
    // 2. 현재 애니메이션의 최대 프레임 정보 (TrackPosition 으로 설정할듯?)
    // Process Notify로 이미 받아옴.
    // 해당 정보를 바탕으로 설정 값 조금 추가해서 list에 struct로 추가. 

    if (iSelectedIndex >= 0 && iSelectedIndex < m_SoundTags.size())
        Edit_SoundNotify();

}

void CAnimNotifyTool::Render_CurrentNotify()
{
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    _bool IsDeleted = { false };

    if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Sound List"))
        {
            // 삭제할 index
            _uint iDeleteIndex = {};
            

            // 현재 애니메이션 이름과 총 Duration 값을 맨 위에서 출력
            ImGui::Text("Animation Name : %s", m_strCurrentAnimName.c_str());
            ImGui::Text("Duration : %.2f", m_fCurrentDuration);
            ImGui::Separator();

            // 현재 등록된 list 구조체 정보를 전체 렌더링한다.
            _uint iIndex = { 0 };
            for (auto& SoundNotify : m_SoundNotifyes)
            {

                // 현재 루프의 인덱스를 사용하여 고유한 ID 스택을 만듭니다.
                ImGui::PushID(iIndex);

                ImGui::Text("TrackPosition : %.2f", SoundNotify.fTrackPosition);
                ImGui::Text("Volume : %.2f", SoundNotify.fVolume);
                ImGui::Text("Sound Tag : %s", SoundNotify.strSoundTag.c_str());


                if (ImGui::Button("Delete"))
                {
                    IsDeleted = true;
                    iDeleteIndex = iIndex;
                }
                    
                // ID 스택을 원래대로 되돌립니다.
                ImGui::PopID();

                // 마지막 항목이 아닐 때만 구분선 추가
                if (iIndex < m_SoundNotifyes.size() - 1)
                    ImGui::Separator();

                iIndex++;
            }

            if (IsDeleted)
            {
                auto iterDelete = next(m_SoundNotifyes.begin(), iDeleteIndex);
                m_SoundNotifyes.erase(iterDelete);
            }
                


            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Effect List"))
        {
            // 현재 애니메이션 이름과 총 Duration 값을 맨 위에서 출력
            ImGui::Text("Animation Name : %s", m_strCurrentAnimName.c_str());
            ImGui::Text("Duration : %.2f", m_fCurrentDuration);

            _uint iIndex = { 0 };
            for (auto& EffectNotify : m_EffectNotifyes)
            {
                ImGui::Text("TrackPosition : %.2f", EffectNotify.fTrackPosition);
                ImGui::Text("Effect Tag : %s", EffectNotify.strEffectTag.c_str());

                if (EffectNotify.IsUseBone)
                    ImGui::Text("Bone Name : %s", EffectNotify.strBoneName);
                else
                    ImGui::Text("UnUsed Bone");

                // 마지막 항목이 아닐 때만 구분선 추가
                if (iIndex < m_EffectNotifyes.size() - 1)
                    ImGui::Separator();

                iIndex++;
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Collider List"))
        {
            // 현재 애니메이션 이름과 총 Duration 값을 맨 위에서 출력
            ImGui::Text("Animation Name : %s", m_strCurrentAnimName.c_str());
            ImGui::Text("Duration : %.2f", m_fCurrentDuration);

            // 현재 등록된 list 구조체 정보를 전체 렌더링한다.
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Light List"))
        {
            // 현재 애니메이션 이름과 총 Duration 값을 맨 위에서 출력
            ImGui::Text("Animation Name : %s", m_strCurrentAnimName.c_str());
            ImGui::Text("Duration : %.2f", m_fCurrentDuration);

            // 현재 등록된 list 구조체 정보를 전체 렌더링한다.
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }


}

// 현재 기록된 Notify 정보를 Json 파일로 파싱해서 저장.
void CAnimNotifyTool::Save_NotifyToJson()
{
    // 0. 저장 방식도 방식인데 경로는 어떻게? => AnimationActor 생성할 때 FilePath를 미리 저장할까?
    // LoadDat할때 해당 모델의 .dat 폴더 경로를 저장해놓자.



    // 1. Sound

    // 2. Effect

    // 3. Collider

    // 4. Light
}

void CAnimNotifyTool::Load_NotifyFromJson()
{

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

void CAnimNotifyTool::Edit_SoundNotify()
{
    ImGui::BeginChild("Right pane", ImVec2(g_iWinSizeX * 0.25f, 0), true);

    static float fTrackPosition = {};
    ImGui::InputFloat("TrackPosition", &fTrackPosition);

    // 값 넘으면 Max 값으로 자동 설정.
    fTrackPosition = clamp(fTrackPosition, 0.f, m_fCurrentDuration);
   

    static float fVolume = {};
    ImGui::SliderFloat("Volume", &fVolume, 0.f, 1.f);

    // 1. 구조체로 리스트에 저장하기.
    if (ImGui::Button("Add SoundNotify"))
    {
        SOUNDNOTIFY Desc{};
        Desc.fTrackPosition = fTrackPosition;
        Desc.fVolume = fVolume;
        Desc.strSoundTag = m_CurrentSoundTag;
        m_SoundNotifyes.emplace_back(Desc);
    }

    ImGui::EndChild();
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
    Clear();
}