#include "EditorPch.h"
#include "AnimNotifyTool.h"
#include "AnimationActor.h"
#include "SoundNotify.h"
#include "ColliderNotify.h"


#pragma region 
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

// Notify ?깅줉 ??臾댁“嫄댁쟻?쇰줈 ?꾩슂???뺣낫.
void CAnimNotifyTool::Process_Notify(CAnimationActor* pActor, const _string& strAnimName, const _string& strModelDirPath, _float fDuration)
{
    ASSERT_CRASH(pActor);
    m_pCurrentActor = pActor;
    
    m_strCurrentAnimName = strAnimName;

    // 鍮꾩뼱?덉? ?딆쓣 ?뚮쭔 ??ν븷 ?대뜑 寃쎈줈瑜?諛쏆뒿?덈떎.
    if (!strModelDirPath.empty())
    {
        m_strCurrentFolderPath = strModelDirPath;
        m_strCurrentFolderPath += string("\\Notify");
    }

    m_fCurrentDuration = fDuration;
}



void CAnimNotifyTool::Clear()
{
    for (auto& notify : m_AnimNotifies)
        Safe_Release(notify);

    m_AnimNotifies.clear();

    for (auto& notify : m_SoundNotifies)
        Safe_Release(notify);
    
    m_SoundNotifies.clear();

    for (auto& notify : m_ColliderNotifies)
        Safe_Release(notify);
    
    m_ColliderNotifies.clear();


    
    //m_SoundNotifies.clear();
    //m_ColliderNotifies.clear();
    //m_EffectNotifies.clear();
    //m_LightNotifies.clear();
}

void CAnimNotifyTool::RenderUI_EditNotify()
{
    ImGuiIO& io = ImGui::GetIO();

    // ?ㅻⅨ履????꾩튂 怨꾩궛 (李??ш린 300x250 怨좊젮)
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

        // ?ㅼ젙??紐⑤뱺 ?뺣낫瑜?????섎㈃?? ?ㅼ젙???뺣낫???뺤씤 媛?ν븯寃?
        if (ImGui::BeginTabItem("Save"))
        {
            RenderUI_SaveNotify();
            ImGui::EndTabItem();
        }

        // ?ㅼ젙???뺣낫瑜?遺덈윭???Notify瑜??뺤씤?섍린
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
    // 1. ?ъ슫???뚯씪 紐⑸줉 FileDialog濡??좏깮?
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Load"))
        {
            Load_SoundFiles();
            ImGui::EndTabItem();
        }

        // 2. Load??Sound File???댁슜 Notify ?ㅼ젙??異붽??쒕떎. 
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
    // 1. Collider Tag, TrackPosition, Active;

    static float fTrackPosition = {};
    ImGui::SetNextItemWidth(120.f); 
    ImGui::InputFloat("TrackPosition", &fTrackPosition);

    static string strColliderTag;
    static bool IsActive = { false };

    if (strColliderTag.empty())
    strColliderTag.resize(256);

    ImGui::SetNextItemWidth(120.f);
    ImGui::InputText("Tag", &strColliderTag[0], strColliderTag.capacity());
    
    ImGui::Checkbox("Active", & IsActive);


    // 2. ?ㅼ젙???뺣낫瑜?Notify?ㅼ젙.
    if (ImGui::Button("Apply Collider Notify"))
    {
        json ColliderJson;
        ColliderJson["TrackPosition"] = fTrackPosition;
        ColliderJson["ColliderTag"] = strColliderTag.c_str(); // ?낅젰諛쏆? ?ㅽ듃留곸쑝濡?
        ColliderJson["IsActive"] = IsActive;
        CColliderNotify* pColliderNotify = CColliderNotify::From_Json(ColliderJson);
        m_ColliderNotifies.emplace_back(pColliderNotify);
    }


}

void CAnimNotifyTool::RenderUI_SaveNotify()
{
    // 0. 怨듯넻 ?ы빆.
    // ?꾩옱 ?좊땲硫붿씠???대쫫怨?珥?Duration 媛믪쓣 留??꾩뿉??異쒕젰
    ImGui::Text("Animation Name : %s", m_strCurrentAnimName.c_str());
    ImGui::Text("Duration : %.2f", m_fCurrentDuration);

    // 1. ?댁뿉??list???깅줉??Notify ?꾩껜瑜??뺤씤?????덉뼱?쇳븳??
#ifdef _DEBUG
    Render_CurrentNotify();
#endif
    ImGui::Separator();

    // 2. Save瑜??꾨Ⅴ硫??꾩옱 ?깅줉??Notify ?뺣낫瑜??뺤씤?섍퀬? Json??湲곕줉?쒕떎.
    Save_Notify();
    
}

