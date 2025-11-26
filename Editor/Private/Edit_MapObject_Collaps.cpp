#include"EditorPch.h"
#include "Edit_MapObject_Collaps.h"

CEdit_MapObject_Collaps::CEdit_MapObject_Collaps(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CEdit_MapObject_Collaps::CEdit_MapObject_Collaps(const CEdit_MapObject_Collaps& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CEdit_MapObject_Collaps::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEdit_MapObject_Collaps::Initialize_Clone(void* pArg)
{
	return S_OK;
}

void CEdit_MapObject_Collaps::Priority_Update(_float fTimeDelta)
{
}

void CEdit_MapObject_Collaps::Update(_float fTimeDelta)
{
}

void CEdit_MapObject_Collaps::Late_Update(_float fTimeDelta)
{
}

void CEdit_MapObject_Collaps::Ready_Components(void* pArg)
{
}

CEdit_MapObject_Collaps* CEdit_MapObject_Collaps::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_MapObject_Collaps* pInstance = new CEdit_MapObject_Collaps(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Edit_MapObject_Collaps");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_MapObject_Collaps::Clone(void* pArg)
{
	CEdit_MapObject_Collaps* pInstance = new CEdit_MapObject_Collaps(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Edit_MapObject_Collaps (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_MapObject_Collaps::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pMapInterface);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pRigidbodyCom);
}
