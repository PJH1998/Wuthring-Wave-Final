#include"Editorpch.h"
#include "Edit_MapObject.h"
#include"Model_Instance.h"
#include"Mesh_Instance.h"
#include"Event_Level.h"
#include "AnimationActor.h"
#include"Level_Map.h"

_uint CEdit_MapObject::g_iNumObjects = {};
CEdit_MapObject::CEdit_MapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    //:CStaticObject(pDevice, pContext)
    :CGameObject(pDevice, pContext)
{
}

CEdit_MapObject::CEdit_MapObject(const CEdit_MapObject& Prototype)
    //:CStaticObject(Prototype)
    :CGameObject(Prototype)
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

    strcpy_s(m_ModelName, pDesc->ModelName);
    m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

    if (FAILED(Ready_Component(pArg)))
        return E_FAIL;
    //m_iNumLOD = m_pModelComArray.size()-1;

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

        //경로 지정할 때 상위 폴더엔 뒤에 LOD 빼고. 폴더를 지정. 그리고 그 안에 있는 폴더 하위 1개 돌면서 .dat들 읽고 객체 안에 넣기?
        
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

    m_iNumObject = CEdit_MapObject::g_iNumObjects++;
    return S_OK;
}

void CEdit_MapObject::Priority_Update(_float fTimeDelta)
{

}

void CEdit_MapObject::Update(_float fTimeDelta)
{
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

void CEdit_MapObject::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_MapObject::Render()
//void CEdit_MapObject::Render(_uint iLOD)
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
            if (m_pMaskTextureCom[i])
                m_pMaskTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_MaskTexture");
            else
                m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
            if (m_pMaskDiffuseTextureCom[i])
                m_pMaskDiffuseTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_MaskDiffuse");
        }
        else
        {
            _uint FailedCnt = {};
            if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE)))
                FailedCnt++;
            if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
                FailedCnt++;

            if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK)))
                m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);

            if (FailedCnt > 1)
                m_iShaderPassIndex = 1;
            else
                m_iShaderPassIndex = 0;
        }

        //이니셜라이즈 할 때 Bind_Materials 메쉬별로 한 번씩 돌려서 텍스쳐 없는 메쉬만 내가 직접 넣어서 저장할 수 있게?
        
        m_pShaderCom->Begin(m_iShaderPassIndex);

        m_pModelCom->Render(i);
    }
}

void CEdit_MapObject::Render_Shadow()
{

}

