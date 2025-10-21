#include"Editorpch.h"
#include "Edit_MapObject.h"
#include"Model_Instance.h"
#include"Mesh_Instance.h"
#include"Event_Level.h"
#include "AnimationActor.h"
#include"Level_Map.h"
#include"Map_Interface.h"

_uint CEdit_MapObject::g_iNumObjects = {};

CEdit_MapObject::CEdit_MapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CStaticObject(pDevice, pContext)
{
}

CEdit_MapObject::CEdit_MapObject(const CEdit_MapObject& Prototype)
    :CStaticObject(Prototype)
{
}

HRESULT CEdit_MapObject::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEdit_MapObject::Initialize_Clone(void* pArg)
{
    MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;
#ifdef _DEBUG
    strcpy_s(m_ModelName, pDesc->ModelName);

    m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

    if (FAILED(Ready_Component(pArg)))
        return E_FAIL;

    m_iNumLOD = m_pModelComArray.size()-1;

    /*Sync_BoundingBox(m_pModelCom->Get_BoundingBox(0), m_pTransformCom->Get_WorldMatrix());
    m_pGameInstance->Add_To_OctoTree(this, m_pModelCom->Get_BoundingBox(0));*/
    _vector vScale, vRotation, vTranslation;

    XMMatrixDecompose(&vScale, &vRotation, &vTranslation, m_pTransformCom->Get_WorldMatrix());

    XMStoreFloat3(&m_vScale, vScale);
    XMStoreFloat3(&m_vTranslation, vTranslation);
    m_vNewScale = m_vScale;
    m_vRotation = m_vNewRotation = _float3(0.f, 0.f, 0.f);
    m_vNewTranslation = m_vTranslation;
    m_iShaderPassIndex = pDesc->iShaderPassIndex;
    MODELTYPE::MAP;

    _char Tag[MAX_PATH] = "NonInteraction";
    MAP_CREATE event(Tag, this);

    m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Create_Object"), event);

    m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map"), [this](const MAP_SAVE& event) {
        if (!m_isActivate)
            return;

        //寃쎈줈 吏?뺥븷 ???곸쐞 ?대뜑???ㅼ뿉 LOD 鍮쇨퀬. ?대뜑瑜?吏?? 洹몃━怨?洹??덉뿉 ?덈뒗 ?대뜑 ?섏쐞 1媛??뚮㈃??.dat???쎄퀬 媛앹껜 ?덉뿉 ?ｊ린?
        
        /*OBJECT_SAVE Save{};
        Save.m_iNameLength = strlen(m_ModelName);
        strcpy_s(Save.ModelName, m_ModelName);
        Save.iShaderPassIndex = m_iShaderPassIndex;
        XMStoreFloat4x4(&Save.WorldMatrix, m_pTransformCom->Get_WorldMatrix());
        
        event.File.write(reinterpret_cast<const char*>(&Save), sizeof(OBJECT_SAVE));*/
        
        _uint Length = strlen(m_ModelName);
        event.File.write(reinterpret_cast<const char*>(&Length), sizeof(_uint));
        event.File.write(m_ModelName, Length);
        if (m_iShaderPassIndex == 3)
            m_iShaderPassIndex = 0;
        event.File.write(reinterpret_cast<const char*>(&m_iShaderPassIndex), sizeof(_uint));

        _float4x4 WorldMatrix;
        XMStoreFloat4x4(&WorldMatrix, m_pTransformCom->Get_WorldMatrix());
        event.File.write(reinterpret_cast<const _char*>(&WorldMatrix), sizeof(_float4x4));
        });
