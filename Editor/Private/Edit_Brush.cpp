#include "EditorPch.h"
#include "Edit_Brush.h"

CEdit_Brush::CEdit_Brush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CGameObject(pDevice, pContext)

{
}

CEdit_Brush::CEdit_Brush(const CEdit_Brush& Prototype)
    :CGameObject(Prototype)
{
}

HRESULT CEdit_Brush::Initialize_Prototype()
{
    __super::Initialize_Clone(nullptr);

    Ready_Components();
    m_fRange = 100.f;
    return S_OK;
}

HRESULT CEdit_Brush::Initialize_Clone(void* pArg)
{
    return S_OK;
}

void CEdit_Brush::Priority_Update(_float fTimeDelta)
{
}

void CEdit_Brush::Update(_float fTimeDelta)
{
    if (m_pGameInstance->isPicked(&m_vMousePos))
        m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_vMousePos), 1.f));

    m_pGameInstance->Add_Render_Object(RENDERGROUP::RD_DEBUG, this);
}

void CEdit_Brush::Late_Update(_float fTimeDelta)
{
}

void CEdit_Brush::Render()
{
    Bind_Resources();
    m_pShaderCom->Begin(0);
    m_pVIBufferCom->Bind_Resources();
    m_pVIBufferCom->Render();
}

void CEdit_Brush::Bind_Resources()
{
    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

    if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Depth"), m_pShaderCom, "g_DepthTexture")))
        CRASH("Render Fail")

    m_pShaderCom->Bind_Value("g_fRange", &m_fRange, sizeof(_float));
    m_pShaderCom->Bind_Value("g_iNumInstance", &m_iNumInstance, sizeof(_uint));
}

void CEdit_Brush::Ready_Components()
{
    __super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_Brush"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr);

    __super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_VIBuffer_Point"),
        TEXT("Com_VIBufferCom"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr);
}

CEdit_Brush* CEdit_Brush::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEdit_Brush* pInstance = new CEdit_Brush(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : Edit_Brush");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEdit_Brush::Free()
{
    __super::Free();
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pVIBufferCom);
}
