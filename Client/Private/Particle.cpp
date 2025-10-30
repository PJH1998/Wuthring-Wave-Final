#include "ClientPch.h"
#include "Particle.h"

CParticle::CParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CParticle::CParticle(const CParticle& Prototype)
    : CGameObject{ Prototype }
    , m_tDesc {Prototype.m_tDesc}
{
}

HRESULT CParticle::Initialize_Prototype(const PARTICLE_DESC* pDesc)
{
    m_tDesc = *pDesc;

    return S_OK;
}

HRESULT CParticle::Initialize_Clone(void* pArg)
{
   // PARTICLE_DESC* pDesc = static_cast<PARTICLE_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components(m_tDesc)))
        return E_FAIL;

    m_iShaderPass = m_tDesc.iShaderPass;
    m_vColor = m_tDesc.vColor;
    m_vLifeTime = m_tDesc.vLifeTime;

    _vector Pos = XMVectorSet(m_tDesc.vPos.x, m_tDesc.vPos.y, m_tDesc.vPos.z, 1.f);

    m_pTransformCom->Set_State(STATE::POSITION, Pos);
    m_pTransformCom->Scale(_float3(m_tDesc.vSize.x, m_tDesc.vSize.y, m_tDesc.vSize.z));

    if (m_IsSprite = m_tDesc.IsSprite)
    {
        m_iRow = m_tDesc.iRows;
        m_iCol = m_tDesc.iCols;
    }

    m_isActivate = false;

    return S_OK;
}

void CParticle::Priority_Update(_float fTimeDelta)
{
}

void CParticle::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    m_pVIBufferCom->Bind_CS_Speed(fTimeDelta);
    m_pVIBufferCom->Bind_CSResources(m_pComputeShader);

   m_vLifeTime.x += fTimeDelta;

   if (m_vLifeTime.x >= m_vLifeTime.y)
   {
       m_isActivate = false;
       m_vLifeTime.x = 0.f;
   }
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

    m_pShaderCom->Begin(m_iShaderPass);

    m_pVIBufferCom->Bind_Resources();

    m_pVIBufferCom->Render();
}

void CParticle::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
    if(_bool* IsActivate = static_cast<_bool*>(pArg))
        m_isActivate = *IsActivate;

     m_vLifeTime.x = 0.f;
     Root_Transform(WorldMatrix);
     m_pVIBufferCom->Reset_UAV(m_pComputeShader);
}

void CParticle::Root_Transform(_fmatrix WorldMatrix)
{
    _vector vPos =  XMVectorSetW(WorldMatrix.r[3], 1.f);

    m_pTransformCom->Set_State(STATE::POSITION, vPos);
}

void CParticle::Bind_CS_SpriteInfo()
{
}

HRESULT CParticle::Ready_Components(PARTICLE_DESC& Desc)
{
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Shader_VtxInstance_PointParticle"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(Desc.CurrentLevel, Desc.strVIBufferTag,
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(Desc.CurrentLevel, Desc.strTextureTag,
        TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Shader_ComputeShader_Particle"),
        TEXT("Com_CShader"), reinterpret_cast<CComponent**>(&m_pComputeShader), nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT CParticle::Bind_ShaderResources()
{

    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Value("g_vCamPosition", m_pGameInstance->Get_CamPos(), sizeof(_float4))))
        return E_FAIL;

    if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture", 0)))
        return E_FAIL;

    if (m_IsSprite)
    {
        if (FAILED(m_pShaderCom->Bind_Value("g_vColor", &m_vColor, sizeof(_float4))))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_Value("g_iRow", &m_iRow, sizeof(_int))))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_Value("g_iCol", &m_iCol, sizeof(_int))))
            return E_FAIL;
    }

    return S_OK;
}

CParticle* CParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext ,const PARTICLE_DESC* pDesc)
{
    CParticle* pInstance = new CParticle(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pDesc)))
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
    Safe_Release(m_pComputeShader);
}
