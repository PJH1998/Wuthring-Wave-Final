#include"Editorpch.h"
#include "MapObject.h"
#include"Model_Instance.h"
#include"Mesh_Instance.h"
#include"Event_Level.h"
#include "AnimationActor.h"
#include"Level_Map.h"

CMapObject::CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CGameObject(pDevice, pContext)
{
}

CMapObject::CMapObject(const CMapObject& Prototype)
    :CGameObject(Prototype)
{
}

HRESULT CMapObject::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CMapObject::Initialize_Clone(void* pArg)
{
    MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);
    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    strcpy_s(m_ModelName, pDesc->ModelName);
    m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

    if (FAILED(Ready_Component(pArg)))
        return E_FAIL;


    _vector vScale, vRotation, vTranslation;

    XMMatrixDecompose(&vScale, &vRotation, &vTranslation, m_pTransformCom->Get_WorldMatrix());

    XMStoreFloat3(&m_vScale, vScale);
    XMStoreFloat3(&m_vTranslation, vTranslation);
    m_vNewScale = m_vScale;
    m_vRotation = m_vNewRotation = _float3(0.f, 0.f, 0.f);
    m_vNewTranslation = m_vTranslation;
    m_iShaderPassIndex = pDesc->iShaderPassIndex;
    MODELTYPE::MAP;

    //ImGui에서 저장 누를 때 모델 이름별로 이 모델은 어디다 저장할지 선택하게?
#ifdef _DEBUG

    //진짜 마음에 안듦. 나중에 물어보고 수정할것

    //MAP_CREATE event(m_ModelName, this);

    _char Tag[MAX_PATH] = "NonInteraction";
    MAP_CREATE event(Tag, this);

    m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Create_Object"), event);

    m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map"), [this](const MAP_SAVE& event) {
        if (!m_isActivate)
            return;

        _uint Length = strlen(m_ModelName);
        event.File.write(reinterpret_cast<const char*>(&Length), sizeof(_uint));
        event.File.write(m_ModelName, Length);
        event.File.write(reinterpret_cast<const char*>(&m_iShaderPassIndex), sizeof(_uint));
        _float4x4 WorldMatrix;
        XMStoreFloat4x4(&WorldMatrix, m_pTransformCom->Get_WorldMatrix());
        event.File.write(reinterpret_cast<const _char*>(&WorldMatrix), sizeof(_float4x4));
        });


#endif
    m_pDiffuseTextureCom.resize(m_pModelCom->Get_NumMesh());
    m_pNormalTextureCom.resize(m_pModelCom->Get_NumMesh());

    m_iSelectedDiffuseIndex = new _uint[m_pModelCom->Get_NumMesh()];
    m_iSelectedNormalIndex = new _uint[m_pModelCom->Get_NumMesh()];
    return S_OK;
}

void CMapObject::Priority_Update(_float fTimeDelta)
{
    /*if(m_pModelCom->Is_Picked(XMLoadFloat4(m_pGameInstance->Get_CamPos()), m_pGameInstance->Get_MouseDir(), &fDistance))
        m_pGameInstance->Publish(ENUM_CLASS(LEVEL::MAP),TEXT("Model_Pick"))*/
}

void CMapObject::Update(_float fTimeDelta)
{
    //if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
    if(m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB)== KEYSTATE::DOWN)
    {
        //여기에 클릭 최적화 하려면 프러스텀 컬링까지.

        _float fDistance = {};
        //월드도 바꿔야함.
        _vector RayPos = XMVector3TransformCoord(XMLoadFloat3(&CLevel_Map::m_vWorldPos), m_pTransformCom->Get_WorldMatrix_Inv());
        _vector RayDir = XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&CLevel_Map::m_vWorldDir), m_pTransformCom->Get_WorldMatrix_Inv()));
        if (m_pModelCom->Is_Picked(RayPos,RayDir, &fDistance))
        {
            MAP_PICK event(this, fDistance);

            m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("ObjectPick"), event);
        }
    }
}

void CMapObject::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CMapObject::Render()
{
    Bind_Resources();

    for (_uint i = 0; i < m_pModelCom->Get_NumMesh(); ++i)
    {
        if (m_TexMode)
        {
            if (m_pDiffuseTextureCom[i])
                m_pDiffuseTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture");
            if (m_pNormalTextureCom[i])
                m_pNormalTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_NormalTexture");
        }
        else
        {
            m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
            m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL);
        }

        //이니셜라이즈 할 때 Bind_Materials 메쉬별로 한 번씩 돌려서 텍스쳐 없는 메쉬만 내가 직접 넣어서 저장할 수 있게?
        
        m_pShaderCom->Begin(m_iShaderPassIndex);

        m_pModelCom->Render(i);
    }
}

