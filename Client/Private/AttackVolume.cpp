#include "ClientPch.h"
#include "AttackVolume.h"

CAttackVolume::CAttackVolume(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject { pDevice, pContext }
{
}

CAttackVolume::CAttackVolume(const CAttackVolume& Prototype)
    : CPartObject { Prototype }
{
}

HRESULT CAttackVolume::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAttackVolume::Initialize_Clone(void* pArg)
{
    ATKVOLUME_DESC* pDesc = static_cast<ATKVOLUME_DESC*>(pArg);
    Ready_Component(pDesc);

    m_pSocketMatrix = pDesc->pSocketMatrix;
	m_CollisionCallback = pDesc->CollisionCallback;
    return S_OK;
}

void CAttackVolume::Priority_Update(_float fTimeDelta)
{
}

void CAttackVolume::Update(_float fTimeDelta)
{
}

void CAttackVolume::Late_Update(_float fTimeDelta)
{
}

void CAttackVolume::Render()
{
}

void CAttackVolume::Ready_Component(ATKVOLUME_DESC* pDesc)
{
	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	RigidbodyDesc.vExtent = pDesc->vExtent;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_Enter(iLayer, pDesc, Manifold);
		});
}

void CAttackVolume::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if(ENUM_CLASS(COLLISIONLAYER::PLAYER) == iLayer)
	{
		if (m_CollisionCallback)
			m_CollisionCallback();
	}
}

CAttackVolume* CAttackVolume::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

CGameObject* CAttackVolume::Clone(void* pArg)
{
    return nullptr;
}

void CAttackVolume::Free()
{
}