#endif

    m_pDiffuseTextureCom.resize(m_pModelCom->Get_NumMesh());
    m_pNormalTextureCom.resize(m_pModelCom->Get_NumMesh());
    m_pMaskTextureCom.resize(m_pModelCom->Get_NumMesh());
    m_pMaskDiffuseTextureCom.resize(m_pModelCom->Get_NumMesh());

    m_SelectedDiffuseName.resize(m_pModelCom->Get_NumMesh());
    m_SelectedNormalName.resize(m_pModelCom->Get_NumMesh());
    m_SelectedMaskTextureName.resize(m_pModelCom->Get_NumMesh());
    m_SelectedMaskDiffuseName.resize(m_pModelCom->Get_NumMesh());


    m_SelectedDiffuseTexturePath.resize(m_pModelCom->Get_NumMesh());
    m_SelectedNormalTexturePath.resize(m_pModelCom->Get_NumMesh());
    m_SelectedMaskTexturePath.resize(m_pModelCom->Get_NumMesh());
    m_SelectedMaskTexturePath.resize(m_pModelCom->Get_NumMesh());
    m_SelectedMaskDiffusePath.resize(m_pModelCom->Get_NumMesh());


    m_iSelectedDiffuseIndex = new _uint[m_pModelCom->Get_NumMesh()];
    m_iSelectedNormalIndex = new _uint[m_pModelCom->Get_NumMesh()];
    m_iSelectedMaskIndex = new _uint[m_pModelCom->Get_NumMesh()];
    m_iSelectedMaskDiffuseIndex = new _uint[m_pModelCom->Get_NumMesh()];


    m_iSelectedMesh = 0;
    m_iSelectedMeshName = "Mesh : 0";


    m_iNumObject = CEdit_MapObject::g_iNumObjects++;
    return S_OK;
}

void CEdit_MapObject::Priority_Update(_float fTimeDelta)
{
}

void CEdit_MapObject::Update(_float fTimeDelta)
{
#ifdef _DEBUG
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        if (m_iLevel == ENUM_CLASS(LEVEL::MAP))
        {

            if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN)
            {
                //?ш린???대┃ 理쒖쟻???섎젮硫??꾨윭?ㅽ? 而щ쭅源뚯?.

                _float fDistance = {};
                //?붾뱶??諛붽퓭?쇳븿.
                _vector RayPos = XMVector3TransformCoord(XMLoadFloat3(&CLevel_Map::m_vWorldPos), m_pTransformCom->Get_WorldMatrix_Inv());
                _vector RayDir = XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&CLevel_Map::m_vWorldDir), m_pTransformCom->Get_WorldMatrix_Inv()));
                if (m_pModelCom->Is_Picked(RayPos, RayDir, &fDistance))
                {
                    MAP_PICK event(this, fDistance);

                    m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("ObjectPick"), event);
                }
            }
        }
    }
#endif
    m_pModelCom = m_pModelComArray[m_iLODIndex];
}

void CEdit_MapObject::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_MapObject::Render()
{
    //�Ⱥ��̴� �� ����
    Bind_Resources();

    for (_uint i = 0; i < m_pModelComArray[m_iLODIndex]->Get_NumMesh(); ++i)
    {
        if (m_TexMode)
        {
            //Ŀ���� �ؽ��� ���� ����ŷ �̹��� ��� ����� �ȵ�.
            if (m_pDiffuseTextureCom[i])
                m_pDiffuseTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture",0);
            if (m_pNormalTextureCom[i])
                m_pNormalTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_NormalTexture");
            if (m_pMaskTextureCom[i])
                m_pMaskTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_MaskTexture");
            else
                m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
            if (m_pMaskDiffuseTextureCom[i])
                m_pMaskDiffuseTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture", 1);
        }
        else
        {
            m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);

            _bool HasNormal = { true };

            if (FAILED(m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
                HasNormal = false;

            m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));

            if (FAILED(m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK)))
                m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
        }

        m_pShaderCom->Begin(m_iShaderPassIndex);

        m_pModelComArray[m_iLODIndex]->Render(i);
    }
}

void CEdit_MapObject::Render_Shadow()
{

}

