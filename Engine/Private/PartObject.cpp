#include "EnginePch.h"
#include "PartObject.h"

#include "Transform.h"
#include "Navigation.h"
#include "Model.h"
#include "Shader.h"

CPartObject::CPartObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CPartObject::CPartObject(const CPartObject& Prototype)
	: CGameObject { Prototype }
{
}

const _float4x4* CPartObject::Get_SocketBoneMatrix(const _char* pBoneName)
{
	return m_pModelCom->Get_BoneMatrixPtr(pBoneName);
}

HRESULT CPartObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPartObject::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	PART_DESC* pDesc = static_cast<PART_DESC*>(pArg);
	m_pParentTransform = pDesc->pParentTransform;
	Safe_AddRef(m_pParentTransform);
	m_pParentNavigation = pDesc->pParentNavigation;
	Safe_AddRef(m_pParentNavigation);
	m_pActionState = pDesc->pActionState;
	m_pSubActionState = pDesc->pSubActionState;
	m_pTrackPosition = pDesc->pTrackPosition;

	return S_OK;
}

void CPartObject::Priority_Update(_float fTimeDelta)
{
}

void CPartObject::Update(_float fTimeDelta)
{
}

void CPartObject::Late_Update(_float fTimeDelta)
{
}

HRESULT CPartObject::Render()
{
	return S_OK;
}

void CPartObject::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);

	Safe_Release(m_pParentTransform);
	Safe_Release(m_pParentNavigation);
}
