#include "Editorpch.h"
#include "Effect_Rect.h"

CEffect_Rect::CEffect_Rect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Rect::CEffect_Rect(const CEffect_Rect& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CEffect_Rect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Rect::Initialize_Clone(void* pArg)
{
	FXRECT_DESC* pDesc = static_cast<FXRECT_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components(*pDesc)))
        return E_FAIL;

    m_iShaderPass = pDesc->fShaderPass;
    m_vColor = pDesc->vColor;
    m_vLifeTime = pDesc->vLifeTime;

    _vector Pos = XMVectorSet(pDesc->vPos.x, pDesc->vPos.y, pDesc->vPos.z, 1.f);

    m_pTransformCom->Set_State(STATE::POSITION, Pos);
    m_pTransformCom->Scale(_float3(pDesc->vSize.x, pDesc->vSize.y, pDesc->vSize.z));

    //if (m_IsSprite = pDesc->IsSprite)
    //{

    //    m_iRow = pDesc->iRows;
    //    m_iCol = pDesc->iCols;
    //}

    //처음 만들어질 땐 무조건 활성화 ?
    m_isActivate = true;

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

   if (m_vLifeTime.x >= m_vLifeTime.y)
   {
       m_isActivate = false;
       m_vLifeTime.x = 0.f;
   }
}

void CEffect_Rect::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    m_pGameInstance->Add_Render_Object(RENDERGROUP::EMISSIVE, this);
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
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxFXRect"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT),TEXT("Prototype_Componnent_VIBuffer_FXRect"),
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
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

    return S_OK;
}

CEffect_Rect* CEffect_Rect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Rect* pInstance = new CEffect_Rect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
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