void CAnimNotifyTool::RenderUI_LoadNotify()
{
    // 0. 怨듯넻 ?ы빆.
    // ?꾩옱 ?좊땲硫붿씠???대쫫怨?珥?Duration 媛믪쓣 留??꾩뿉??異쒕젰
    ImGui::Text("Animation Name : %s", m_strCurrentAnimName.c_str());
    ImGui::Text("Duration : %.2f", m_fCurrentDuration);

    if (m_IsLoadNotify)
        ImGui::Text("All Animation Notify Loaded");


#ifdef _DEBUG
    // 1. ?댁뿉??list???깅줉??Notify ?꾩껜瑜??뺤씤?????덉뼱?쇳븳??
    if (m_IsLoadNotify) // Load?ㅻ? ?뚮???寃쎌슦?먮쭔 蹂댁뿬以띾땲??
        Render_CurrentNotify();
#endif

    ImGui::Separator();

    // 2. Load瑜??꾨Ⅴ硫?Json?쇰줈遺??Notfiy?뺣낫瑜??쎌뼱???List??梨꾩썙?먭퀬
    Load_NotifyFromFile();

    // 3. Load?????쒕쾲???대뜑瑜??ㅼ씫?댁???Load?섍퀬 ?뺤씤.
#ifdef _DEBUG
    if (ImGui::Button("Load All Animation Notifies"))
    {
        m_IsLoadNotify = true;
        ASSERT_CRASH(m_pCurrentActor);
        m_pCurrentActor->Register_AllNotifies(m_strCurrentFolderPath);
    }
#endif
}