void CEdit_MapObject::Set_ImGuiOption()
{

    ImGui::Text(m_ModelName);


    About_Parent();

    About_Transform();

    ImGuiID ShaderId = ImGui::GetID("ShaderPass");
    ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
    ImGui::Text("ShaderPass");

    for (_uint i = 0; i < m_pShaderCom->Get_PassCount(); ++i)
    {
        if (!strcmp("DebugRender", m_pShaderCom->Get_PassName(i)))
            continue;

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
    //LOD가 총 4단계로 나뉘어져있는데 이거 어떻게 할 건지 생각.
    //제일 간단한 방법 => 쿼드트리에서 크기에 비례해서 렌더할 때 모델 갈아끼기.
    //=> 인스턴싱한 메쉬들은 각 매트릭스마다 비교해서 메쉬 뭐 쓸지 결정해야할듯?

}


HRESULT CEdit_MapObject::Ready_Component(void* pArg)
{
    m_pGameInstance->Wait_Thread_End();

    //이 부분 나중에 .Dat로드할때 데이터화 시켜서 로드 시킬것.
    _tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
    _tchar Name[MAX_PATH] = {};
    MultiByteToWideChar(CP_ACP, 0, m_ModelName, -1, Name, strlen(m_ModelName));
    lstrcat(Model, Name);
    m_pModelComArray.resize(4);
    /*if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), Model,
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
        return E_FAIL;*/


    for (_uint i = 0; i < 4; ++i)
    {
        //m_pGameInstance->Add_Work([&,Index = i, Name = Model]() {
        //    CModel* pModel = nullptr;
        //    _wstring ModelCom = Name;
        //    ModelCom.pop_back();
        //    ModelCom += to_wstring(Index);

        //    _char ModelName[MAX_PATH] = {};
        //    sprintf_s(ModelName, "Com_Model%d", Index);
        //    if (FAILED(Add_Component(ENUM_CLASS(LEVEL::MAP), ModelCom,
        //        //StringToWString(ModelName), reinterpret_cast<CComponent**>(&pModel), nullptr)))
        //        StringToWString(ModelName), reinterpret_cast<CComponent**>(&m_pModelComArray[Index]), nullptr)))
        //        CRASH("FAILED");
        //    //m_pModelComArray.push_back(pModel);
        //    });
        CModel* pModel = nullptr;
        _wstring ModelCom = Model;
        ModelCom.pop_back();
        ModelCom += to_wstring(i);

        _char ModelName[MAX_PATH] = {};
        sprintf_s(ModelName, "Com_Model%d", i);
        if (FAILED(Add_Component(ENUM_CLASS(LEVEL::MAP), ModelCom,
            //StringToWString(ModelName), reinterpret_cast<CComponent**>(&pModel), nullptr)))
            StringToWString(ModelName), reinterpret_cast<CComponent**>(&m_pModelComArray[i]), nullptr)))
            CRASH("FAILED");
        //m_pModelComArray.push_back(pModel);
    }
    m_pGameInstance->Wait_Thread_End();

    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_NonAnimMesh"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;
    
#pragma region 리지드바디
        //CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
        //RigidbodyDesc.eShape = SHAPE::MESH;
        //XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
        //RigidbodyDesc.eType = EMotionType::Static;
        //RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
        //RigidbodyDesc.pModel = m_pModelCom;

        //if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
        //    TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
        //    CRASH("FAILED");
#pragma endregion
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

    if (pObject->m_pParent)
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

    //최종 폴더 경로.

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

    //return;
    //_string SubstrLOD;
    //SubstrLOD = m_ModelName;
    //size_t SubStrPos = SubstrLOD.find("_LOD");

    //if (SubStrPos != std::string::npos)
    //    SubstrLOD = SubstrLOD.substr(0, SubStrPos);
    ////strcpy_s(ModelPath, "/"); +SubstrLOD + "/";
    //strcat_s(ModelPath, SubstrLOD.c_str());
    //strcat_s(ModelPath, "/");
    //strcat_s(ModelPath, m_ModelName);
    //strcat_s(ModelPath, "/");

    config1.path = string(ModelPath);
    config1.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;
    _char Text[32] = {};
    sprintf_s(Text,"Object %d###Select.dat", m_iNumObject);
    ImGuiFileDialog::Instance()->OpenDialog("Export Json", Text, ".dat", config1);

    if (ImGuiFileDialog::Instance()->Display("Export Json")) {
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
                _char MaskFileName[MAX_PATH] = {};
                _char MaskDiffuseFileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};

                _splitpath_s(m_SelectedDiffuseTexturePath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, DiffuseFileName, MAX_PATH, FileExt, MAX_PATH);
                _splitpath_s(m_SelectedNormalTexturePath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, NormalFileName, MAX_PATH, FileExt, MAX_PATH);
                _splitpath_s(m_SelectedMaskTexturePath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, MaskFileName, MAX_PATH, FileExt, MAX_PATH);
                _splitpath_s(m_SelectedMaskDiffusePath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, MaskDiffuseFileName, MAX_PATH, FileExt, MAX_PATH);

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

    //위치만 갖고오기?
    //부모만 회전해야할 때는 어떻게함?
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
            //MAP_CREATE event(m_ModelName, this);
            //m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Set_Parent"), event);
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
        }
        ImGui::EndChildFrame();

    }
}

