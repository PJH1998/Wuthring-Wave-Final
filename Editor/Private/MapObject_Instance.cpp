#include"Editorpch.h"
#include "MapObject_Instance.h"
#include"Model_Instance.h"
#include"Mesh_Instance.h"
#include"Event_Level.h"
#include "AnimationActor.h"

CMapObject_Instance::CMapObject_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CGameObject(pDevice,pContext)
{
}

CMapObject_Instance::CMapObject_Instance(const CMapObject_Instance& Prototype)
    :CGameObject(Prototype)
{
}


HRESULT CMapObject_Instance::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CMapObject_Instance::Initialize_Clone(void* pArg)
{
    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Component(pArg)))
        return E_FAIL;

    m_iShaderPassIndex = 0;
    MODELTYPE::MAP;
    return S_OK;
}

void CMapObject_Instance::Priority_Update(_float fTimeDelta)
{

}

void CMapObject_Instance::Update(_float fTimeDelta)
{
    if (m_pGameInstance->Get_DIKeyState(DIK_G) == KEYSTATE::DOWN)
    {
        _float fDistance = {};

        //if (m_pModelCom->Is_Picked(XMLoadFloat4(m_pGameInstance->Get_CamPos()), m_pGameInstance->Get_MouseDir(), &fDistance))
        {
            MAP_PICK event(this, fDistance);
            m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("ObjectPick"), event);
        }
    }
}

void CMapObject_Instance::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CMapObject_Instance::Render()
{
    Bind_Resources();

    //m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    

    for (_uint i = 0; i < m_pModelCom->Get_NumMesh(); ++i)
    {
        m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture",i,TEXTURETYPE::DIFFUSE);
        m_pShaderCom->Begin(m_iShaderPassIndex);

        m_pModelCom->Render(i);
    }
}

void CMapObject_Instance::Render_Shadow()
{

}

void CMapObject_Instance::Set_ImGuiOption()
{
    ImGuiID PickID = ImGui::GetID("MapPick");
    char Pick_buffer[30];
    sprintf_s(Pick_buffer, "%d", m_iPickedInstance);
    strcat_s(Pick_buffer, " : Picked");
    ImGui::Text(Pick_buffer);

    ImGui::BeginChildFrame(PickID, ImVec2(100, 200));

    for (_uint i = 0; i < m_iNumInstance; ++i)
    {
        char buffer[10];
        sprintf_s(buffer, "%d", i);
        if (ImGui::Button(buffer))
            m_iPickedInstance = i;
    }

    ImGui::EndChildFrame();

    _matrix PickedMatrix = XMLoadFloat4x4(&m_pInstanceMatrix[m_iPickedInstance]);
    _vector vScale, vRotation, vTranslation;
    _matrix Scale, Rotation, Translation;
    XMMatrixDecompose(&vScale, &vRotation, &vTranslation, PickedMatrix);

    //사이즈가 점점 작아짐. 나중에 수정할것.
    //m_pScale에다가 저장한 뒤 버튼 누르면 적용되게 하면 안바뀔듯.
    ImGui::Text("Size");
    {
        ImGui::PushItemWidth(90.0f);
        ImGui::InputFloat("R", &vScale.m128_f32[0], 0.1f, 0.1f); ImGui::SameLine();
        ImGui::InputFloat("U", &vScale.m128_f32[1], 0.1f, 0.1f); ImGui::SameLine();
        ImGui::InputFloat("L", &vScale.m128_f32[2], 0.1f, 0.1f);

        Scale = XMMatrixScalingFromVector(vScale);
    }


    ImGui::Text("Turn_Quaternion");
    {
        //로테이션이 계속 업데이트 되어서 값이 초기화됨.
        ImGui::PushItemWidth(90.0f);
        //_float3 DegreeRotation = _float3(XMConvertToDegrees(m_pRotation[m_iPickedInstance].x), XMConvertToDegrees(m_pRotation[m_iPickedInstance].y), XMConvertToDegrees(m_pRotation[m_iPickedInstance].z));
        _float4 DegreeRotation = m_pRotation[m_iPickedInstance];
        
        //디그리 각도로 0도에서 360도까지.

        ImGui::InputFloat("Yaw",    &DegreeRotation.x, 0.1f, 0.1f); ImGui::SameLine();
        ImGui::InputFloat("Picth",  &DegreeRotation.y, 0.1f, 0.1f); ImGui::SameLine();
        ImGui::InputFloat("Roll",   &DegreeRotation.z, 0.1f, 0.1f);

        m_pRotation[m_iPickedInstance] = DegreeRotation;
        DegreeRotation = _float4(XMConvertToRadians(DegreeRotation.x), XMConvertToRadians(DegreeRotation.y), XMConvertToRadians(DegreeRotation.z), 0.f);
        Rotation = XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat4(&DegreeRotation)));
    }

    ImGui::Text("Position");
    _vector vPos = PickedMatrix.r[3];
    {
        ImGui::PushItemWidth(90.0f);
        ImGui::InputFloat("X", &vTranslation.m128_f32[0], 1.f, 1.f); ImGui::SameLine();
        ImGui::InputFloat("Y", &vTranslation.m128_f32[1], 1.f, 1.f); ImGui::SameLine();
        ImGui::InputFloat("Z", &vTranslation.m128_f32[2], 1.f, 1.f);

        Translation = XMMatrixTranslationFromVector(vTranslation);
    }
    ImGui::PopItemWidth();

    PickedMatrix = Scale * Rotation * Translation;
    
    XMStoreFloat4x4(&m_pInstanceMatrix[m_iPickedInstance], PickedMatrix);

    if (ImGui::Button("OK"))
        m_pModelCom->Change_InstanceInfo(m_iPickedInstance, PickedMatrix);

    ImGuiID ShaderId = ImGui::GetID("ShaderPass");
    ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
    for (_uint i = 0; i<m_pShaderCom->Get_PassCount(); ++i)
    {
        if (ImGui::Button(m_pShaderCom->Get_PassName(i))) {
            m_iShaderPassIndex = i;
        }
    }
    ImGui::EndChildFrame();

    //LOD가 총 4단계로 나뉘어져있는데 이거 어떻게 할 건지 생각.
    //제일 간단한 방법 => 쿼드트리에서 크기에 비례해서 렌더할 때 모델 갈아끼기.
    //=> 인스턴싱한 메쉬들은 각 매트릭스마다 비교해서 메쉬 뭐 쓸지 결정해야할듯?

}

