#include "ClientPch.h"
#include "Effect_Rect.h"

CEffect_Rect::CEffect_Rect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Rect::CEffect_Rect(const CEffect_Rect& Prototype)
    : CGameObject{ Prototype }
	, m_tDesc { Prototype.m_tDesc }
{
}

HRESULT CEffect_Rect::Initialize_Prototype(const FXRECT_DESC* pDesc)
{
	m_tDesc = *pDesc;

    return S_OK;
}

HRESULT CEffect_Rect::Initialize_Clone(void* pArg)
{

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components(m_tDesc)))
        return E_FAIL;

    m_iShaderPass = m_tDesc.iShaderPass;
	m_iMaskFlag = m_tDesc.iMaskFlag;

    m_vColor = m_tDesc.vColor;
    m_vLifeTime = m_tDesc.vLifeTime;

	m_fSweepSpeed = m_tDesc.fSweepSpeed;
	m_fSoft = m_tDesc.fSoft;

	m_fXSize = m_tDesc.fXSize;
	m_fYSize = m_tDesc.fYSize;

    _vector Pos = XMVectorSet(m_tDesc.vPos.x, m_tDesc.vPos.y, m_tDesc.vPos.z, 1.f);

    m_pTransformCom->Set_State(STATE::POSITION, Pos);

    m_isActivate = false;

    return S_OK;
}

void CEffect_Rect::Priority_Update(_float fTimeDelta)
{
}

void CEffect_Rect::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

   m_vLifeTime.x += fTimeDelta;
   m_fSweep += fTimeDelta * m_fSweepSpeed;

   if (m_vLifeTime.x >= m_vLifeTime.y)
   {
       m_isActivate = false;
       m_vLifeTime.x = 0.f;
	   m_fSweep = 0.f;
   }
}

void CEffect_Rect::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    m_pGameInstance->Add_Render_Object(RENDERGROUP::EFFECT, this);
}

void CEffect_Rect::Render()
{   
	if (FAILED(Bind_ShaderResources()))
        return;
	
    m_pShaderCom->Begin(m_iShaderPass);
	
    m_pVIBufferCom->Bind_Resources();
	
    m_pVIBufferCom->Render();
}

void CEffect_Rect::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
    if(_bool* IsActivate = static_cast<_bool*>(pArg))
        m_isActivate = *IsActivate;

     m_vLifeTime.x = 0.f;
	 m_fSweep = 0.f;
     Root_Transform(WorldMatrix);
}

void CEffect_Rect::Root_Transform(_fmatrix WorldMatrix)
{
    _vector vPos =  XMVectorSetW(WorldMatrix.r[3], 1.f);

    m_pTransformCom->Set_State(STATE::POSITION, vPos);
}

void CEffect_Rect::Bind_CS_SpriteInfo()
{
}

HRESULT CEffect_Rect::Ready_Components(FXRECT_DESC& Desc)
{
     if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Shader_VtxFXRect"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC),TEXT("Prototype_Componnent_VIBuffer_FXRect"),
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(Desc.CurrentLevel, Desc.strTextureTag,
        TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Rect::Bind_ShaderResources()
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

	if (FAILED(m_pShaderCom->Bind_Value("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_Sweep", &m_fSweep, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_Soft", &m_fSoft, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_fXSize", &m_fXSize, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_fYSize", &m_fYSize, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_MaskFlag", &m_iMaskFlag, sizeof(_int))))
		return E_FAIL;

    return S_OK;
}

CEffect_Rect* CEffect_Rect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const FXRECT_DESC* pDesc)
{
    CEffect_Rect* pInstance = new CEffect_Rect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pDesc)))
    {
        MSG_BOX("Failed to Created : CEffect_Rect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Rect::Clone(void* pArg)
{
    CEffect_Rect* pInstance = new CEffect_Rect(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CEffect_Rect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Rect::Free()
{
    __super::Free();

    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
}
