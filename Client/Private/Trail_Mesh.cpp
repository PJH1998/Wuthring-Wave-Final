#include "ClientPch.h"
#include "Trail_Mesh.h"

CTrail_Mesh::CTrail_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CTrail_Mesh::CTrail_Mesh(const CTrail_Mesh& Prototype)
    : CGameObject{ Prototype }
    , m_tDesc { Prototype.m_tDesc }
{
}

HRESULT CTrail_Mesh::Initialize_Prototype(const TRAILMESH_DESC* pDesc)
{
    m_tDesc = *pDesc;

    return S_OK;
}

HRESULT CTrail_Mesh::Initialize_Clone(void* pArg)
{
    //TRAILMESH_DESC* pDesc = static_cast<TRAILMESH_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components(m_tDesc)))
        return E_FAIL;

    m_iShaderPass = m_tDesc.iShaderPass;
    m_vColor = m_tDesc.vColor;
    m_vLifeTime = m_tDesc.vLifeTime;

    //트레일 전용 값
    m_fSweepSpeed = m_tDesc.fSweep;
    m_fSweepWitdh = m_tDesc.fSweepWitdh;

    _vector Pos = XMVectorSet(m_tDesc.vPos.x, m_tDesc.vPos.y, m_tDesc.vPos.z, 1.f);

    m_pTransformCom->Set_State(STATE::POSITION, Pos);
    m_pTransformCom->Scale(_float3(m_tDesc.vSize.x, m_tDesc.vSize.y, m_tDesc.vSize.z));

    m_IsRoot = m_tDesc.IsRootOn;
    
    //임시처리
    m_isActivate = false;
    m_fColorSpeed = 1.f;

    XMStoreFloat4x4(&m_ComBindMatrix, XMMatrixIdentity());
    Root_Transform(XMLoadFloat4x4(&m_ComBindMatrix));

    return S_OK;
}

void CTrail_Mesh::Priority_Update(_float fTimeDelta)
{
}

void CTrail_Mesh::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    //여기서 Sweep 계산 후 셰이더에 바인딩 해줘야 함.
    m_fSweep += fTimeDelta * m_fSweepSpeed;
    m_fColorSweep += fTimeDelta * m_fColorSpeed;
    m_vLifeTime.x += fTimeDelta;

    if (m_vLifeTime.x >= m_vLifeTime.y)
    {
        m_fSweep = 0.f;
        m_isActivate = false;
        m_fColorSweep = 0.f;
        m_vLifeTime.x = 0.f;
    }

    if (m_fSweep >= 1.f + m_fSweepWitdh)
    {
        m_fSweep = 0.f;
        m_isActivate = false;
        m_fColorSweep = 0.f;
    }
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

void CTrail_Mesh::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
    if (_bool* IsActivate = static_cast<_bool*>(pArg))
        m_isActivate = *IsActivate;

    m_fSweep = 0.f;
    m_fColorSweep = 0.f;
    m_vLifeTime.x = 0.f;
    Root_Transform(WorldMatrix);
}

void CTrail_Mesh::Root_Transform(_fmatrix WorldMatrix)
{
     if (m_IsRoot)
     {
         _matrix SpawnMatrix = WorldMatrix;
         _float4x4 SpawnW = {};
         XMStoreFloat4x4(&SpawnW, SpawnMatrix);

         _float fYaw = atan2f(SpawnW._31, SpawnW._33);
         XMMATRIX RotationMatrix = XMMatrixRotationY(fYaw) * XMMatrixRotationX(-90.f);      //나중에 설정값 줄수 있게?

         XMVECTOR vPos = XMVectorSet(SpawnW._41, SpawnW._42, SpawnW._43, 1.f);
         XMMATRIX PosMatrix = XMMatrixTranslationFromVector(vPos);

         SpawnMatrix = RotationMatrix * PosMatrix;

         XMStoreFloat4x4(&m_ComBindMatrix,
             m_pTransformCom->Get_WorldMatrix() *
             SpawnMatrix);
     }
     else
     {
         _vector vPos = XMVectorSetW(WorldMatrix.r[3], 1.f);
         m_pTransformCom->Set_State(STATE::POSITION, vPos);

         XMStoreFloat4x4(&m_ComBindMatrix,
             m_pTransformCom->Get_WorldMatrix());
     }
}

HRESULT CTrail_Mesh::Ready_Components(TRAILMESH_DESC& Desc)
{
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Shader_VtxTrailMesh"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(Desc.CurrentLevel, Desc.strVIBufferTag,
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;

    //텍스처 여러개 써야하는데 어떻게 할지 고민해보자
    if (FAILED(CGameObject::Add_Component(Desc.CurrentLevel, Desc.strTextureTag,
        TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(Desc.CurrentLevel, Desc.strColorTextureTag,
        TEXT("Com_ColorTexture"), reinterpret_cast<CComponent**>(&m_pColorTextureCom), nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT CTrail_Mesh::Bind_ShaderResources()
{
    //if (!m_IsRoot)
    //{
    //    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
    //        return E_FAIL;
    //}
    //else
    //{
        if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_ComBindMatrix)))
            return E_FAIL;
   /* }*/

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

    if (FAILED(m_pShaderCom->Bind_Value("g_Time", &m_fTime, sizeof(_float))))
        return E_FAIL;

    return S_OK;
}

CTrail_Mesh* CTrail_Mesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const TRAILMESH_DESC* pDesc)
{
    CTrail_Mesh* pInstance = new CTrail_Mesh(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pDesc)))
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