HRESULT CMapObject_Instance::Ready_Component(void* pArg)
{
    //이 부분 나중에 .Dat로드할때 데이터화 시켜서 로드 시킬것.
    //ifstream File();
    CMesh_Instance::MESH_INST_DESC Desc{};
    m_iNumInstance = Desc.iNumInstance = 2;
    _float4x4* pMatrix = new _float4x4[Desc.iNumInstance];
    _matrix TT =XMMatrixScalingFromVector(XMVectorSet(20.f,10.f,1.f,0.f)) * XMMatrixTranslationFromVector(XMVectorSet(10.f, 0.f, 0.f, 1.f));
    memcpy(&pMatrix[0], &TT, sizeof(_float4x4));
    TT = XMMatrixTranslationFromVector(XMVectorSet(-10.f, 30.f, 0.f, 1.f));
    memcpy(&pMatrix[1], &TT, sizeof(_float4x4));
    m_pInstanceMatrix = Desc.pTransformMatrix = pMatrix;

    _vector vScale, vRotation, vTranslation;
    m_pRotation = new _float4[m_iNumInstance];

    //회전값은 미리 저장
    for (_uint i = 0; i < m_iNumInstance; ++i)
    {
        XMStoreFloat4(&m_pRotation[i], XMVectorSet(0.f, 0.f, 0.f, 0.f));
    }

    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Model_Wolf_Instance"),
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), &Desc)))
        return E_FAIL;

    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_NonAnimMesh_Instance"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    return S_OK;
}

void CMapObject_Instance::Bind_Resources()
{
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

CMapObject_Instance* CMapObject_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMapObject_Instance* pInstance = new CMapObject_Instance(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : MapObject_Instance");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CMapObject_Instance::Clone(void* pArg)
{
    CMapObject_Instance* pInstance = new CMapObject_Instance(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : MapObject_Instance (Clone)");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CMapObject_Instance::Free()
{
    __super::Free();
    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);
    Safe_Delete_Array(m_pInstanceMatrix);
    Safe_Delete_Array(m_pRotation);
}