void CEdit_MapObject::About_Transform()
{
    m_pGameInstance->Use_Gizmo(m_pTransformCom);

    _float vScale[3] = {};
    _float vRotation[3] = {};
    _float vTransfrom[3] = {};
    _matrix matrix = m_pTransformCom->Get_WorldMatrix();
    ImGuizmo::DecomposeMatrixToComponents(reinterpret_cast<_float*>(&matrix), vTransfrom, vRotation, vScale);

    ImGui::Text("Size");
    {
        ImGui::PushItemWidth(300.0f);
        ImGui::InputFloat3("Scale", vScale);
    }

    ImGui::Text("Turn_Quaternion");
    {
        //로테이션이 계속 업데이트 되어서 값이 초기화됨.
        ImGui::PushItemWidth(300.0f);
        //디그리 각도로 0도에서 360도까지.
        ImGui::InputFloat3("Rotation", vRotation);
    }

    ImGui::Text("Position");
    {
        ImGui::PushItemWidth(300.0f);
        ImGui::InputFloat3("Translation", vTransfrom);
    }

    ImGui::PopItemWidth();

    ImGuizmo::RecomposeMatrixFromComponents(vTransfrom, vRotation, vScale, reinterpret_cast<_float*>(&matrix));

    m_pTransformCom->Set_WorldMatrix(matrix);

    //m_pGameInstance->Add_Light()
    //자식은 간단하게 부모의 변화량만 추가로 하게 하면 될듯? 회전은 모르겠음..

    if (m_IsParent)
    {
        for (auto& pChild : m_ChildObjects)
            pChild->Child_UpdateMatrix(m_pTransformCom->Get_WorldMatrix(), XMVectorSetW(XMLoadFloat3(&m_vNewTranslation), 1.f), XMVectorSetW(XMLoadFloat3(&m_vNewTranslation) - XMLoadFloat3(&m_vTranslation), 1.f));
    }

    if (m_pParent)
        Make_ChildLocalMatrix(dynamic_cast<CTransform*>(m_pParent->Get_Component(TEXT("Com_Transform")))->Get_WorldMatrix());
    {

        //ImGui::Text("Size");
        //{
        //    ImGui::PushItemWidth(90.0f);
        //    ImGui::InputFloat("R", &m_vNewScale.x, 0.1f, 0.1f); ImGui::SameLine();
        //    ImGui::InputFloat("U", &m_vNewScale.y, 0.1f, 0.1f); ImGui::SameLine();
        //    ImGui::InputFloat("L", &m_vNewScale.z, 0.1f, 0.1f);

        //}

        //ImGui::Text("Turn_Quaternion");
        //{
        //    //로테이션이 계속 업데이트 되어서 값이 초기화됨.
        //    ImGui::PushItemWidth(90.0f);
        //    //_float4 DegreeRotation = m_pRotation[m_iPickedInstance];

        //    //디그리 각도로 0도에서 360도까지.

        //    ImGui::InputFloat("Yaw", &m_vNewRotation.x, 0.1f, 0.1f); ImGui::SameLine();
        //    ImGui::InputFloat("Picth", &m_vNewRotation.y, 0.1f, 0.1f); ImGui::SameLine();
        //    ImGui::InputFloat("Roll", &m_vNewRotation.z, 0.1f, 0.1f);

        //}

        //ImGui::Text("Position");
        ////_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
        //{
        //    ImGui::PushItemWidth(90.0f);
        //    ImGui::InputFloat("X", &m_vNewTranslation.x, 1.f, 1.f); ImGui::SameLine();
        //    ImGui::InputFloat("Y", &m_vNewTranslation.y, 1.f, 1.f); ImGui::SameLine();
        //    ImGui::InputFloat("Z", &m_vNewTranslation.z, 1.f, 1.f);


        //}
        //ImGui::PopItemWidth();


        //if (ImGui::Button("OK"))
        //{
        //    //자식은 간단하게 부모의 변화량만 추가로 하게 하면 될듯? 회전은 모르겠음..
        //    _matrix Scale, Rotation, Translation;
        //    _float3 DegreeRotation = _float3(XMConvertToRadians(m_vNewRotation.x), XMConvertToRadians(m_vNewRotation.y), XMConvertToRadians(m_vNewRotation.z));

        //    Scale = XMMatrixScalingFromVector(XMLoadFloat3(&m_vNewScale));
        //    Rotation = XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&DegreeRotation)));
        //    Translation = XMMatrixTranslationFromVector(XMVectorSetW(XMLoadFloat3(&m_vNewTranslation), 1.f));

        //    m_vScale = m_vNewScale;
        //    m_vRotation = m_vNewRotation;
        //    m_vTranslation = m_vNewTranslation;

        //    _matrix PickedMatrix = Scale * Rotation * Translation;
        //    if (m_IsParent)
        //    {
        //        for (auto& pChild : m_ChildObjects)
        //            pChild->Child_UpdateMatrix(PickedMatrix, XMVectorSetW(XMLoadFloat3(&m_vNewTranslation), 1.f), XMVectorSetW(XMLoadFloat3(&m_vNewTranslation) - XMLoadFloat3(&m_vTranslation), 1.f));
        //    }

        //    m_pTransformCom->Set_WorldMatrix(PickedMatrix);

        //    if (m_pParent)
        //        Make_ChildLocalMatrix(dynamic_cast<CTransform*>(m_pParent->Get_Component(TEXT("Com_Transform")))->Get_WorldMatrix());
        //}
    }

}

