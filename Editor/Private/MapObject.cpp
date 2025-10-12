#include"Editorpch.h"
#include "MapObject.h"
#include"Model_Instance.h"
#include"Mesh_Instance.h"
#include"Event_Level.h"
#include "AnimationActor.h"

CMapObject::CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CGameObject(pDevice,pContext)
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

    if (FAILED(Ready_Component(pArg)))
        return E_FAIL;

    m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

    _vector vScale, vRotation, vTranslation;

    XMMatrixDecompose(&vScale, &vRotation, &vTranslation, m_pTransformCom->Get_WorldMatrix());

    XMStoreFloat3(&m_vScale, vScale);
    XMStoreFloat3(&m_vTranslation, vTranslation);
    m_vNewScale = m_vScale;
    m_vRotation = m_vNewRotation = _float3(0.f, 0.f, 0.f);
    m_vTranslation = m_vNewTranslation;
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
        _uint Length = strlen(m_ModelName);
        event.File.write(reinterpret_cast<const char*>(&Length), sizeof(_uint));
        event.File.write(m_ModelName, Length);
        event.File.write(reinterpret_cast<const char*>(&m_iShaderPassIndex), sizeof(_uint));
        _float4x4 WorldMatrix;
        XMStoreFloat4x4(&WorldMatrix, m_pTransformCom->Get_WorldMatrix());
        event.File.write(reinterpret_cast<const _char*>(&WorldMatrix), sizeof(_float4x4));
        });


#endif
    return S_OK;
}

void CMapObject::Priority_Update(_float fTimeDelta)
{

}

void CMapObject::Update(_float fTimeDelta)
{
    if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
    {
        MAP_PICK event(this);
        m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("ObjectPick"), event);
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
        m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture",i,TEXTURETYPE::DIFFUSE);
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

        ImGui::InputFloat("Yaw",    &m_vNewRotation.x, 0.1f, 0.1f); ImGui::SameLine();
        ImGui::InputFloat("Picth",  &m_vNewRotation.y, 0.1f, 0.1f); ImGui::SameLine();
        ImGui::InputFloat("Roll",   &m_vNewRotation.z, 0.1f, 0.1f);

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

    for (_uint i = 0; i<m_pShaderCom->Get_PassCount(); ++i)
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
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom),nullptr)))
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

    return S_OK;
}

void CMapObject::Bind_Resources()
{
    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
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

    for (auto& pModel : m_pModelComArray)
        Safe_Release(pModel);

    m_pGameInstance->Unscribe();
}