void CMapObject::Render_Shadow()
{

}

void CMapObject::Set_ImGuiOption()
{

    _matrix Scale, Rotation, Translation;

    //사이즈가 점점 작아짐. 나중에 수정할것.
    //m_pScale에다가 저장한 뒤 버튼 누르면 적용되게 하면 안바뀔듯.
    ImGui::Text("Size");
    {
        ImGui::PushItemWidth(90.0f);
        ImGui::InputFloat("R", &m_vNewScale.x, 0.1f, 0.1f); ImGui::SameLine();
        ImGui::InputFloat("U", &m_vNewScale.y, 0.1f, 0.1f); ImGui::SameLine();
        ImGui::InputFloat("L", &m_vNewScale.z, 0.1f, 0.1f);

    }

    ImGui::Text("Turn_Quaternion");
    {
        //로테이션이 계속 업데이트 되어서 값이 초기화됨.
        ImGui::PushItemWidth(90.0f);
        //_float4 DegreeRotation = m_pRotation[m_iPickedInstance];

        //디그리 각도로 0도에서 360도까지.

        ImGui::InputFloat("Yaw", &m_vNewRotation.x, 0.1f, 0.1f); ImGui::SameLine();
        ImGui::InputFloat("Picth", &m_vNewRotation.y, 0.1f, 0.1f); ImGui::SameLine();
        ImGui::InputFloat("Roll", &m_vNewRotation.z, 0.1f, 0.1f);

    }

    ImGui::Text("Position");
    //_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    {
        ImGui::PushItemWidth(90.0f);
        ImGui::InputFloat("X", &m_vNewTranslation.x, 1.f, 1.f); ImGui::SameLine();
        ImGui::InputFloat("Y", &m_vNewTranslation.y, 1.f, 1.f); ImGui::SameLine();
        ImGui::InputFloat("Z", &m_vNewTranslation.z, 1.f, 1.f);


    }
    ImGui::PopItemWidth();


    if (ImGui::Button("OK"))
    {
        Scale = XMMatrixScalingFromVector(XMLoadFloat3(&m_vNewScale));
        Rotation = XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&m_vNewRotation)));
        Translation = XMMatrixTranslationFromVector(XMVectorSetW(XMLoadFloat3(&m_vNewTranslation), 1.f));
        m_vScale = m_vNewScale;
        m_vRotation = m_vNewRotation;
        m_vTranslation = m_vNewTranslation;
        _matrix PickedMatrix = Scale * Rotation * Translation;
        m_pTransformCom->Set_WorldMatrix(PickedMatrix);
    }

    ImGuiID ShaderId = ImGui::GetID("ShaderPass");
    ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
    ImGui::Text("ShaderPass");

    for (_uint i = 0; i < m_pShaderCom->Get_PassCount(); ++i)
    {
        if (ImGui::Button(m_pShaderCom->Get_PassName(i))) {
            m_iShaderPassIndex = i;
        }
    }
    ImGui::EndChildFrame();

    ImGui::SameLine();
    ShaderId = ImGui::GetID("Test");
    ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
    ImGui::Text("LOD");
    _char LOD_Index[10] = {};

    for (_uint i = 0; i < 4; ++i)
    {
        if (m_pModelComArray[i] == nullptr)
            continue;

        sprintf_s(LOD_Index, "LOD%d", i);
        if (ImGui::Button(LOD_Index))
        {
            m_pModelCom = m_pModelComArray[i];
        }
    }
    ImGui::EndChildFrame();

    if (ImGui::Button("Set Texture"))
    {
        if (!m_IsTest)
        {
            m_DiffuseTextureName.clear();
            m_NormalTextureName.clear();
        }
        m_IsTest = !m_IsTest;
    }

    ImGui::SameLine();
    ImGui::Checkbox("Custom Tex", &m_TexMode);

    if (ImGui::Button("Destroy"))
        m_isActivate = false;

    if (m_IsTest)
    {

        IGFD::FileDialogConfig config;

        //C:\Users\dnheu\source\repos
        config.path = "C:/Users/dnheu/source/repos";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

        ImGuiFileDialog::Instance()->OpenDialog("Texture File Load", "Import File", nullptr, config);
        //ImGuiFileDialog::Instance()->OpenDialog("Texture File Load", "Import File", ".txt", config);

        if (ImGuiFileDialog::Instance()->Display("Texture File Load")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                _string strFolderPath = ImGuiFileDialog::Instance()->GetCurrentPath();
                for (const auto& entry : filesystem::recursive_directory_iterator(strFolderPath)) {
                    if (entry.is_regular_file()) {
                        if (entry.path().string().find("_D_") != std::string::npos)
                        {
                            if (entry.path().extension() == ".png") {
                                {
                                    string fileName = entry.path().filename().string();
                                    //m_DiffuseTextureName.push_back(fileName);
                                    m_DiffuseTextureName.push_back(entry.path().string());
                                }
                            }
                        }
                        else if (entry.path().string().find("_N_") != std::string::npos)
                        {
                            if (entry.path().extension() == ".png") {
                                {
                                    string fileName = entry.path().filename().string();
                                    //m_NormalTextureName.push_back(fileName);
                                    m_NormalTextureName.push_back(entry.path().string());
                                }
                            }
                        }
                    }
                }
                m_IsLoaded = true;
                m_IsTest = !m_IsTest;
                ImGuiFileDialog::Instance()->Close();
            }
        }
    }

    if (m_IsLoaded)
    {
        ImGui::Begin("Texture Change");

        _char LOD_Index[10] = {};

        if (ImGui::BeginCombo("Meshes", "?"))
        {
            for (_uint i = 0; i < m_pModelCom->Get_NumMesh(); ++i)
            {
                sprintf_s(LOD_Index, "Mesh : %d", i);
                if (ImGui::Selectable(LOD_Index))
                {
                    m_iSelectedMesh = i;
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Diffuse", m_SelectedDiffuse.c_str()))
        {
            for (_uint i = 0; i < m_DiffuseTextureName.size(); ++i)
            {
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(m_DiffuseTextureName[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);
                if (ImGui::Selectable(FileName))
                {
                    ImGui::SetItemDefaultFocus();
                    m_SelectedDiffuse = m_DiffuseTextureName[i];


                    _wstring Test(m_SelectedDiffuse.begin(), m_SelectedDiffuse.end());
                    if (m_pDiffuseTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pDiffuseTextureCom[m_iSelectedMesh]);

                    m_pDiffuseTextureCom[m_iSelectedMesh] = (CTexture::Create(m_pDevice, m_pContext, Test.c_str(), 1));
                    m_iSelectedDiffuseIndex[m_iSelectedMesh] = i;
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Normal", m_SelectedNormal.c_str()))
        {

            for (_uint i = 0; i < m_NormalTextureName.size(); ++i)
            {
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(m_NormalTextureName[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                if (ImGui::Selectable(FileName))
                {
                    ImGui::SetItemDefaultFocus();
                    m_SelectedNormal = m_NormalTextureName[i].c_str();

                    //W스트링으로 바꿔.
                    _wstring Test(m_SelectedNormal.begin(), m_SelectedNormal.end());
                    if (m_pNormalTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pNormalTextureCom[m_iSelectedMesh]);

                    m_pNormalTextureCom[m_iSelectedMesh] = (CTexture::Create(m_pDevice, m_pContext, Test.c_str(), 1));
                    m_iSelectedNormalIndex[m_iSelectedMesh] = i;
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Make_Json"))
            m_MakeJson = !m_MakeJson;

        if (m_MakeJson)
            Export_MaterialData();

        ImGui::Begin("Textures", nullptr,ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

        if (m_pDiffuseTextureCom[m_iSelectedMesh])
            ImGui::Image((ImTextureID)m_pDiffuseTextureCom[m_iSelectedMesh]->Get_SRV(0), ImVec2(256, 256));

        if (m_pNormalTextureCom[m_iSelectedMesh])
            ImGui::Image((ImTextureID)m_pNormalTextureCom[m_iSelectedMesh]->Get_SRV(0), ImVec2(256, 256));

        ImGui::End();

        ImGui::End();
    }
    //LOD가 총 4단계로 나뉘어져있는데 이거 어떻게 할 건지 생각.
    //제일 간단한 방법 => 쿼드트리에서 크기에 비례해서 렌더할 때 모델 갈아끼기.
    //=> 인스턴싱한 메쉬들은 각 매트릭스마다 비교해서 메쉬 뭐 쓸지 결정해야할듯?

}


HRESULT CMapObject::Ready_Component(void* pArg)
{
    //이 부분 나중에 .Dat로드할때 데이터화 시켜서 로드 시킬것.
    _tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
    _tchar Name[MAX_PATH] = {};
    MultiByteToWideChar(CP_ACP, 0, m_ModelName, -1, Name, strlen(m_ModelName));
    lstrcat(Model, Name);

    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), Model,
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
        return E_FAIL;

    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), Model,
        TEXT("Com_Model_LOD0"), reinterpret_cast<CComponent**>(&m_pModelComArray[0]), nullptr)))
        return E_FAIL;

    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Model_Wolf"),
        TEXT("Com_Model_LOD1"), reinterpret_cast<CComponent**>(&m_pModelComArray[1]), nullptr)))
        return E_FAIL;

    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), Model,
        TEXT("Com_Model_LOD2"), reinterpret_cast<CComponent**>(&m_pModelComArray[2]), nullptr)))
        return E_FAIL;

    /*if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), Model,
        TEXT("Com_Model_LOD3"), reinterpret_cast<CComponent**>(&m_pModelComArray[3]), nullptr)))
        return E_FAIL;*/

    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_NonAnimMesh"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    /*CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
    RigidbodyDesc.eShape = SHAPE::MESH;
    XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
    RigidbodyDesc.eType = EMotionType::Static;
    RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
    RigidbodyDesc.pModel = m_pModelCom;

    Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
        TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);*/


    return S_OK;
}

