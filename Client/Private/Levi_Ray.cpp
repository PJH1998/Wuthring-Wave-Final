#include "ClientPch.h"
#include "Levi_Ray.h"

CLevi_Ray::CLevi_Ray(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CLevi_Ray::CLevi_Ray(const CLevi_Ray& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CLevi_Ray::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLevi_Ray::Initialize_Clone(void* pArg)
{
	return S_OK;
}

void CLevi_Ray::Priority_Update(_float fTimeDelta)
{
}

void CLevi_Ray::Update(_float fTimeDelta)
{
}

void CLevi_Ray::Late_Update(_float fTimeDelta)
{
}

void CLevi_Ray::Render()
{
}

void CLevi_Ray::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
}

HRESULT CLevi_Ray::Bind_Resources()
{
	return S_OK;
}

void CLevi_Ray::Ready_Component(LEVIRAY_DESC* pDesc)
{
}

void CLevi_Ray::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
}

CLevi_Ray* CLevi_Ray::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevi_Ray* pInstance = new CLevi_Ray(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CLevi_Ray");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevi_Ray::Clone(void* pArg)
{
	CLevi_Ray* pClone = new CLevi_Ray(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CLevi_Ray (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CLevi_Ray::Free()
{
	__super::Free();
	Safe_Release(m_pRigidBodyCom);
}
