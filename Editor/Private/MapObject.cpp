#include"Editorpch.h"
#include "MapObject.h"
#include"Model_Instance.h"
#include"Mesh_Instance.h"

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
    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Component(pArg)))
        return E_FAIL;

    m_iShaderPassIndex = 0;
    return S_OK;
}

void CMapObject::Priority_Update(_float fTimeDelta)
{

}

void CMapObject::Update(_float fTimeDelta)
{
    /*_vector vPos{}, vDir{};
    _float vDist{};
    if(m_pGameInstance->Get_DIKeyState(DIK_G)== KEYSTATE::DOWN)
    {
        if (m_pModelCom->Is_Picked(vPos, vDir, &vDist))
            int a = 0;
    }*/
}

void CMapObject::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CMapObject::Render()
{
    Bind_Resources();

    //m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
    
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
    _float3 vScale = m_pTransformCom->Get_Scaled();
    ImGui::InputFloat("Scale", &vScale.x);
    ImGui::InputFloat("Scale", &vScale.y);
    ImGui::InputFloat("Scale", &vScale.z);

    //개인폴 때 이렇게 했다가 크기가 점점 작아졌었음. 주의
    m_pTransformCom->Set_State(STATE::RIGHT, XMLoadFloat(&vScale.x));
    m_pTransformCom->Set_State(STATE::UP, XMLoadFloat(&vScale.y));
    m_pTransformCom->Set_State(STATE::LOOK, XMLoadFloat(&vScale.z));

    //LOD가 총 4단계로 나뉘어져있는데 이거 어떻게 할 건지 생각.
    //제일 간단한 방법 => 쿼드트리에서 크기에 비례해서 렌더할 때 모델 갈아끼기.
    //=> 인스턴싱한 메쉬들은 각 매트릭스마다 비교해서 메쉬 뭐 쓸지 결정해야할듯?
}

HRESULT CMapObject::Ready_Component(void* pArg)
{
    CMesh_Instance::MESH_INST_DESC Desc{};
    Desc.iNumInstance = 2;
    _float4x4* pMatrix = new _float4x4[Desc.iNumInstance];
    _matrix TT = XMMatrixTranslationFromVector(XMVectorSet(10.f, 0.f, 0.f, 1.f));
    memcpy(&pMatrix[0], &TT, sizeof(_float4x4));
    TT = XMMatrixTranslationFromVector(XMVectorSet(-10.f, 30.f, 0.f, 1.f));
    memcpy(&pMatrix[1], &TT, sizeof(_float4x4));

    Desc.pTransformMatrix = pMatrix;
    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Model_Wolf"),
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), &Desc)))
        return E_FAIL;

    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_NonAnimMesh"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    return S_OK;
}

void CMapObject::Bind_Resources()
{

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

}