void CMapObject::Bind_Resources()
{
    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

void CMapObject::Export_MaterialData()
{

    //최종 폴더 경로.

    IGFD::FileDialogConfig config;

    config.path = "../../Client/Resource/";
    config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;
    //config.flags = ImGuiFileDialogFlags_SelectDirectory;

    ImGuiFileDialog::Instance()->OpenDialog("why", "Test", ".dat", config);

    if (ImGuiFileDialog::Instance()->Display("why")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFolderName = ImGuiFileDialog::Instance()->GetCurrentPath();
            _string strTexturePath = {};
            strFolderName += "/Mat/";
            strTexturePath = strFolderName;
            strFolderName += m_ModelName;
            strFolderName += ".json";

            ofstream File(strFolderName);

#pragma region 텍스쳐 복사 및 Json 저장
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
                _char FileExt[MAX_PATH] = {};

                _splitpath_s(m_DiffuseTextureName[m_iSelectedDiffuseIndex[i]].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, DiffuseFileName, MAX_PATH, FileExt, MAX_PATH);
                _splitpath_s(m_NormalTextureName[m_iSelectedNormalIndex[i]].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, NormalFileName, MAX_PATH, FileExt, MAX_PATH);

                json MaterialData;

                json DiffuseData;

                DiffuseData["TextureCnt"] = 1;
                DiffuseData["FileName"] = json::array();

                _char DiffuseName[MAX_PATH] = {};
                strcat_s(DiffuseName, DiffuseFileName);
                strcat_s(DiffuseName, FileExt);

                DiffuseData["FileName"].push_back(DiffuseName);

                MaterialData["Diffuse"] = DiffuseData;

                json NormalData;

                NormalData["TextureCnt"] = 1;
                NormalData["FileName"] = json::array();

                _char NormalName[MAX_PATH] = {};
                strcat_s(NormalName, NormalFileName);
                strcat_s(NormalName, FileExt);

                NormalData["FileName"].push_back(NormalName);

                MaterialData["Normal"] = NormalData;

                Totaljson["Materials"].push_back(MaterialData);


                filesystem::copy_file(m_DiffuseTextureName[i], strTexturePath + DiffuseFileName + FileExt, filesystem::copy_options::overwrite_existing);
                filesystem::copy_file(m_NormalTextureName[i], strTexturePath + NormalFileName + FileExt, filesystem::copy_options::overwrite_existing);
            }
            File << Totaljson.dump(4);
            File.close();
            m_MakeJson = !m_MakeJson;
#pragma endregion
        }
    }
}


CMapObject* CMapObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMapObject* pInstance = new CMapObject(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : MapObject");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CMapObject::Clone(void* pArg)
{
    CMapObject* pInstance = new CMapObject(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : MapObject (Clone)");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CMapObject::Free()
{
    __super::Free();
    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pRigidbodyCom);
    Safe_Delete(m_iSelectedDiffuseIndex);
    Safe_Delete(m_iSelectedNormalIndex);
        
    for (auto& pModel : m_pModelComArray)
        Safe_Release(pModel);

    for (auto& pTexture : m_pDiffuseTextureCom)
        if (pTexture)
            Safe_Release(pTexture);
    
    for (auto& pTexture : m_pNormalTextureCom)
        if (pTexture)
            Safe_Release(pTexture);


    m_pGameInstance->Unscribe();
}