void CAnimNotifyTool::Load_SoundFiles()
{

    if (ImGui::Button("Load Sound File"))
    {
        IGFD::FileDialogConfig config;
        config.path = "../../Client/Bin/Resource/Sound/";
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

    ImVec2 vMinSize = ImVec2(600, 400);  // 理쒖냼 ?ш린
    ImVec2 vMaxSize = ImVec2(800, 400); // 理쒕? ?ш린

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

// 紐⑸줉 ?뺤씤 諛?Sound ?뚯씪 ?좏깮.
void CAnimNotifyTool::Select_SoundNotify()
{
    _wstring objTag = {};
    _wstring modelTag = {};

    ImGui::BeginChild("left pane", ImVec2(g_iWinSizeX * 0.25f, 0), true);

    static int iSelectedIndex = -1;
    _uint id = 0;

    // 1. ?꾩옱 Sound Tag瑜????
    for (auto& pair : m_SoundTags)
    {
        if (ImGui::Selectable(pair.first.c_str(), id == iSelectedIndex))
        {
            iSelectedIndex = id;
            // Sound Tag留???ν븯??
            m_CurrentSoundTag = pair.first;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    // ?꾩슂???뺣낫
    // 1. ?꾩옱 ?뚮젅??以묒씤 ?좊땲硫붿씠???뺣낫
    // 2. ?꾩옱 ?좊땲硫붿씠?섏쓽 理쒕? ?꾨젅???뺣낫 (TrackPosition ?쇰줈 ?ㅼ젙?좊벏?)
    // Process Notify濡??대? 諛쏆븘??
    // ?대떦 ?뺣낫瑜?諛뷀깢?쇰줈 ?ㅼ젙 媛?議곌툑 異붽??댁꽌 list??struct濡?異붽?. 

    if (iSelectedIndex >= 0 && iSelectedIndex < m_SoundTags.size())
        Edit_SoundNotify();

}

#ifdef _DEBUG
void CAnimNotifyTool::Render_CurrentNotify()
{
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    _bool IsDeleted = { false };

    if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Sound List"))
        {
            // ??젣??index
            _uint iDeleteIndex = {};

            // ?꾩옱 ?깅줉??list 援ъ“泥??뺣낫瑜??꾩껜 ?뚮뜑留곹븳??
            _uint iIndex = { 0 };
            for (auto& SoundNotify : m_SoundNotifies)
            {
                // ?꾩옱 猷⑦봽???몃뜳?ㅻ? ?ъ슜?섏뿬 怨좎쑀??ID ?ㅽ깮??留뚮벊?덈떎.
                ImGui::PushID(iIndex);

                SoundNotify->ImGui_Print();

                if (ImGui::Button("Delete"))
                {
                    IsDeleted = true;
                    iDeleteIndex = iIndex;
                }

                // ID ?ㅽ깮???먮옒?濡??섎룎由쎈땲??
                ImGui::PopID();

                // 留덉?留???ぉ???꾨땺 ?뚮쭔 援щ텇??異붽?
                if (iIndex < m_SoundNotifies.size() - 1)
                    ImGui::Separator();

                iIndex++;
            }

            if (IsDeleted)
            {
                auto iterDelete = next(m_SoundNotifies.begin(), iDeleteIndex);
                Safe_Release(*iterDelete);
                m_SoundNotifies.erase(iterDelete);
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Effect List"))
        {
            //// ?꾩옱 ?좊땲硫붿씠???대쫫怨?珥?Duration 媛믪쓣 留??꾩뿉??異쒕젰
            //ImGui::Text("Animation Name : %s", m_strCurrentAnimName.c_str());
            //ImGui::Text("Duration : %.2f", m_fCurrentDuration);


            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Collider List"))
        {
            // ??젣??index
            _uint iDeleteIndex = {};

            // ?꾩옱 ?깅줉??list 援ъ“泥??뺣낫瑜??꾩껜 ?뚮뜑留곹븳??
            _uint iIndex = { 0 };
            for (auto& ColliderNotify : m_ColliderNotifies)
            {
                // ?꾩옱 猷⑦봽???몃뜳?ㅻ? ?ъ슜?섏뿬 怨좎쑀??ID ?ㅽ깮??留뚮벊?덈떎.
                // -> ImGui??String ID媛 ?숈씪??媛앹껜媛 媛숈? ?붾㈃???덉쑝硫??ㅻ쪟媛 ?덉쓬.
                ImGui::PushID(iIndex);

                ColliderNotify->ImGui_Print();

                if (ImGui::Button("Delete"))
                {
                    IsDeleted = true;
                    iDeleteIndex = iIndex;
                }

                // ID ?ㅽ깮???먮옒?濡??섎룎由쎈땲??
                ImGui::PopID();

                // 留덉?留???ぉ???꾨땺 ?뚮쭔 援щ텇??異붽?
                if (iIndex < m_ColliderNotifies.size() - 1)
                    ImGui::Separator();

                iIndex++;
            }

            if (IsDeleted)
            {
                auto iterDelete = next(m_SoundNotifies.begin(), iDeleteIndex);
                Safe_Release(*iterDelete);
                m_SoundNotifies.erase(iterDelete);
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Light List"))
        {
            // ?꾩옱 ?좊땲硫붿씠???대쫫怨?珥?Duration 媛믪쓣 留??꾩뿉??異쒕젰
            /*ImGui::Text("Animation Name : %s", m_strCurrentAnimName.c_str());
            ImGui::Text("Duration : %.2f", m_fCurrentDuration);*/

            // ?꾩옱 ?깅줉??list 援ъ“泥??뺣낫瑜??꾩껜 ?뚮뜑留곹븳??
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }


}
#endif // _DEBUG



// ?꾩옱 湲곕줉??Notify ?뺣낫瑜?Json ?뚯씪濡??뚯떛?댁꽌 ???
void CAnimNotifyTool::Save_Notify()
{
    // 0. ???諛⑹떇??諛⑹떇?몃뜲 寃쎈줈???대뼸寃? => AnimationActor ?앹꽦????FilePath瑜?誘몃━ ??ν븷源?
    // LoadDat?좊븣 ?대떦 紐⑤뜽??.dat ?대뜑 寃쎈줈瑜???ν빐?볦옄.

    if (ImGui::Button("Save All Notifyes"))
    {
        IGFD::FileDialogConfig config;
        //config.path = "../../Client/Bin/Resource/"; // ?ш린???ㅼ뼱媛?쇳븿.
        config.path = m_strCurrentFolderPath;
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

        ImGuiFileDialog::Instance()->OpenDialog("Save Notify", "Export File", ".json", config);
    }

    ImVec2 vMinSize = ImVec2(600, 400);  // 理쒖냼 ?ш린
    ImVec2 vMaxSize = ImVec2(800, 400); // 理쒕? ?ш린

    if (ImGuiFileDialog::Instance()->Display("Save Notify"
        , ImGuiWindowFlags_NoCollapse
        , vMinSize
        , vMaxSize
    )) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

            Save_NotifyToJson(strFilePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

void CAnimNotifyTool::Load_NotifyFromFile()
{
    // 1. 踰꾪듉???뚮윭???뚯씪濡쒕???Json ?곗씠?곕? ?뚯떛?쒕떎.
    if (ImGui::Button("Load Notifyes"))
    {
        IGFD::FileDialogConfig config;
        config.path = m_strCurrentFolderPath;
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

        ImGuiFileDialog::Instance()->OpenDialog("Load Notify", "Import File", ".json", config);
    }

    ImVec2 vMinSize = ImVec2(600, 400);  // 理쒖냼 ?ш린
    ImVec2 vMaxSize = ImVec2(800, 400); // 理쒕? ?ш린
    _string strFileName = {};
    _string strFilePath = {};
    _string strFolderPath = {};

    // 2. ?뚯떛???곗씠?곕? 諛뷀깢?쇰줈 list??媛믪쓣 梨꾩썙以띾땲??
    if (ImGuiFileDialog::Instance()->Display("Load Notify"
        , ImGuiWindowFlags_NoCollapse
        , vMinSize
        , vMaxSize
    )) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

            size_t lastSlashPos = strFilePath.find_last_of("\\");
            if (lastSlashPos != string::npos) {
                // 0踰덉㎏ ?꾩튂遺??'.' ?꾩튂源뚯? 臾몄옄?댁쓣 ?섎씪?낅땲??
                strFolderPath += strFilePath.substr(0, lastSlashPos);

            }
            Load_NotifyFromJson(strFilePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }    
}


void CAnimNotifyTool::Load_SoundsFromFile(const _string& strFilePath, const _string& strSoundPath)
{
    // .wav ?섎씪?닿린.
    size_t last_dot_pos = strSoundPath.find_last_of('.');
    if (last_dot_pos != std::string::npos) {
        // 0踰덉㎏ ?꾩튂遺??'.' ?꾩튂源뚯? 臾몄옄?댁쓣 ?섎씪?낅땲??
        _string strSoundTag = strSoundPath.substr(0, last_dot_pos);
        _wstring wStrSoundTag = StringToWString(strSoundTag);

        // 1. Sound Load
        m_pGameInstance->Load_Sound(wStrSoundTag, strFilePath.c_str());

        // 2. Sound ?대쫫 愿由?
        m_SoundTags.emplace(strSoundTag, wStrSoundTag);
    }
    else
    {
        MSG_BOX("寃쎈줈 ?섎せ??");
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

            // .wav ?뚯씪留?泥섎━
            if (extension == ".wav" || extension == ".WAV")
            {
                _string soundTag = entry.path().stem().string(); // ?뺤옣???쒖쇅???뚯씪紐?
                _wstring wSoundTag = StringToWString(soundTag);

                m_pGameInstance->Load_Sound(wSoundTag, filePath.c_str());
                m_SoundTags.emplace(soundTag, wSoundTag);
            }
        }
    }
}

void CAnimNotifyTool::Edit_SoundNotify()
{
    ImGui::BeginChild("Right pane", ImVec2(g_iWinSizeX * 0.25f, 0), true);

    ImGui::Text("Current Sound Tag :  %s", m_CurrentSoundTag.c_str());

    static float fTrackPosition = {};
    ImGui::InputFloat("TrackPosition", &fTrackPosition);

    // 媛??섏쑝硫?Max 媛믪쑝濡??먮룞 ?ㅼ젙.
    fTrackPosition = clamp(fTrackPosition, 0.f, m_fCurrentDuration);
   

    static float fVolume = {};
    ImGui::SliderFloat("Volume", &fVolume, 0.f, 1.f);

    if (ImGui::RadioButton("Effect", m_CurrentSoundType == "Effect"))
        m_CurrentSoundType = "Effect";

    ImGui::SameLine();

    if (ImGui::RadioButton("Other", m_CurrentSoundType == "Other"))
        m_CurrentSoundType = "Other";
  

    // 1. ?대옒?ㅻ줈 由ъ뒪?몄뿉 ??ν븯湲?
    if (ImGui::Button("Add SoundNotify"))
    {
        json SoundJson;
        SoundJson["TrackPosition"] = fTrackPosition;
        SoundJson["SoundTag"] = m_CurrentSoundTag;
        SoundJson["SoundType"] = m_CurrentSoundType;
        SoundJson["Volume"] = fVolume;  // ?뚯닔??3?먮━濡?諛섏삱由?
        CSoundNotify* pSoundNotify = CSoundNotify::From_Json(SoundJson);
        m_SoundNotifies.emplace_back(pSoundNotify);
    }

    ImGui::EndChild();
}

void CAnimNotifyTool::Save_NotifyToJson(const _string& strFilePath)
{
    // 吏?뺣맂 File 寃쎈줈濡?Json 留뚮뱾湲?.
    ofstream jsonStream(strFilePath.c_str());

    // 0. ?꾩껜 Json
    json notifyJson;

    // 1. Sound
    notifyJson["AnimName"] = m_strCurrentAnimName;
    notifyJson["Notifies"] = json::array();

    for (auto& soundNotify : m_SoundNotifies)
        notifyJson["Notifies"].emplace_back(soundNotify->To_Json());

    // 2. Effect

    // 3. Collider
    for (auto& colliderNotify : m_ColliderNotifies)
        notifyJson["Notifies"].emplace_back(colliderNotify->To_Json());

    // ????꾨즺.
    jsonStream << notifyJson.dump(4);
    jsonStream.close();
}

void CAnimNotifyTool::Load_NotifyFromJson(const _string& strFilePath)
{
    // 1. ?꾩옱 濡쒕뱶???곗씠??紐⑤몢 ??젣.
    Clear();

    ifstream jsonStream(strFilePath.c_str());
    if (!jsonStream.is_open())
    {
        return;
    }

    // 2. ?뚯씪 ?댁슜??json 媛앹껜濡??뚯떛
    json notifyJson;
    jsonStream >> notifyJson;
    jsonStream.close();

    // 3. ?뚯떛???곗씠?곕줈 硫ㅻ쾭 蹂??梨꾩슦湲?
    m_strCurrentAnimName = notifyJson["AnimName"].get<_string>();
    
    // 4. "Notifies" 諛곗뿴 ?쒗쉶 諛???낆뿉 留욊쾶 蹂듭썝
    if (notifyJson.contains("Notifies") && notifyJson["Notifies"].is_array())
    {
        for (const auto& notifyObject : notifyJson["Notifies"])
        {
            string type = notifyObject["NotifyType"].get<string>();

            if (type == "Sound")
            {
                // CSoundNotify ?대옒?ㅼ뿉??From_Json ?⑥닔媛 ?덈떎怨?媛??
                CSoundNotify* pSoundNotify = CSoundNotify::From_Json(notifyObject);
                m_SoundNotifies.emplace_back(pSoundNotify);
                

                Safe_AddRef(pSoundNotify);

                // 紐⑤뜽???꾨떖??List 而⑦뀒?대꼫
                m_AnimNotifies.emplace_back(pSoundNotify);
            }
            else if (type == "Collider")
            {
                CColliderNotify* pColliderNotify = CColliderNotify::From_Json(notifyObject);
                m_ColliderNotifies.emplace_back(pColliderNotify);

                Safe_AddRef(pColliderNotify);

                // 紐⑤뜽???꾨떖??List 而⑦뀒?대꼫
                m_AnimNotifies.emplace_back(pColliderNotify);
            }
            
            // else if (type == "Effect") { ... }
        }
    }
}
    



#pragma endregion


HRESULT CAnimNotifyTool::Ready_Sound()
{
    //m_pGameInstance->Load_Sound(TEXT("Augusta_Attack01"), "../../Client/Bin/Resource/Player/Augusta/Sound/Attack/Augusta_Attack01_01.wav");
    
    return S_OK;
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