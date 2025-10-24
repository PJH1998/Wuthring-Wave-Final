#include "Editorpch.h"
#include "Trail_Mesh.h"

CTrail_Mesh::CTrail_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CTrail_Mesh::CTrail_Mesh(const CTrail_Mesh& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CTrail_Mesh::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTrail_Mesh::Initialize_Clone(void* pArg)
{
    TRAILMESH_DESC* pDesc = static_cast<TRAILMESH_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components(*pDesc)))
        return E_FAIL;

    m_iShaderPass = pDesc->iShaderPass;
    m_vColor = pDesc->vColor;
    m_vLifeTime = pDesc->vLifeTime;

    //트레일 전용 값
    m_fSweepSpeed = pDesc->fSweep;
    m_fSweepWitdh = pDesc->fSweepWitdh;

    _vector Pos = XMVectorSet(pDesc->vPos.x, pDesc->vPos.y, pDesc->vPos.z, 1.f);

    m_pTransformCom->Set_State(STATE::POSITION, Pos);
    m_pTransformCom->Scale(_float3(pDesc->vSize.x, pDesc->vSize.y, pDesc->vSize.z));

    m_IsRoot = pDesc->IsRootOn;
    
    if (m_IsRoot)
        m_ParentMatrix = pDesc->RootMatrix;
    //임시처리
    //m_isActivate = true;
    m_fColorSpeed = 1.f;

    return S_OK;
}

void CTrail_Mesh::Priority_Update(_float fTimeDelta)
{
}

void CTrail_Mesh::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    if(m_IsRoot)
        Root_Transform();

    //여기서 Sweep 계산 후 셰이더에 바인딩 해줘야 함.

    m_fSweep += fTimeDelta * m_fSweepSpeed;
    m_fColorSweep += fTimeDelta * m_fColorSpeed;

    if (m_fSweep >= 1.f + m_fSweepWitdh)
    {
        m_fSweep = 0.f;
        m_isActivate = false;
        m_fColorSweep = 0.f;
    }
    
    //라이프타임 끝나면 비활성화
}

void CTrail_Mesh::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CTrail_Mesh::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return;

    m_pShaderCom->Begin(m_iShaderPass);

    m_pVIBufferCom->Bind_Resources();

    m_pVIBufferCom->Render();
}

//Test
void CTrail_Mesh::Root_Transform()
{
    _matrix RootMatrix = XMLoadFloat4x4(m_ParentMatrix);

    for (size_t i = 0; i < 3; i++)
        RootMatrix.r[i] = XMVector3Normalize(RootMatrix.r[i]);

    XMStoreFloat4x4(&m_ComBindMatrix,
        (m_pTransformCom->Get_WorldMatrix() * RootMatrix));
}

HRESULT CTrail_Mesh::Ready_Components(TRAILMESH_DESC& Desc)
{
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxTrailMesh"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strVIBufferTag,
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;

    //텍스처 여러개 써야하는데 어떻게 할지 고민해보자
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
        TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strColorTextureTag,
        TEXT("Com_ColorTexture"), reinterpret_cast<CComponent**>(&m_pColorTextureCom), nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT CTrail_Mesh::Bind_ShaderResources()
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

    if (FAILED(m_pColorTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture", 0)))
        return E_FAIL;

    if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_MaskTexture", 0)))
        return E_FAIL;

    //셰이더에 바인딩 해주자.
    if (FAILED(m_pShaderCom->Bind_Value("g_Sweep", &m_fSweep, sizeof(_float))))
        return E_FAIL;

    if(FAILED(m_pShaderCom->Bind_Value("g_SweepWitdh", &m_fSweepWitdh, sizeof(_float))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Value("g_ColorSpeed", &m_fColorSweep, sizeof(_float))))
        return E_FAIL;

    return S_OK;
}

CTrail_Mesh* CTrail_Mesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTrail_Mesh* pInstance = new CTrail_Mesh(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CTrail_Mesh");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTrail_Mesh::Clone(void* pArg)
{
    CTrail_Mesh* pInstance = new CTrail_Mesh(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CTrail_Mesh");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTrail_Mesh::Free()
{
    __super::Free();

    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pColorTextureCom);
}
