#include "EditorPch.h"
#include "Load_Controller.h"

#include "Effect_Prefab.h"

CLoad_Controller::CLoad_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CLoad_Controller::Initialize()
{

    return S_OK;
}

void CLoad_Controller::Update()
{

}

void CLoad_Controller::Render()
{

}

void CLoad_Controller::Prefab_Load_Tab()
{
    if (ImGui::Button("Load Prefab"))
    {
        IGFD::FileDialogConfig config;
        config.path = "../../Client/Bin/Resource/Effect/Prefab";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

        ImGuiFileDialog::Instance()->OpenDialog("Load Prefab", "Import", ".json", config);
    }

    if (ImGuiFileDialog::Instance()->Display("Load Prefab"), ImGuiWindowFlags_NoCollapse)
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            _string strFilePath = {};
            _string strFolderPath = {};

            strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

            size_t lastSlashPos = strFilePath.find_last_of("\\");

            //마지막 문자열 빼고 폴더 경로만 가져오기. 
            if (lastSlashPos != string::npos) {
                strFolderPath += strFilePath.substr(0, lastSlashPos);
            }

            Load_Prefab_FromJson(strFilePath);
        }
    }
}

void CLoad_Controller::Load_Prefab_FromJson(const _string& strFilePath)
{
    ifstream JsonStream(strFilePath.c_str());

    if (!JsonStream.is_open())
        return;

    json PrefabJson;
    JsonStream >> PrefabJson;
    JsonStream.close();

    CEffect_Prefab::PREFAB_DESC pPrefabDesc = {};
    
    if (PrefabJson.contains("Prefab_Name"))
        pPrefabDesc.strPrefabTag = StringToWString(PrefabJson["Prefab_Name"].get<string>());

    if(PrefabJson.contains("Children_Number"))
        pPrefabDesc.ChildrenCount = PrefabJson["Children_Number"].get<_int>();

    if (PrefabJson.contains("Prefab_LifeTime") && PrefabJson["Prefab_LifeTime"].is_array())
    {
        json LifeTime = PrefabJson["Prefab_LifeTime"];

        pPrefabDesc.vLifeTime.x = static_cast<_float>(LifeTime[0].get<double>());
        pPrefabDesc.vLifeTime.y = static_cast<_float>(LifeTime[1].get<double>());
    }

    if (PrefabJson.contains("Frames") && PrefabJson["Frames"].is_array())
    {
        for (size_t i = 0; i < pPrefabDesc.ChildrenCount; i++)
        {
            CEffect_Prefab::FRAME_DESC pFrameDesc = {};

            json Frame = PrefabJson["Frames"][i];

            if (Frame.contains("Children_Name"))
            {
                pFrameDesc.strChildrenTag = StringToWString(Frame["Children_Name"].get<string>());
            }

            if(Frame.contains(""))

            if (Frame.contains("Activate_Time"))
            {
                pFrameDesc.fActivateTime = Frame["Activate_Time"].get<double>();
            }
        }

    }

}

CLoad_Controller* CLoad_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLoad_Controller* pInstance = new CLoad_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CLoad_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoad_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

}