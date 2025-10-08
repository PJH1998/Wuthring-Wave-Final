#include"Editorpch.h"
#include "MapObject.h"

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

    if (FAILED(Ready_Component()))
        return E_FAIL;

    m_iShaderPassIndex = 0;
    return S_OK;
}

void CMapObject::Priority_Update(_float fTimeDelta)
{

}

void CMapObject::Update(_float fTimeDelta)
{

}

void CMapObject::Late_Update(_float fTimeDelta)
{

}

void CMapObject::Render()
{
    Bind_Resources();

    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
    
    for (_uint i = 0; i < m_pModelCom->Get_NumMesh(); ++i)
    {
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
}

HRESULT CMapObject::Ready_Component()
{
    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Model_Wolf"),
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
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
