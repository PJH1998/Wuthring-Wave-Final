#include "Editorpch.h"
#include "Effect_Light.h"

CEffect_Light::CEffect_Light(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Light::CEffect_Light(const CEffect_Light& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CEffect_Light::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Light::Initialize_Clone(void* pArg)
{
	LIGHT_DESC* pDesc = static_cast<LIGHT_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    m_vColor = pDesc->vColor;
    m_vLifeTime = pDesc->vLifeTime;
	m_wstrMyTag = pDesc->wstrLightTag;
	m_fSpeed = pDesc->fSpeed;
	m_vRange = pDesc->vRange;

    m_isActivate = false;

    return S_OK;
}

void CEffect_Light::Priority_Update(_float fTimeDelta)
{
}

void CEffect_Light::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	//DECAL_DATA Desc{};
	//Desc.eType = DECAL_DATA::NONSTATIC;
	//Desc.fLifeTime = m_LifeTime;
	//Desc.vColor = m_vColor;
	//Desc.WorldMatrix = m_ComBindMatrix; /*m_pTransformCom->Get_WorldMatrix();*/
	//Desc.EndWorldMatrix = m_ComBindMatrix;
	//Desc.fBlendTime = m_fBlendTime;
	//Desc.fEmissiveIntensity = m_fEmissiveIntensity;

	//m_pGameInstance->Add_DecalData(m_wstrMyTag, Desc);

	m_isActivate = false;
}

void CEffect_Light::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

}

void CEffect_Light::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	EFFECT_INFO* pDesc = static_cast<EFFECT_INFO*>(pArg);

	if (m_isActivate = pDesc->IsActive)
		m_pTransformCom->Set_WorldMatrix(WorldMatrix);
   
}

CEffect_Light* CEffect_Light::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Light* pInstance = new CEffect_Light(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEffect_Light");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Light::Clone(void* pArg)
{
    CEffect_Light* pInstance = new CEffect_Light(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CEffect_Light");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Light::Free()
{
    __super::Free();

}
