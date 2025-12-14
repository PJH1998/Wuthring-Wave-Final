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
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	LEVIRAY_DESC* pDesc = static_cast<LEVIRAY_DESC*>(pArg);
	Ready_Component(pDesc);

	m_fLifeTime = pDesc->fLifeTime;
	m_iTargetLayer = pDesc->iTargetLayer;
	m_isActivate = false;
	return S_OK;
}

void CLevi_Ray::Priority_Update(_float fTimeDelta)
{
}

void CLevi_Ray::Update(_float fTimeDelta)
{
	if (m_fLifeAcc >= m_fLifeTime)
	{
		m_pRigidBodyCom->IsActivate(false);
		m_isActivate = false;
	}
	else
		m_fLifeAcc += fTimeDelta;

	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CLevi_Ray::Late_Update(_float fTimeDelta)
{
	if (m_isHit)
	{
		m_pRigidBodyCom->IsActivate(false);
		//m_isActivate = false;
		m_isHit = false;
		m_fLifeAcc = m_fLifeTime;
		return;
	}

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CLevi_Ray::Render()
{
#ifdef _DEBUG
	m_pRigidBodyCom->Render();
#endif // _DEBUG

}

void CLevi_Ray::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	LEVIRAY_RESET* pDesc = static_cast<LEVIRAY_RESET*>(pArg);
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	m_pTransformCom->LookAt(XMVectorSetW(XMLoadFloat3(&pDesc->vTargetPos), 1.f));
	m_pRigidBodyCom->IsActivate(true);
	m_fLifeAcc = 0.f;
	m_isActivate = true;
	m_isHit = false;
	//PREFAB_INFO Info{};
	//m_pGameInstance->Spawn_PoolingObject(m_wstrEffectTag, m_pTransformCom->Get_WorldMatrix(), &Info);
}

HRESULT CLevi_Ray::Bind_Resources()
{
	return S_OK;
}

void CLevi_Ray::Ready_Component(LEVIRAY_DESC* pDesc)
{
	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer =pDesc->iLayer;
	RigidbodyDesc.vExtent = pDesc->vExtent;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_Enter(iLayer, pDesc, Manifold);
		});

	m_CallBack.eType = pDesc->eType;
	m_CallBack.fAttack = pDesc->fAttackDamage;
	m_CallBack.pTransform = m_pTransformCom;
	//m_CallBack.strEffectTag = ;
	m_pRigidBodyCom->Set_Desc(&m_CallBack);
	m_pRigidBodyCom->IsActivate(false);
}

void CLevi_Ray::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if(iLayer == m_iTargetLayer)
	{
		m_isHit = true;
#ifdef _DEBUG
		cout << "On Hit! (Levi Ray)" << endl;
#endif // _DEBUG
	}

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