void CEdit_MapObject::Set_ImGuiOption()
{
#ifdef _DEBUG
    ImGui::Text(m_ModelName);


    About_Parent();

    About_Transform();

    m_pMapInterface->Set_ShaderPass(m_pShaderCom, &m_iShaderPassIndex);
    ImGui::SameLine();
    m_pMapInterface->Set_LOD(m_pModelComArray, &m_iLODIndex);

    if (ImGui::Button("Set Texture"))
    {
        if (!m_IsCustomTexture)
        {
            m_EntireDiffuseTextureName.clear();
            m_EntireNormalTextureName.clear();
            m_EntireMaskTextureName.clear();

        }
        m_IsCustomTexture = !m_IsCustomTexture;
    }

    ImGui::SameLine();
    ImGui::Checkbox("Custom Tex", &m_TexMode);

    if (ImGui::Button("Destroy"))
        m_isActivate = false;

    About_Texture();
#endif
}


HRESULT CEdit_MapObject::Ready_Component(void* pArg)
{
    m_pGameInstance->Wait_Thread_End();
    MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

    m_iLevel = pDesc->iLevel;
    
    _tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
    _tchar Name[MAX_PATH] = {};

    MultiByteToWideChar(CP_ACP, 0, m_ModelName, -1, Name, strlen(m_ModelName));
    lstrcat(Model, Name);
    _uint V = m_ModelName[strlen(m_ModelName) - 1] - '0' + 1;
    
    m_pModelComArray.resize(V);

    for (_uint i = 0; i < V; ++i)
    {
        _wstring ModelCom = Model;
        ModelCom.pop_back();
        ModelCom += to_wstring(i);
        
        _char ModelName[MAX_PATH] = {};
        sprintf_s(ModelName, "Com_Model%d", i);
        if (FAILED(Add_Component(pDesc->iLevel, ModelCom,
            StringToWString(ModelName), reinterpret_cast<CComponent**>(&m_pModelComArray[i]), nullptr)))
            CRASH("FAILED");

    }
    m_pGameInstance->Wait_Thread_End();

    if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    m_pMapInterface = CMap_Interface::Create(m_pDevice, m_pContext);
    m_pGameInstance->Wait_Thread_End();
    m_pModelCom = m_pModelComArray[0];
    return S_OK;
}

