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
	m_iTargetLayers = pDesc->iTargetLayers;
    return S_OK;
}

void CProjectile::Priority_Update(_float fTimeDelta)
{
	if (m_fLifeTime < 10.f)
		m_fLifeTime += fTimeDelta;
	else
		m_isCollision = true;
}

void CProjectile::Update(_float fTimeDelta)
{
	m_pTransformCom->Go_Straight(fTimeDelta);

	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CProjectile::Late_Update(_float fTimeDelta)
{
	if (m_isCollision)
	{
		m_isActivate = false;
		m_pRigidBodyCom->IsActivate(false);
		return;
	}
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
	m_pTransformCom->LookAt(XMVectorSetW(XMLoadFloat3(&pDesc->vTargetPos), 1.f));
	m_isCollision = false;
	m_pRigidBodyCom->IsActivate(true);
	m_isActivate = true;
	m_fLifeTime = 0.f;
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
	//m_CallBack.pCondition = &m_iState;
	//m_tCallDesc.strEffectTag = ;
	m_CallBack.eType = pDesc->eType;
	m_pRigidBodyCom->Set_Desc(&m_CallBack);
}

void CProjectile::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	for( auto& iTarget : m_iTargetLayers)
	{
		if (iLayer == iTarget)
		{
			m_isCollision = true;
			//CAMERA_SHAKE ShakeDesc{};
			//ShakeDesc.fAmplitude = 1.f;
			//ShakeDesc.fDuration = 0.1f;
			//ShakeDesc.fFovKick = 0.f;
			//ShakeDesc.fFrequency = 60.f;
			//ShakeDesc.vRotation = _float3(0.05f, 0.05f, 0.f);
			//ShakeDesc.vTranslation;
			//m_pGameInstance->OnShake(ShakeDesc);
#ifdef _DEBUG
			cout << "On Hit! (Projectile)" << endl;
#endif // _DEBUG
			return;
		}
	}

}

CProjectile* CProjectile::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CProjectile* pInstance = new CProjectile(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CProjectile");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CProjectile::Clone(void* pArg)
{
	CProjectile* pClone = new CProjectile(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CProjectile (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CProjectile::Free()
{
	__super::Free();

	Safe_Release(m_pRigidBodyCom);
}