void CEdit_MapObject::About_Texture()
{
    if (m_IsCustomTexture)
    {
        IGFD::FileDialogConfig config;

        //config.path = "C:/Users/dnheu/source/repos";
        config.path = filesystem::current_path().parent_path().parent_path().parent_path().string();
        //그때그때 바꿔끼기.
        //config.path = "C:/Users/dnheu/Downloads/FModel/Output/Exports/Client/Content/Aki/Scene/Assets/Levels/LiNaXiTa/QiQiu/Common/Rock/Tex/";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

        _char Text[32] = {};
        sprintf_s(Text, "Object %d###Texture File Load", m_iNumObject);
        ImGuiFileDialog::Instance()->OpenDialog(Text, "Import File", nullptr, config);
        //ImGuiFileDialog::Instance()->OpenDialog(Text, "Import File", ".txt", config);

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
                                    //m_EntireDiffuseTextureName.push_back(fileName);
                                    m_EntireDiffuseTextureName.push_back(Filepath.string());
                                }
                            }
                        }
                        else if (Filepath.string().find("_N_") != std::string::npos || (Filepath.string().find("_N") != std::string::npos))
                        {
                            if (Filepath.extension() == ".png") {
                                {
                                    string fileName = Filepath.filename().string();
                                    //m_EntireNormalTextureName.push_back(fileName);
                                    m_EntireNormalTextureName.push_back(Filepath.string());
                                }
                            }
                        }
                        else if (Filepath.string().find("_MA_") != std::string::npos || (Filepath.string().find("_MA") != std::string::npos))
                        {
                            if (Filepath.extension() == ".png") {
                                {
                                    string fileName = Filepath.filename().string();
                                    //m_EntireNormalTextureName.push_back(fileName);
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

        //메쉬마다 공통되는 거 있으면 여기에 지정.
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

                    //W스트링으로 바꿔.
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

                    //W스트링으로 바꿔.
                    _wstring Test(m_SelectedMask.begin(), m_SelectedMask.end());
                    if (m_pMaskTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pMaskTextureCom[m_iSelectedMesh]);

                    m_pMaskTextureCom[m_iSelectedMesh] = (CTexture::Create(m_pDevice, m_pContext, Test.c_str(), 1));
                    m_SelectedMaskTexturePath[m_iSelectedMesh] = m_EntireMaskTextureName[i].c_str();

                    m_SelectedMaskTextureName[m_iSelectedMesh] = FileName;
                    m_iSelectedMaskIndex[m_iSelectedMesh] = i;
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("MaskDiffuse", m_SelectedMaskDiffuseName[m_iSelectedMesh].c_str()))
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

                    //W스트링으로 바꿔.
                    _wstring Test(m_SelectedMaskDiffuse.begin(), m_SelectedMaskDiffuse.end());
                    if (m_pMaskDiffuseTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pMaskDiffuseTextureCom[m_iSelectedMesh]);

                    m_pMaskDiffuseTextureCom[m_iSelectedMesh] = (CTexture::Create(m_pDevice, m_pContext, Test.c_str(), 1));
                    m_SelectedMaskDiffusePath[m_iSelectedMesh] = m_EntireDiffuseTextureName[i].c_str();

                    m_SelectedMaskDiffuseName[m_iSelectedMesh] = FileName;
                    m_iSelectedMaskIndex[m_iSelectedMesh] = i;
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Make_Json"))
            m_MakeJson = !m_MakeJson;

        if (m_MakeJson)
            Export_MaterialData();

        ImGui::Begin("Textures", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

        if (m_pDiffuseTextureCom[m_iSelectedMesh])
            ImGui::Image((ImTextureID)m_pDiffuseTextureCom[m_iSelectedMesh]->Get_SRV(0), ImVec2(256, 256));

        ImGui::SameLine();
        if (m_pNormalTextureCom[m_iSelectedMesh])
            ImGui::Image((ImTextureID)m_pNormalTextureCom[m_iSelectedMesh]->Get_SRV(0), ImVec2(256, 256));

        if (m_pMaskTextureCom[m_iSelectedMesh])
            ImGui::Image((ImTextureID)m_pMaskTextureCom[m_iSelectedMesh]->Get_SRV(0), ImVec2(256, 256));

        ImGui::SameLine();
        if (m_pMaskDiffuseTextureCom[m_iSelectedMesh])
            ImGui::Image((ImTextureID)m_pMaskDiffuseTextureCom[m_iSelectedMesh]->Get_SRV(0), ImVec2(256, 256));

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
    //Safe_Release(m_pModelCom);
    m_pModelCom = nullptr;
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pRigidbodyCom);
    Safe_Delete(m_iSelectedDiffuseIndex);
    Safe_Delete(m_iSelectedNormalIndex);
    Safe_Delete(m_iSelectedMaskIndex);
    Safe_Delete(m_iSelectedMaskDiffuseIndex);

    m_pParent = nullptr;

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

    m_pGameInstance->Unscribe();
}