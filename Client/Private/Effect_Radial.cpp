#include "ClientPch.h"
#include "Effect_Radial.h"

CEffect_Radial::CEffect_Radial(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Radial::CEffect_Radial(const CEffect_Radial& Prototype)
    : CGameObject{ Prototype }
	, m_tDesc { Prototype.m_tDesc }
{
}

HRESULT CEffect_Radial::Initialize_Prototype(const RADIAL_DESC* pDesc)
{
	m_tDesc = *pDesc;
    return S_OK;
}

HRESULT CEffect_Radial::Initialize_Clone(void* pArg)
{
	RADIAL_DESC* pDesc = static_cast<RADIAL_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    //처음 만들어질 땐 무조건 활성화 ?
    m_isActivate = false;

    return S_OK;
}

void CEffect_Radial::Priority_Update(_float fTimeDelta)
{
}

void CEffect_Radial::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	if (!m_tDesc.PositionFlag)
		m_pGameInstance->Setting_Radial(m_tDesc.Center, m_tDesc.DistanceRange, m_tDesc.IntensityRange);
	else
		m_pGameInstance->Setting_Radial(m_vObjectPos, m_tDesc.DistanceRange, m_tDesc.IntensityRange);


	m_pGameInstance->Begin_Toggle_SFX(SFX_TOGGLE::RADIAL, m_tDesc.fLifeTime);

	m_isActivate = false;
}

void CEffect_Radial::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

}

void CEffect_Radial::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	EFFECT_INFO* pDesc = static_cast<EFFECT_INFO*>(pArg);

	m_isActivate = pDesc->IsActive;

	if (m_tDesc.PositionFlag)
	{
		m_vObjectPos = WorldMatrix.r[3];
	}
}


CEffect_Radial* CEffect_Radial::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const RADIAL_DESC* pDesc)
{
    CEffect_Radial* pInstance = new CEffect_Radial(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pDesc)))
    {
        MSG_BOX("Failed to Created : CEffect_Radial");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Radial::Clone(void* pArg)
{
    CEffect_Radial* pInstance = new CEffect_Radial(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CEffect_Radial");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Radial::Free()
{
    __super::Free();

}
