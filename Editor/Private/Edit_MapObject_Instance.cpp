#include"Editorpch.h"
#include "Edit_MapObject_Instance.h"
#include"Model_Instance.h"
#include"Mesh_Instance.h"
#include"Event_Level.h"
#include "AnimationActor.h"

CEdit_MapObject_Instance::CEdit_MapObject_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CGameObject(pDevice,pContext)
{
}

CEdit_MapObject_Instance::CEdit_MapObject_Instance(const CEdit_MapObject_Instance& Prototype)
    :CGameObject(Prototype)
{
}


HRESULT CEdit_MapObject_Instance::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEdit_MapObject_Instance::Initialize_Clone(void* pArg)
{
    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Component(pArg)))
        return E_FAIL;

    m_iShaderPassIndex = 0;
    MODELTYPE::MAP;
    return S_OK;
}

void CEdit_MapObject_Instance::Priority_Update(_float fTimeDelta)
{

}

void CEdit_MapObject_Instance::Update(_float fTimeDelta)
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

void CEdit_MapObject_Instance::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_MapObject_Instance::Render()
{
    Bind_Resources();

    for (_uint i = 0; i < m_pModelCom->Get_NumMesh(); ++i)
    {
        m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture",i,TEXTURETYPE::DIFFUSE);
        m_pShaderCom->Begin(m_iShaderPassIndex);

        m_pModelCom->Render(i);
    }
}

void CEdit_MapObject_Instance::Render_Shadow()
{

}

void CEdit_MapObject_Instance::Set_ImGuiOption()
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

    //?ъ씠利덇? ?먯젏 ?묒븘吏? ?섏쨷???섏젙?좉쾬.
    //m_pScale?먮떎媛 ??ν븳 ??踰꾪듉 ?꾨Ⅴ硫??곸슜?섍쾶 ?섎㈃ ?덈컮?붾벏.
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
        //濡쒗뀒?댁뀡??怨꾩냽 ?낅뜲?댄듃 ?섏뼱??媛믪씠 珥덇린?붾맖.
        ImGui::PushItemWidth(90.0f);
        //_float3 DegreeRotation = _float3(XMConvertToDegrees(m_pRotation[m_iPickedInstance].x), XMConvertToDegrees(m_pRotation[m_iPickedInstance].y), XMConvertToDegrees(m_pRotation[m_iPickedInstance].z));
        _float4 DegreeRotation = m_pRotation[m_iPickedInstance];
        
        //?붽렇由?媛곷룄濡?0?꾩뿉??360?꾧퉴吏.

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

    //LOD媛 珥?4?④퀎濡??섎돇?댁졇?덈뒗???닿굅 ?대뼸寃???嫄댁? ?앷컖.
    //?쒖씪 媛꾨떒??諛⑸쾿 => 荑쇰뱶?몃━?먯꽌 ?ш린??鍮꾨??댁꽌 ?뚮뜑????紐⑤뜽 媛덉븘?쇨린.
    //=> ?몄뒪?댁떛??硫붿돩?ㅼ? 媛?留ㅽ듃由?뒪留덈떎 鍮꾧탳?댁꽌 硫붿돩 萸??몄? 寃곗젙?댁빞?좊벏?

}

HRESULT CEdit_MapObject_Instance::Ready_Component(void* pArg)
{
    MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

    CMesh_Instance::MESH_INST_DESC Desc{};
    Desc.iNumInstance= m_iNumInstance = pDesc->iNumInstance;
    Desc.pTransformMatrix = pDesc->WorldMatrix;
    
    _vector vScale, vRotation, vTranslation;
    m_pRotation = new _float4[m_iNumInstance];

    //?뚯쟾媛믪? 誘몃━ ???
    for (_uint i = 0; i < m_iNumInstance; ++i)
    {
        XMStoreFloat4(&m_pRotation[i], XMVectorSet(0.f, 0.f, 0.f, 0.f));
    }

    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), StringToWString(pDesc->ModelName),
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), &Desc)))
        return E_FAIL;

    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_NonAnimMesh_Instance"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    return S_OK;
}

void CEdit_MapObject_Instance::Bind_Resources()
{
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

CEdit_MapObject_Instance* CEdit_MapObject_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEdit_MapObject_Instance* pInstance = new CEdit_MapObject_Instance(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : MapObject_Instance");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEdit_MapObject_Instance::Clone(void* pArg)
{
    CEdit_MapObject_Instance* pInstance = new CEdit_MapObject_Instance(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : MapObject_Instance (Clone)");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEdit_MapObject_Instance::Free()
{
    __super::Free();
    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);
    Safe_Delete_Array(m_pInstanceMatrix);
    Safe_Delete_Array(m_pRotation);
}