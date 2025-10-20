#include"EditorPch.h"
#include "Map_Interface.h"
#include"Edit_PreViewModel.h"
#include"Edit_MapObject.h"

CMap_Interface::CMap_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CInterface_Edit(pDevice,pContext)
{
}

HRESULT CMap_Interface::Initialize()
{

    return S_OK;
}

_bool CMap_Interface::Set_ShaderPass(CShader* pShader, _uint* ShaderPassIndex)
{
    _bool Result = { false };
    ImGuiID ShaderId = ImGui::GetID("ShaderPass");
    ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
    ImGui::Text("ShaderPass");

    for (_uint i = 0; i < pShader->Get_PassCount(); ++i)
    {
        if (!strcmp("DebugRender", pShader->Get_PassName(i)))
            continue;

        if (ImGui::Button(pShader->Get_PassName(i))) {
            *ShaderPassIndex = i;
            Result = true;
            break;
        }
    }

    ImGui::EndChildFrame();


    return Result;
}

_bool CMap_Interface::Set_LOD(vector<CModel*>& pModelArray, _uint* iLODIndex)
{
    ImGuiID LODId= ImGui::GetID("LOD_Level");
    ImGui::BeginChildFrame(LODId, ImVec2(100, 200));

    _bool Result = { false };

    _char LOD[10] = {};
    sprintf_s(LOD, "LOD %d", *iLODIndex);
    ImGui::Text(LOD);
    _char LOD_Index[10] = {};

    for (_uint i = 0; i < pModelArray.size(); ++i)
    {
        if (pModelArray[i] == nullptr)
            continue;

        sprintf_s(LOD_Index, "LOD%d", i);
        if (ImGui::Button(LOD_Index))
        {
            *iLODIndex = i;
            Result = true;
            break;
        }
    }
    ImGui::EndChildFrame();

    return Result;
}

void CMap_Interface::Set_Transform(CTransform* pTransform)
{
    m_pGameInstance->Use_Gizmo(pTransform);

    _float vScale[3] = {};
    _float vRotation[3] = {};
    _float vTransfrom[3] = {};
    _matrix matrix = pTransform->Get_WorldMatrix();
    ImGuizmo::DecomposeMatrixToComponents(reinterpret_cast<_float*>(&matrix), vTransfrom, vRotation, vScale);

    ImGui::Text("Size");
    {
        ImGui::PushItemWidth(300.0f);
        ImGui::InputFloat3("Scale", vScale);
    }

    ImGui::Text("Turn_Quaternion");
    {
        //Î°úÌÖå?¥ÏÖò??Í≥ÑÏÜç ?ÖÎç∞?¥Ìä∏ ?òÏñ¥??Í∞íÏù¥ Ï¥àÍ∏∞?îÎê®.
        ImGui::PushItemWidth(300.0f);
        //?îÍ∑∏Î¶?Í∞ÅÎèÑÎ°?0?ÑÏóê??360?ÑÍπåÏßÄ.
        ImGui::InputFloat3("Rotation", vRotation);
    }

    ImGui::Text("Position");
    {
        ImGui::PushItemWidth(300.0f);
        ImGui::InputFloat3("Translation", vTransfrom);
    }

    ImGui::PopItemWidth();

    ImGuizmo::RecomposeMatrixFromComponents(vTransfrom, vRotation, vScale, reinterpret_cast<_float*>(&matrix));

    pTransform->Set_WorldMatrix(matrix);
}

