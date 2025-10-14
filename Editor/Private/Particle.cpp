#include "Editorpch.h"
#include "Particle.h"
#include "GameInstance.h"

CParticle::CParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CParticle::CParticle(const CParticle& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CParticle::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CParticle::Initialize_Clone(void* pArg)
{
    PARTICLE_DESC* pDesc = static_cast<PARTICLE_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components(*pDesc)))
        return E_FAIL;

    m_fShaderPass = pDesc->fShaderPass;
    m_vColor = pDesc->vColor;
    m_vLifeTime = pDesc->vLifeTime;

    m_pTransformCom->Scale(_float3(pDesc->vSize.x, pDesc->vSize.y, pDesc->vSize.z));

    //임시처리
    m_IsSpread = pDesc->bSpread;
    m_IsDrop = pDesc->bDrop;

    return S_OK;
}

void CParticle::Priority_Update(_float fTimeDelta)
{
}

void CParticle::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    //임시
    if (m_IsDrop)
        m_pVIBufferCom->Drop(fTimeDelta);
    
    if (m_IsSpread)
        m_pVIBufferCom->Spread(fTimeDelta);

    //라이프타임 끝나면 비활성화
}

void CParticle::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CParticle::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return;

    m_pShaderCom->Begin(0);

    m_pVIBufferCom->Bind_Resources();

    m_pVIBufferCom->Render();
}

HRESULT CParticle::Ready_Components(PARTICLE_DESC& Desc)
{
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxInstance_PointParticle"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strVIBufferTag,
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
        TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT CParticle::Bind_ShaderResources()
{
    if(FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Value("g_vCamPosition", m_pGameInstance->Get_CamPos(), sizeof(_float4))))
        return E_FAIL;

    if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture", 0)))
        return E_FAIL;

    return S_OK;
}

CParticle* CParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CParticle* pInstance = new CParticle(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CParticle::Clone(void* pArg)
{
    CParticle* pInstance = new CParticle(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CParticle::Free()
{
    __super::Free();

    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
}