void CEdit_MapObject::Bind_Resources()
{
    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

void CEdit_MapObject::Add_Child(CEdit_MapObject* pObject)
{
    _bool Same = { true };

    if (pObject->m_pParent || pObject == this || m_pParent == pObject)
        return;

    for (auto& pChild : m_ChildObjects)
    {
        if (pChild == pObject)
        {
            Same = false;
            break;
        }
    }
    if(Same)
    {
        pObject->m_pParent = this;
        pObject->Make_ChildLocalMatrix(m_pTransformCom->Get_WorldMatrix());
        m_ChildObjects.push_back(pObject);
        m_IsParent = true;
    }
    pObject->m_IsSetParent = false;
}

void CEdit_MapObject::Quit_Child(CEdit_MapObject* pObject)
{
    m_ChildObjects.remove(pObject);
    if (m_ChildObjects.empty())
        m_IsParent = false;
}

void CEdit_MapObject::Make_ChildLocalMatrix(_fmatrix ParentMatrix)
{
    XMStoreFloat4x4(&m_ChildLocalMat, m_pTransformCom->Get_WorldMatrix() * XMMatrixInverse(nullptr, ParentMatrix));
    _matrix NewChildWolrd = XMLoadFloat4x4(&m_ChildLocalMat) * ParentMatrix;
    m_pTransformCom->Set_WorldMatrix(NewChildWolrd);
}


void CEdit_MapObject::Export_MaterialData()
{

    //理쒖쥌 ?대뜑 寃쎈줈.

    IGFD::FileDialogConfig config1;
    _char ModelPath[MAX_PATH] = {};

    strcat_s(ModelPath, filesystem::current_path().parent_path().parent_path().string().c_str());


    strcat_s(ModelPath, "/Client/Bin/Resource/Map/");
    for (const auto& entry : filesystem::recursive_directory_iterator(ModelPath))
    {
        if (entry.path().extension() != ".dat")
            continue;
        _string Name = entry.path().filename().replace_extension().string();

        if (entry.is_regular_file() && !strcmp(Name.c_str(), m_ModelName))
        {
            strcpy_s(ModelPath, entry.path().parent_path().string().c_str());
            strcat_s(ModelPath, "/");
            break;
        }
    }

    config1.path = string(ModelPath);
    config1.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;
    _char Text[32] = {};
    sprintf_s(Text,"Export %d###Select.dat", m_iNumObject);
    ImGuiFileDialog::Instance()->OpenDialog("Export Json", Text, ".dat", config1);

    if (ImGuiFileDialog::Instance()->Display("Export Json")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFolderName = ImGuiFileDialog::Instance()->GetCurrentPath();
            _string strTexturePath = {};
            strFolderName += "/Mat/";
            strTexturePath = strFolderName;
            strFolderName += m_ModelName;
            strFolderName.pop_back();

            _string FileExt = ".json";
            _int i = 0;

            ofstream File(strFolderName + to_string(i++) + FileExt);
        
#pragma region Json 추출
            strTexturePath += "/Tex/";
            filesystem::create_directories(strTexturePath);
            json Totaljson;
            Totaljson["NumMaterial"] = m_pModelCom->Get_NumMesh();
            Totaljson["Materials"] = json::array();

            for (_uint i = 0; i < m_pModelCom->Get_NumMesh(); ++i)
            {
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char DiffuseFileName[MAX_PATH] = {};
                _char NormalFileName[MAX_PATH] = {};
                _char MaskFileName[MAX_PATH] = {};
                _char MaskDiffuseFileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};

                _splitpath_s(m_SelectedDiffuseTexturePath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, DiffuseFileName, MAX_PATH, FileExt, MAX_PATH);
                _splitpath_s(m_SelectedNormalTexturePath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, NormalFileName, MAX_PATH, FileExt, MAX_PATH);
                if (!m_SelectedMaskTexturePath[i].empty())
                {
                    _splitpath_s(m_SelectedMaskTexturePath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, MaskFileName, MAX_PATH, FileExt, MAX_PATH);
                    _splitpath_s(m_SelectedMaskDiffusePath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, MaskDiffuseFileName, MAX_PATH, FileExt, MAX_PATH);
                }

                json MaterialData;

                json DiffuseData;

                _uint TextureCnt = { 0 };
                DiffuseData["FileName"] = json::array();

                _char DiffuseName[MAX_PATH] = {};
                strcat_s(DiffuseName, DiffuseFileName);
                strcat_s(DiffuseName, FileExt);

                DiffuseData["FileName"].push_back(DiffuseName);

                if(!m_SelectedMaskTexturePath[i].empty())
                {
                    _char MaskDiffuseName[MAX_PATH] = {};
                    strcat_s(MaskDiffuseName, MaskDiffuseFileName);
                    strcat_s(MaskDiffuseName, FileExt);

                    DiffuseData["FileName"].push_back(MaskDiffuseName);
                    DiffuseData["TextureCnt"] = 2;
                }
                else
                    DiffuseData["TextureCnt"] = 1;

                MaterialData["Diffuse"] = DiffuseData;

                json NormalData;

                NormalData["TextureCnt"] = 1;
                NormalData["FileName"] = json::array();

                _char NormalName[MAX_PATH] = {};
                strcat_s(NormalName, NormalFileName);
                strcat_s(NormalName, FileExt);

                NormalData["FileName"].push_back(NormalName);

                MaterialData["Normal"] = NormalData;

                json MaskData;
                if (!m_SelectedMaskTexturePath[i].empty())
                {
                    MaskData["TextureCnt"] = 1;
                    MaskData["FileName"] = json::array();

                    _char MaskName[MAX_PATH] = {};
                    strcat_s(MaskName, MaskFileName);
                    strcat_s(MaskName, FileExt);

                    MaskData["FileName"].push_back(MaskName);

                    MaterialData["Mask"] = MaskData;
                }


                Totaljson["Materials"].push_back(MaterialData);


                filesystem::copy_file(m_SelectedDiffuseTexturePath[i], strTexturePath + DiffuseFileName + FileExt, filesystem::copy_options::overwrite_existing);
                filesystem::copy_file(m_SelectedNormalTexturePath[i], strTexturePath + NormalFileName + FileExt, filesystem::copy_options::overwrite_existing);
                if (!m_SelectedMaskTexturePath[i].empty())
                {

                    filesystem::copy_file(m_SelectedMaskTexturePath[i], strTexturePath + MaskFileName + FileExt, filesystem::copy_options::overwrite_existing);
                    filesystem::copy_file(m_SelectedMaskDiffusePath[i], strTexturePath + MaskDiffuseFileName + FileExt, filesystem::copy_options::overwrite_existing);
                }
            }
            File << Totaljson.dump(4);
            File.close();

            if(m_ExportAllLOD)
            {
                for (_uint i = 1; i <= m_iNumLOD; ++i)
                {
                    _string JsonName = strFolderName + to_string(i) + FileExt;
                    ofstream File(JsonName);
                    File << Totaljson.dump(4);
                    File.close();
                }
            }

            m_MakeJson = !m_MakeJson;
            ImGuiFileDialog::Instance()->Close();

#pragma endregion
        }
        else
        {
            ImGuiFileDialog::Instance()->Close();
            m_MakeJson = !m_MakeJson;
        }
    }
}