_bool CMap_Interface::Load_Textures(_uint Origin, vector<_string>* VectorTextures, _string ResearchKeyWord, _string ResearchExt, _bool IsIntoChild, _string TextureFolderPath, _bool IsPng, _string SecondKeyWord)
{
    IGFD::FileDialogConfig config;

    TextureFolderPath.empty() ?
        //Soulution Parent Path
        config.path = filesystem::current_path().parent_path().parent_path().parent_path().string()
        :
        config.path = TextureFolderPath;

    _string FileExt;
    IsPng ?
        FileExt = ".png" :
        FileExt = ".dds";
    config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

    _char Text[32] = {};
    sprintf_s(Text, "%d###Texture File Load", 999);

    //if ResearchExt is empty, It Can Pick Folder
    ResearchExt.empty() ?
        ImGuiFileDialog::Instance()->OpenDialog(Text, "Texture File", nullptr, config)
        :
        ImGuiFileDialog::Instance()->OpenDialog(Text, "Texture File", ResearchExt.c_str(), config);

    if (ImGuiFileDialog::Instance()->Display(Text)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFolderPath = ImGuiFileDialog::Instance()->GetCurrentPath();
            for (const auto& entry : filesystem::recursive_directory_iterator(strFolderPath)) {
                if (entry.is_regular_file()) {
                    filesystem::path Filepath = entry.path();

                    _bool Condition;

                    SecondKeyWord.empty() ?
                        Condition = Filepath.string().find(ResearchKeyWord) != std::string::npos
                        :
                        Condition = (Filepath.string().find(ResearchKeyWord) != std::string::npos) || (Filepath.string().find(SecondKeyWord) != std::string::npos);

                    if (Condition)
                    {

                        if (Filepath.extension() == FileExt) {
                            {
                                string fileName = Filepath.filename().string();
                                VectorTextures->push_back(Filepath.string());
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

_bool CMap_Interface::Display_Textures(CTexture* pTexture, _uint iTextureNum, _float SizeX, _float SizeY)
{
    ImVec2 ImageSize;

    SizeY == 0.f ?
        ImageSize = ImVec2(SizeX, SizeX)
        :
        ImageSize = ImVec2(SizeX, SizeY);
    if (!pTexture)
        return false;

    ImGui::Image((ImTextureID)pTexture->Get_SRV(iTextureNum), ImageSize);

    return true;
}

_bool CMap_Interface::Initialize_ModelPath(_bool* Test)
{
    m_ModelPaths.clear();

    IGFD::FileDialogConfig config;
    config.path = "../../Client/Bin/Resource/Map/";
    config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

    _char Text[32] = {};
    sprintf_s(Text, "Object %d###Model Folder Load", 999);

    ImGuiFileDialog::Instance()->OpenDialog(Text, "Model Folder", nullptr, config);

    _int version = {};
    _int Lastversion = {};
    _wstring LastVersionName;
    _string LastVersionPath;

    if (ImGuiFileDialog::Instance()->Display(Text)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFolderPath = ImGuiFileDialog::Instance()->GetCurrentPath();

            for (const auto& entry : filesystem::recursive_directory_iterator(strFolderPath)) {
                if (entry.is_regular_file()) {
                    if (entry.path().string().find("MapData") != std::string::npos)
                        continue;

                    if (entry.path().extension() == ".dat") {

                        _char FileDrive[MAX_PATH] = {};
                        _char FileDir[MAX_PATH] = {};
                        _char FileName[MAX_PATH] = {};
                        _char FileExt[MAX_PATH] = {};
                        _splitpath_s(entry.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                        _wstring baseName = StringToWString(FileName);

                        // LOD ∏∂¡ˆ∏∑ø° ∫Ÿ¿∫ º˝¿⁄ √ﬂ√‚
                        size_t pos = baseName.find_last_not_of(TEXT("0123456789"));
                        _wstring namePart = baseName.substr(0, pos + 1);
                        _wstring numberPart = baseName.substr(pos + 1);
                        version = stoi(numberPart);

                        _wstring key = L"Prototype_Component_Model_" + namePart;
                        _string VersionPath = FileDir;
                        VersionPath += FileName;
                        VersionPath += ".dat";

                        if (lstrcmp(LastVersionName.c_str(), key.c_str()) && !LastVersionName.empty())
                        {
                            m_ModelPaths.push_back(LastVersionPath);
                        }

                        Lastversion = version;
                        LastVersionName = key;
                        LastVersionPath = VersionPath;
                    }
                }
            }
        }
            *Test = true;

            return true;
    }
    return false;
}

void CMap_Interface::Add_MapObject(_uint iLevel, _fmatrix PreTransformMatrix,_fvector vPos)
{
    if (m_ModelPaths.empty())
        return;

    if (!m_IsCreateProto)
    {
        /*if (FAILED(m_pGameInstance->Add_Prototype(iLevel, TEXT("Prototype_GameObject_MapObject"),
            CEdit_MapObject::Create(m_pDevice, m_pContext))))
            CRASH("Prototype Create Failed");*/

        m_IsCreateProto = true;
    }
    ImGui::Begin("Create Model Prototype & Clone");

    ImGuiID MapId = ImGui::GetID("Map Model");
    ImGui::BeginChildFrame(MapId, ImVec2(300, 200));
    for (_uint i = 0; i < m_ModelPaths.size(); ++i)
    {
        _char FileDrive[MAX_PATH] = {};
        _char FileDir[MAX_PATH] = {};
        _char FileName[MAX_PATH] = {};
        _char FileExt[MAX_PATH] = {};
        _splitpath_s(m_ModelPaths[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

        if (ImGui::Selectable(FileName))
        {


            //_wstring baseName = StringToWString(FileName);

            // LOD ∏∂¡ˆ∏∑ø° ∫Ÿ¿∫ º˝¿⁄ √ﬂ√‚
            //size_t pos = baseName.find_last_not_of(TEXT("0123456789"));
            //_wstring namePart = baseName.substr(0, pos + 1);
            //_wstring numberPart = baseName.substr(pos + 1);

            //_wstring PrototypeName = L"Prototype_Component_Model_" + namePart;

            _string ModelPath = FileDir;
            ModelPath += FileName;
            //ø©±‚º≠ º˝¿⁄ √ﬂ√‚
            ModelPath.pop_back();
            ModelPath += to_string(0);
            ModelPath += ".dat";

            //¡¶¿œ ≥Ù¿∫ º˝¿⁄∞° µÈæÓ∞®. 

            _wstring PrototypeName = TEXT("Prototype_Component_Model_");
            _wstring ModelName = StringToWString(FileName);
            ModelName.pop_back();
            PrototypeName += ModelName;

            /*if (FAILED(m_pGameInstance->Add_Prototype(iLevel, PrototypeName,
                CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, ModelPath.c_str()))))
                CRASH("Prototype Create Failed");*/

            _wstring ObjectName;
            CEdit_MapObject::MAP_LOAD Desc{};
            _float4x4 DefaultMatrix{};

            if (vPos.m128_f32[3] == 0.f)
                XMStoreFloat4x4(&DefaultMatrix, XMMatrixTranslationFromVector(vPos));
            else
                XMStoreFloat4x4(&DefaultMatrix, XMMatrixIdentity());

            Desc.WorldMatrix = &DefaultMatrix;
            Desc.iShaderPassIndex = 0;
            strcpy_s(Desc.ModelName, FileName);

            m_pGameInstance->Add_GameObject_ToLayer(iLevel, TEXT("Prototype_GameObject_MapObject")
                , iLevel, TEXT("Layer_MapObject"), &Desc);
            break;
        }
    }
    ImGui::EndChildFrame();


    ImGui::End();
}

CMap_Interface* CMap_Interface::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMap_Interface* pInstance = new CMap_Interface(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Map_Insterface");
        Safe_Release(pInstance);
    }

    return pInstance;
}


void CMap_Interface::Free()
{
    __super::Free();
    Safe_Release(m_pPreView);
}
