#include "ClientPch.h"
#include "Effect_VA.h"

CEffect_VA::CEffect_VA(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CEffect_VA::CEffect_VA(const CEffect_VA& Prototype)
	: CGameObject { Prototype },
	m_tDesc{ Prototype.m_tDesc }
{
}

HRESULT CEffect_VA::Initialize_Prototype(const VA_DESC* pDesc)
{
	m_tDesc = *pDesc;

	return S_OK;
}

HRESULT CEffect_VA::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		CRASH("Transform");

	m_isActivate = false;

	return S_OK;
}

void CEffect_VA::Priority_Update(_float fTimeDelta)
{
}

void CEffect_VA::Update(_float fTimeDelta)
{
	if (m_iTrackPosition > m_iAnimationDuration)
		m_isActivate = false;

	m_iTrackPosition += 1;
}

void CEffect_VA::Late_Update(_float fTimeDelta)
{
}

void CEffect_VA::Render()
{
	Bind_Resource();

	
}

void CEffect_VA::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_iTrackPosition = 0;
}

void CEffect_VA::Bind_Resource()
{
}

CEffect_VA* CEffect_VA::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const VA_DESC* pDesc)
{
	CEffect_VA* pInstance = new CEffect_VA(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pDesc)))
		CRASH("EFfect_VA");

	return pInstance;
}

CGameObject* CEffect_VA::Clone(void* pArg)
{
	CEffect_VA* pClone = new CEffect_VA(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
		CRASH("EFfect_VA");

	return pClone;
}

void CEffect_VA::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pColorTextureCom);
}
