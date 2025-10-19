#include "EditorPch.h"
#include "SpringCamera_Edit.h"

CSpringCamera_Edit::CSpringCamera_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera { pDevice, pContext }
{
}

CSpringCamera_Edit::CSpringCamera_Edit(const CSpringCamera_Edit& Prototype)
	: CCamera { Prototype }
{
}

void CSpringCamera_Edit::SetUp_Target(CTransform* pTargetTransform)
{
	Safe_Release(m_pTargetTransform);
	m_pTargetTransform = pTargetTransform;
	Safe_AddRef(m_pTargetTransform);
}

HRESULT CSpringCamera_Edit::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSpringCamera_Edit::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		CRASH("Camera");



    return S_OK;
}

void CSpringCamera_Edit::Priority_Update(_float fTimeDelta)
{
}

void CSpringCamera_Edit::Update(_float fTimeDelta)
{
}

void CSpringCamera_Edit::Update_Action(const _fvector& vQuaternion, _float fDistance, _float fTimeDelta)
{
}

void CSpringCamera_Edit::Late_Update(_float fTimeDelta)
{
}

void CSpringCamera_Edit::Render()
{
}

CSpringCamera_Edit* CSpringCamera_Edit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSpringCamera_Edit* pInstance = new CSpringCamera_Edit(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : SpringCamera_Edit");
		Safe_Release(pInstance);
	}

    return pInstance;
}

CGameObject* CSpringCamera_Edit::Clone(void* pArg)
{
	CSpringCamera_Edit* pClone = new CSpringCamera_Edit(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : SpringCamera_Edit (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CSpringCamera_Edit::Free()
{
	__super::Free();

	Safe_Release(m_pTargetTransform);
}
