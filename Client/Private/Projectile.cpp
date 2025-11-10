#include "ClientPch.h"
#include "Projectile.h"

CProjectile::CProjectile(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CProjectile::CProjectile(const CProjectile& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CProjectile::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CProjectile::Initialize_Clone(void* pArg)
{
    return S_OK;
}

void CProjectile::Priority_Update(_float fTimeDelta)
{
}

void CProjectile::Update(_float fTimeDelta)
{
}

void CProjectile::Late_Update(_float fTimeDelta)
{
}

void CProjectile::Render()
{
}

void CProjectile::Reset(const _fmatrix& WorldMatrix, void* pArg)
{

}

void CProjectile::Ready_Component(PROJECTILEDESC* pDesc)
{
}

void CProjectile::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
}

CProjectile* CProjectile::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CGameObject* CProjectile::Clone(void* pArg)
{
    return nullptr;
}

void CProjectile::Free()
{
	__super::Free();

	Safe_Release(m_pRigidBodyCom);
}
