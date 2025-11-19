#include "ClientPch.h"
#include "SFX_Prefab.h"

CSFX_Prefab::CSFX_Prefab(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CSFX_Prefab::CSFX_Prefab(const CSFX_Prefab& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CSFX_Prefab::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

    return S_OK;
}

HRESULT CSFX_Prefab::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	SFX_PREFAB_DESC* pDesc = static_cast<SFX_PREFAB_DESC*>(pArg);

	m_Children = *pDesc->Children;

	m_iNumChildren = static_cast<_uint>(pDesc->Children->size());

	m_isActivate = false;

    return S_OK;
}

void CSFX_Prefab::Priority_Update(_float fTimeDelta)
{
}

void CSFX_Prefab::Update(_float fTimeDelta)
{
	m_fCurrentTime += fTimeDelta;
	
	while (m_Children[m_iCurrentChild].fStartTime <= m_fCurrentTime)
	{
		m_pGameInstance->Spawn_PoolingObject(m_Children[m_iCurrentChild].strSfxTag, XMMatrixIdentity(), nullptr);
		m_iCurrentChild++;

		if (m_iCurrentChild >= m_iNumChildren)
		{
			m_isActivate = false;
			break;
		}
	}
}

void CSFX_Prefab::Late_Update(_float fTimeDelta)
{
}

void CSFX_Prefab::Render()
{
}

void CSFX_Prefab::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	if (m_iNumChildren <= 0)
		CRASH("SFX_Prefab is Has No Child");

	m_isActivate = true;
	m_fCurrentTime = 0.f;
	m_iCurrentChild = 0;
}

CSFX_Prefab* CSFX_Prefab::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSFX_Prefab* pInstance = new CSFX_Prefab(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CSFX_Prefab");
		Safe_Release(pInstance);
	}
    return pInstance;
}

CGameObject* CSFX_Prefab::Clone(void* pArg)
{
	CSFX_Prefab* pInstance = new CSFX_Prefab(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CSFX_Prefab");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CSFX_Prefab::Free()
{
	__super::Free();
}