void CEdit_MapObject::Child_UpdateMatrix(_fmatrix Matrix, _fvector vParentsPos, _fvector vDeltaTranslation)
{
    _matrix NewChildWolrd = XMLoadFloat4x4(&m_ChildLocalMat) * Matrix;
    m_pTransformCom->Set_WorldMatrix(NewChildWolrd);


    XMStoreFloat3(&m_vNewTranslation, NewChildWolrd.r[3]);

    for (auto& pChild : m_ChildObjects)
        pChild->Child_UpdateMatrix(m_pTransformCom->Get_WorldMatrix(), XMVectorSetW(XMLoadFloat3(&m_vNewTranslation), 1.f), XMVectorSetW(XMLoadFloat3(&m_vNewTranslation) - XMLoadFloat3(&m_vTranslation), 1.f));
}

void CEdit_MapObject::About_Parent()
{
    if (ImGui::Button("Set Parent"))
    {
        MAP_CREATE event(m_ModelName, this);
        m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Set_Parent"), event);
    }

    if (m_pParent)
    {
        ImGui::SameLine();
        if (ImGui::Button("Delete Parent"))
        {
            m_pParent->Quit_Child(this);
            m_pParent = nullptr;
        }
    }

    if (m_IsParent)
    {
        ImGuiID ShaderId = ImGui::GetID("Child");
        ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
        ImGui::Text("Child");

        for (auto& pChildObject : m_ChildObjects)
        {
            _char ChildName[MAX_PATH] = {};
            sprintf_s(ChildName, "%d", pChildObject->m_iNumObject);

            if (ImGui::Button(ChildName) || ImGui::IsItemHovered())
            {
                if (m_pPickedChild)
                    m_pPickedChild->m_iShaderPassIndex = 0;
                m_pPickedChild = pChildObject;
                m_pPickedChild->m_iShaderPassIndex = 3;
            }
            else
                if (m_pPickedChild)
                    m_pPickedChild->m_iShaderPassIndex = 0;
        }
        ImGui::EndChildFrame();
    }
}

void CEdit_MapObject::About_Transform()
{
    m_pMapInterface->Set_Transform(m_pTransformCom);


    for (auto& pChild : m_ChildObjects)
        pChild->Child_UpdateMatrix(m_pTransformCom->Get_WorldMatrix(), XMVectorSetW(XMLoadFloat3(&m_vNewTranslation), 1.f), XMVectorSetW(XMLoadFloat3(&m_vNewTranslation) - XMLoadFloat3(&m_vTranslation), 1.f));

    if (m_pParent)
        Make_ChildLocalMatrix(dynamic_cast<CTransform*>(m_pParent->Get_Component(TEXT("Com_Transform")))->Get_WorldMatrix());
}

