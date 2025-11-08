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
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	PROJECTILEDESC* pDesc = static_cast<PROJECTILEDESC*>(pArg);
	Ready_Component(pDesc);
    return S_OK;
}

void CProjectile::Priority_Update(_float fTimeDelta)
{
}

void CProjectile::Update(_float fTimeDelta)
{
	m_pTransformCom->Go_Straight(fTimeDelta);

	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CProjectile::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::EFFECT, this)))
		return;
}

void CProjectile::Render()
{
#ifdef _DEBUG
	m_pRigidBodyCom->Render();
#endif
}

void CProjectile::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	PROJECTILERESET* pDesc = static_cast<PROJECTILERESET*>(pArg);
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	m_pTransformCom->LookAt(XMLoadFloat3(&pDesc->vTargetPos));
}

void CProjectile::Ready_Component(PROJECTILEDESC* pDesc)
{
	CRigidbody::SPHEREBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::SPHERE;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = pDesc->iLayer;
	RigidbodyDesc.fRadius = pDesc->fRadius;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_Enter(iLayer, pDesc, Manifold);
		});

	m_CallBack.pTransform = m_pTransformCom;
	m_CallBack.fAttack = pDesc->fAttackDamage;
	m_pRigidBodyCom->Set_Desc(&m_CallBack);
}

void CProjectile::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	for( auto& iTarget : m_iTargetLayers)
	{
		if (iLayer == iTarget)
		{
			m_isActivate = false;
#ifdef _DEBUG
			cout << "On Hit! (Projectile)" << endl;
#endif // _DEBUG
			return;
		}
	}

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
