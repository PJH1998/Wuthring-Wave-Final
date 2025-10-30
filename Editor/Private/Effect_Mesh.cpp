#include "Editorpch.h"
#include "Effect_Mesh.h"

CEffect_Mesh::CEffect_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Mesh::CEffect_Mesh(const CEffect_Mesh& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CEffect_Mesh::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Mesh::Initialize_Clone(void* pArg)
{
    EFFECTMESH_DESC* pDesc = static_cast<EFFECTMESH_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components(*pDesc)))
        return E_FAIL;

    m_fShaderPass = pDesc->fShaderPass;
    m_vColor = pDesc->vColor;
    m_vLifeTime = pDesc->vLifeTime;

    _vector Pos = XMVectorSet(pDesc->vPos.x, pDesc->vPos.y, pDesc->vPos.z, 1.f);

    m_pTransformCom->Set_State(STATE::POSITION, Pos);
    m_pTransformCom->Scale(_float3(pDesc->vSize.x, pDesc->vSize.y, pDesc->vSize.z));

    m_IsRoot = pDesc->IsRootOn;
    
    if (m_IsRoot)
        m_ParentMatrix = pDesc->RootMatrix;
    //�ӽ�ó��
    m_isActivate = true;

    return S_OK;
}

void CEffect_Mesh::Priority_Update(_float fTimeDelta)
{
}

void CEffect_Mesh::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    if(m_IsRoot)
        Root_Transform();


    m_pVIBufferCom->Bind_CSResources(m_pComputeShaderCom, fTimeDelta);
    //������ ó�� ��� ?
   /* m_pVIBufferCom->Bind_CSResources(m_pComputeShader, fTimeDelta);*/

   // m_vLifeTime.x += fTimeDelta;
   //
   // if (m_vLifeTime.x >= m_vLifeTime.y)
   //     m_isActivate = false;
   //
    //������Ÿ�� ������ ��Ȱ��ȭ
}

void CEffect_Mesh::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEffect_Mesh::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return;

    m_pShaderCom->Begin(0);

    m_pVIBufferCom->Bind_Resources();

    m_pVIBufferCom->Render();
}

//Test
void CEffect_Mesh::Root_Transform()
{
    _matrix RootMatrix = XMLoadFloat4x4(m_ParentMatrix);

    for (size_t i = 0; i < 3; i++)
        RootMatrix.r[i] = XMVector3Normalize(RootMatrix.r[i]);

    XMStoreFloat4x4(&m_ComBindMatrix,
        (m_pTransformCom->Get_WorldMatrix() * RootMatrix));
}

HRESULT CEffect_Mesh::Ready_Components(EFFECTMESH_DESC& Desc)
{
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxInstance_FXMesh"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strVIBufferTag,
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;

    //�ؽ�ó ������ ����ϴµ� ��� ���� ����غ���
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
        TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_ComputeShader_FXMesh"),
        TEXT("Com_CShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Mesh::Bind_ShaderResources()
{
    if (!m_IsRoot)
    {
        if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
            return E_FAIL;
    }
    else
    {
        if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_ComBindMatrix)))
            return E_FAIL;
    }

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        return E_FAIL;

    if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture", 0)))
        return E_FAIL;

    return S_OK;
}

CEffect_Mesh* CEffect_Mesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Mesh* pInstance = new CEffect_Mesh(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEffect_Mesh");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Mesh::Clone(void* pArg)
{
    CEffect_Mesh* pInstance = new CEffect_Mesh(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CEffect_Mesh");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Mesh::Free()
{
    __super::Free();

    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pComputeShaderCom);
}
