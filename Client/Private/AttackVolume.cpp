#include "ClientPch.h"
#include "AttackVolume.h"

CAttackVolume::CAttackVolume(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject { pDevice, pContext }
{
}

CAttackVolume::CAttackVolume(const CAttackVolume& Prototype)
    : CGameObject{ Prototype }
	, m_eType { Prototype.m_eType }
{
}

HRESULT CAttackVolume::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAttackVolume::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

    ATKVOLUME_DESC* pDesc = static_cast<ATKVOLUME_DESC*>(pArg);
    
	m_eType = pDesc->eType;
	m_pParenTransform = pDesc->pParenTransform;
	Safe_AddRef(m_pParenTransform);

	Ready_Component(pDesc);

    m_pSocketMatrix = pDesc->pSocketMatrix;

	if (pDesc->eTargetLayers.empty())
		m_eTargetLayer.push_back(pDesc->eTargetLayer);
	else
		m_eTargetLayer = pDesc->eTargetLayers;
	m_eLayer = pDesc->eLayer;
	m_eCurrentLayer = m_eLayer;
	m_CollisionCallback = pDesc->CollisionCallback;
	m_test = pDesc->test;
#ifdef _DEBUG
	m_vOffsetPos = pDesc->vOffsetPos;
	m_vOffsetRot = pDesc->vOffsetRadian;
#else
	_matrix matOffset = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f),
													XMQuaternionRotationRollPitchYaw(pDesc->vOffsetRadian.x, pDesc->vOffsetRadian.y, pDesc->vOffsetRadian.z),
													XMVectorSetW(XMLoadFloat3(&pDesc->vOffsetPos), 1.f));
	XMStoreFloat4x4(&m_OffsetMatrix, matOffset);
#endif // _DEBUG

    return S_OK;
}

void CAttackVolume::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;
}

void CAttackVolume::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;
#ifdef _DEBUG
	_matrix matOffset = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMQuaternionRotationRollPitchYaw(m_vOffsetRot.x, m_vOffsetRot.y, m_vOffsetRot.z), XMVectorSetW(XMLoadFloat3(&m_vOffsetPos), 1.f));
#else
	_matrix matOffset = XMLoadFloat4x4(&m_OffsetMatrix);
#endif // _DEBUG

	_matrix ComBinedMatrix;

	if (nullptr == m_pSocketMatrix)
	{
		if (nullptr == m_pParenTransform)
			CRASH("need parent transform : Bone type");
		ComBinedMatrix = m_pTransformCom->Get_WorldMatrix() * matOffset * m_pParenTransform->Get_WorldMatrix();
	}
	else
	{
		_matrix NonScaleMatrix = XMLoadFloat4x4(m_pSocketMatrix);
		_vector vScale, vQuaternion, vTransition;
		XMMatrixDecompose(&vScale, &vQuaternion, &vTransition, NonScaleMatrix);
		NonScaleMatrix = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), vQuaternion, vTransition);
		if(COMBINED_TYPE::BONE == m_eType)
			ComBinedMatrix = m_pTransformCom->Get_WorldMatrix() * matOffset * NonScaleMatrix * m_pParenTransform->Get_WorldMatrix();
		else
			ComBinedMatrix = m_pTransformCom->Get_WorldMatrix() * matOffset * NonScaleMatrix;
	}

	XMStoreFloat4x4(&m_CombinedMatrix, ComBinedMatrix);

	m_pRigidBodyCom->Update_Rigidbody(ComBinedMatrix, fTimeDelta);
}

void CAttackVolume::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;
}

void CAttackVolume::Render()
{
	if (!m_isActivate)
		return;
#ifdef _DEBUG
	m_pRigidBodyCom->Render();
#endif // _DEBUG

}

void CAttackVolume::TriggerActivate(_bool isActivate)
{
	//m_pRigidBodyCom->IsActivate(isActivate);
	if (isActivate)
	{
		m_pRigidBodyCom->Change_Layer(ENUM_CLASS(m_eLayer));
		m_eCurrentLayer = m_eLayer;
	}
	else
	{
		m_pRigidBodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
		m_eCurrentLayer = COLLISIONLAYER::NONE;
	}
	m_pRigidBodyCom->IsActivate(isActivate);
	m_isActivate = isActivate;
}

void CAttackVolume::Change_Layer(COLLISIONLAYER eLayer)
{
	m_eLayer = eLayer;
}


void CAttackVolume::Ready_Component(ATKVOLUME_DESC* pDesc)
{
	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = pDesc->eShape;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(pDesc->eLayer);
	RigidbodyDesc.vExtent = pDesc->vExtent;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_Enter(iLayer, pDesc, Manifold);
		});
	//m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
	//	OnCollide_During(iLayer, pDesc, Manifold);
	//	});

	m_CallBack.pTransform = m_pParenTransform;
	m_CallBack.fAttack = pDesc->fAttackDmg;

	m_pRigidBodyCom->Set_Desc(&m_CallBack);
}

void CAttackVolume::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	//f (m_eCurrentLayer == COLLISIONLAYER::NONE)
	//	return;
	for(auto& eLayer : m_eTargetLayer)
	{
		if (ENUM_CLASS(eLayer) == iLayer)
		{
			if (m_CollisionCallback)
				m_CollisionCallback(iLayer, pDesc, Manifold);
			if (m_test)
				m_test(iLayer, pDesc, Manifold, m_eCurrentLayer);
			return;
		}
	}
}

//#ifdef _DEBUG
//void CAttackVolume::OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
//{
//	cout << "Collision test" << endl;
//
//}
//#endif // _DEBUG

CAttackVolume* CAttackVolume::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAttackVolume* pInstance = new CAttackVolume(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CAttackVolume");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAttackVolume::Clone(void* pArg)
{
	CAttackVolume* pClone = new CAttackVolume(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CAttackVolume (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CAttackVolume::Free()
{
	__super::Free();

	Safe_Release(m_pRigidBodyCom);
	Safe_Release(m_pParenTransform);

}