void CEdit_MapObject::About_Texture()
{
    if (m_IsCustomTexture)
    {
        IGFD::FileDialogConfig config;

        //config.path = "C:/Users/dnheu/source/repos";
        config.path = filesystem::current_path().parent_path().parent_path().parent_path().string();
        //洹몃븣洹몃븣 諛붽퓭?쇨린.
        //config.path = "C:/Users/dnheu/Downloads/FModel/Output/Exports/Client/Content/Aki/Scene/Assets/Levels/LiNaXiTa/DiSiTaiDi/Rock/Json_Texture";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

        _char Text[32] = {};
        sprintf_s(Text, "Object %d###Texture Load", m_iNumObject);
        ImGuiFileDialog::Instance()->OpenDialog(Text, "Import File", nullptr, config);

        if (ImGuiFileDialog::Instance()->Display(Text)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                _string strFolderPath = ImGuiFileDialog::Instance()->GetCurrentPath();
                for (const auto& entry : filesystem::recursive_directory_iterator(strFolderPath)) {
                    if (entry.is_regular_file()) {
                        filesystem::path Filepath = entry.path();

                        if ((Filepath.string().find("_D_") != std::string::npos) || (Filepath.string().find("_D") != std::string::npos))
                        {
                            if (Filepath.extension() == ".png") {
                                {
                                    string fileName = Filepath.filename().string();
                                    
                                    m_EntireDiffuseTextureName.push_back(Filepath.string());
                                }
                            }
                        }
                        else if (Filepath.string().find("_N_") != std::string::npos || (Filepath.string().find("_N") != std::string::npos))
                        {
                            if (Filepath.extension() == ".png") {
                                {
                                    string fileName = Filepath.filename().string();
                                    m_EntireNormalTextureName.push_back(Filepath.string());
                                }
                            }
                        }
                        else if (Filepath.string().find("_MA_") != std::string::npos || (Filepath.string().find("_MA") != std::string::npos))
                        {
                            if (Filepath.extension() == ".png") {
                                {
                                    string fileName = Filepath.filename().string();
                                    m_EntireMaskTextureName.push_back(Filepath.string());
                                }
                            }
                        }
                    }
                }
                m_IsLoaded = true;
                m_IsCustomTexture = !m_IsCustomTexture;
                ImGuiFileDialog::Instance()->Close();
            }
            else
            {
                ImGuiFileDialog::Instance()->Close();
                m_IsCustomTexture = !m_IsCustomTexture;
            }
        }
    }

    if (m_IsLoaded)
    {
        ImGui::Begin("Texture Change");

        _char LOD_Index[10] = {};

        if (ImGui::BeginCombo("Meshes", m_iSelectedMeshName.c_str()))
        {
            for (_uint i = 0; i < m_pModelCom->Get_NumMesh(); ++i)
            {
                sprintf_s(LOD_Index, "Mesh : %d", i);
                if (ImGui::Selectable(LOD_Index))
                {
                    m_iSelectedMesh = i;
                    m_iSelectedMeshName = LOD_Index;

                }
            }
            ImGui::EndCombo();
        }

        //m_SelectedDiffuseTexturePath[m_iSelectedMesh] = "C:/Users/dnheu/source/repos/Wuthering_Wave_Final/Client/Bin/Resource/Map/Rock/SM_Sev_Roc_15AM/Mat/Tex/T4_Com2_Roc_05A_D.png";

        if (ImGui::BeginCombo("Diffuse", m_SelectedDiffuseName[m_iSelectedMesh].c_str()))
        {
            for (_uint i = 0; i < m_EntireDiffuseTextureName.size(); ++i)
            {
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(m_EntireDiffuseTextureName[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);
                if (ImGui::Selectable(FileName) || ImGui::IsItemHovered())
                {
                    ImGui::SetItemDefaultFocus();
                    m_SelectedDiffuse = m_EntireDiffuseTextureName[i];


                    _wstring Test(m_SelectedDiffuse.begin(), m_SelectedDiffuse.end());
                    if (m_pDiffuseTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pDiffuseTextureCom[m_iSelectedMesh]);

                    m_pDiffuseTextureCom[m_iSelectedMesh] = (CTexture::Create(m_pDevice, m_pContext, Test.c_str(), 1));
                    m_SelectedDiffuseTexturePath[m_iSelectedMesh] = m_EntireDiffuseTextureName[i].c_str();


                    m_SelectedDiffuseName[m_iSelectedMesh] = FileName;
                    m_iSelectedDiffuseIndex[m_iSelectedMesh] = i;
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Normal", m_SelectedNormalName[m_iSelectedMesh].c_str()))
        {

            for (_uint i = 0; i < m_EntireNormalTextureName.size(); ++i)
            {
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(m_EntireNormalTextureName[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                if (ImGui::Selectable(FileName) || ImGui::IsItemHovered())
                {
                    ImGui::SetItemDefaultFocus();
                    m_SelectedNormal = m_EntireNormalTextureName[i].c_str();

                    //W?ㅽ듃留곸쑝濡?諛붽퓭.
                    _wstring Test(m_SelectedNormal.begin(), m_SelectedNormal.end());
                    if (m_pNormalTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pNormalTextureCom[m_iSelectedMesh]);

                    m_pNormalTextureCom[m_iSelectedMesh] = (CTexture::Create(m_pDevice, m_pContext, Test.c_str(), 1));
                    m_SelectedNormalTexturePath[m_iSelectedMesh] = m_EntireNormalTextureName[i].c_str();

                    m_SelectedNormalName[m_iSelectedMesh] = FileName;
                    m_iSelectedNormalIndex[m_iSelectedMesh] = i;
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Mask", m_SelectedMaskTextureName[m_iSelectedMesh].c_str()))
        {

            for (_uint i = 0; i < m_EntireMaskTextureName.size(); ++i)
            {
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(m_EntireMaskTextureName[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                if (ImGui::Selectable(FileName) || ImGui::IsItemHovered())
                {
                    ImGui::SetItemDefaultFocus();
                    m_SelectedMask = m_EntireMaskTextureName[i].c_str();

                    //W��Ʈ������ �ٲ�.
                    _wstring Test(m_SelectedMask.begin(), m_SelectedMask.end());
                    if (m_pMaskTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pMaskTextureCom[m_iSelectedMesh]);

                    m_pMaskTextureCom[m_iSelectedMesh] = (CTexture::Create(m_pDevice, m_pContext, Test.c_str(), 1));
                    m_SelectedMaskTexturePath[m_iSelectedMesh] = m_EntireMaskTextureName[i].c_str();

                    m_SelectedMaskTextureName[m_iSelectedMesh] = FileName;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::Button("X###Mask"))
        {
            if (m_pMaskTextureCom[m_iSelectedMesh])
                Safe_Release(m_pMaskTextureCom[m_iSelectedMesh]);

            m_SelectedMaskTexturePath[m_iSelectedMesh] = "";
            m_SelectedMaskTextureName[m_iSelectedMesh] = "";
        }

        if (ImGui::BeginCombo("X###MaskDiffuse", m_SelectedMaskDiffuseName[m_iSelectedMesh].c_str()))
        {

            for (_uint i = 0; i < m_EntireDiffuseTextureName.size(); ++i)
            {
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(m_EntireDiffuseTextureName[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                if (ImGui::Selectable(FileName) || ImGui::IsItemHovered())
                {
                    ImGui::SetItemDefaultFocus();
                    m_SelectedMaskDiffuse = m_EntireDiffuseTextureName[i].c_str();

                    //W��Ʈ������ �ٲ�.
                    if (m_pMaskDiffuseTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pMaskDiffuseTextureCom[m_iSelectedMesh]);

                    m_pMaskDiffuseTextureCom[m_iSelectedMesh] = CTexture::Create(m_pDevice, m_pContext, StringToWString(m_SelectedMaskDiffuse).c_str(), 1);
                    m_SelectedMaskDiffusePath[m_iSelectedMesh] = m_EntireDiffuseTextureName[i].c_str();

                    m_SelectedMaskDiffuseName[m_iSelectedMesh] = FileName;
                    m_iSelectedMaskIndex[m_iSelectedMesh] = i;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::Button("MaskDiffuse###X"))
        {
            if (m_pMaskDiffuseTextureCom[m_iSelectedMesh])
                Safe_Release(m_pMaskDiffuseTextureCom[m_iSelectedMesh]);

            m_SelectedMaskDiffusePath[m_iSelectedMesh] = "";
            m_SelectedMaskDiffuseName[m_iSelectedMesh] = "";
        }

        if (ImGui::Button("Make_Json"))
            m_MakeJson = !m_MakeJson;

        ImGui::SameLine();
        ImGui::Checkbox("Export All LOD Level", &m_ExportAllLOD);

        if (m_MakeJson)
            Export_MaterialData();

        ImGui::Begin("Textures", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

        ImVec2 ImageSize = ImVec2(256.f, 256.f);
        m_pMapInterface->Display_Textures(m_pDiffuseTextureCom[m_iSelectedMesh]);
        ImGui::SameLine();
        m_pMapInterface->Display_Textures(m_pNormalTextureCom[m_iSelectedMesh]);
        ImGui::SameLine();
        m_pMapInterface->Display_Textures(m_pMaskTextureCom[m_iSelectedMesh]);
        ImGui::SameLine();
        m_pMapInterface->Display_Textures(m_pMaskDiffuseTextureCom[m_iSelectedMesh]);
 
        //ImGui::SameLine();
        //if (m_pNormalTextureCom[m_iSelectedMesh])
        //    ImGui::Image((ImTextureID)m_pNormalTextureCom[m_iSelectedMesh]->Get_SRV(0), ImageSize);

        //if (m_pMaskTextureCom[m_iSelectedMesh])
        //    ImGui::Image((ImTextureID)m_pMaskTextureCom[m_iSelectedMesh]->Get_SRV(0), ImageSize);

        //ImGui::SameLine();
        //if (m_pMaskDiffuseTextureCom[m_iSelectedMesh])
        //    ImGui::Image((ImTextureID)m_pMaskDiffuseTextureCom[m_iSelectedMesh]->Get_SRV(0), ImageSize);

        ImGui::End();

        ImGui::End();
    }

}

CEdit_MapObject* CEdit_MapObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEdit_MapObject* pInstance = new CEdit_MapObject(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : MapObject");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEdit_MapObject::Clone(void* pArg)
{
    CEdit_MapObject* pInstance = new CEdit_MapObject(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : MapObject (Clone)");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEdit_MapObject::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pRigidbodyCom);

    Safe_Delete(m_iSelectedDiffuseIndex);
    Safe_Delete(m_iSelectedNormalIndex);
    Safe_Delete(m_iSelectedMaskIndex);
    Safe_Delete(m_iSelectedMaskDiffuseIndex);

    m_pParent = nullptr;
    m_pPickedChild = nullptr;
    Safe_Release(m_pMapInterface);

    for (auto& pModel : m_pModelComArray)
        Safe_Release(pModel);

    for (auto& pTexture : m_pDiffuseTextureCom)
        if (pTexture)
            Safe_Release(pTexture);
    
    for (auto& pTexture : m_pNormalTextureCom)
        if (pTexture)
            Safe_Release(pTexture);

    for (auto& pTexture : m_pMaskTextureCom)
        if (pTexture)
            Safe_Release(pTexture);

    for (auto& pTexture : m_pMaskDiffuseTextureCom)
        if (pTexture)
            Safe_Release(pTexture);


    for (auto& pChild : m_ChildObjects)
        pChild = nullptr;

    m_pGameInstance->Unscribe();
}