#include"Editorpch.h"
#include "Edit_MapObject_Sonora.h"
#include"Event_Level.h"
#include "AnimationActor.h"
#include"Level_Map.h"
#include"Map_Interface.h"

CEdit_MapObject_Sonora::CEdit_MapObject_Sonora(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CEdit_MapObject(pDevice, pContext)
{
}

CEdit_MapObject_Sonora::CEdit_MapObject_Sonora(const CEdit_MapObject_Sonora& Prototype)
    :CEdit_MapObject(Prototype)
{
}

HRESULT CEdit_MapObject_Sonora::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEdit_MapObject_Sonora::Initialize_Clone(void* pArg)
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
    m_eObjectType = pDesc->eObjectType;
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


    m_iNumObject = CEdit_MapObject_Sonora::g_iNumObjects++;
    return S_OK;
}

void CEdit_MapObject_Sonora::Priority_Update(_float fTimeDelta)
{
}

void CEdit_MapObject_Sonora::Update(_float fTimeDelta)
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

void CEdit_MapObject_Sonora::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_MapObject_Sonora::Render()
{
    //�Ⱥ��̴� �� ����
    Bind_Resources();

    for (_uint i = 0; i < m_pModelComArray[m_iLODIndex]->Get_NumMesh(); ++i)
    {
        _bool HasNormal = { true };
        if (m_TexMode)
        {
            
            if (m_pDiffuseTextureCom[i])
                m_pDiffuseTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture",0);

            if (m_pNormalTextureCom[i])
                if(FAILED(m_pNormalTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_NormalTexture")))
                    HasNormal = false;

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

            if (FAILED(m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
                HasNormal = false;

            if (FAILED(m_pModelComArray[m_iLODIndex]->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK)))
                m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
        }
        m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));

        m_pShaderCom->Begin(m_iShaderPassIndex);

        m_pModelComArray[m_iLODIndex]->Render(i);
    }
}

void CEdit_MapObject_Sonora::Render_Shadow()
{

}

void CEdit_MapObject_Sonora::Set_ImGuiOption()
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


HRESULT CEdit_MapObject_Sonora::Ready_Component(void* pArg)
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

void CEdit_MapObject_Sonora::Bind_Resources()
{
    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

void CEdit_MapObject_Sonora::Add_Child(CEdit_MapObject_Sonora* pObject)
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

void CEdit_MapObject_Sonora::Quit_Child(CEdit_MapObject_Sonora* pObject)
{
    m_ChildObjects.remove(pObject);
    if (m_ChildObjects.empty())
        m_IsParent = false;
}

void CEdit_MapObject_Sonora::Make_ChildLocalMatrix(_fmatrix ParentMatrix)
{
    XMStoreFloat4x4(&m_ChildLocalMat, m_pTransformCom->Get_WorldMatrix() * XMMatrixInverse(nullptr, ParentMatrix));
    _matrix NewChildWolrd = XMLoadFloat4x4(&m_ChildLocalMat) * ParentMatrix;
    m_pTransformCom->Set_WorldMatrix(NewChildWolrd);
}

CEdit_MapObject_Sonora* CEdit_MapObject_Sonora::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEdit_MapObject_Sonora* pInstance = new CEdit_MapObject_Sonora(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : MapObject");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEdit_MapObject_Sonora::Clone(void* pArg)
{
    CEdit_MapObject_Sonora* pInstance = new CEdit_MapObject_Sonora(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : MapObject (Clone)");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEdit_MapObject_Sonora::Free()
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