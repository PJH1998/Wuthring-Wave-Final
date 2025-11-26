#include "ClientPch.h"
#include "NPCCell.h"

CNPCCell::CNPCCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CNPCCell::CNPCCell(const CNPCCell& Prototype)
	: CGameObject{ Prototype }
{
}

HRESULT CNPCCell::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CNPCCell::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	DUMMYCELL_DESC* pDesc = static_cast<DUMMYCELL_DESC*>(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vStartPos), 1.f));
	m_pTransformCom->Rotation_Quaternion(pDesc->vStartRot);
	Ready_Component(pDesc);

	m_pUpdateRootFunc = pDesc->pUpdateRootFunc;
	m_pUpdateAnimStateFunc = pDesc->pUpdateAnimStateFunc;
	m_strAnimationTag = m_strOriginAnimationTag = pDesc->szAnimationTag;
	m_iInstanceIndex = pDesc->iInstanceIndex;
	m_iFaceIndex = m_iOriginFaceIndex = pDesc->iFaceIndex;
	m_fTrackPos = pDesc->fTrackPos;
	m_fRootMotionRate = 1.f;

	if (pDesc->iNumMeshType > 0)
	{
		m_MeshTypeIndices.resize(pDesc->iNumMeshType);
		memcpy(m_MeshTypeIndices.data(), pDesc->iMeshTypes, sizeof(_uint) * pDesc->iNumMeshType);
	}
	m_vDetectOffset = _float3(0.f, 1.f, 0.f);
    return S_OK;
}

void CNPCCell::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
}

void CNPCCell::Update(_float fTimeDelta)
{
	_bool isAnimFinished{};
	if(m_pUpdateRootFunc)
		isAnimFinished = m_pUpdateRootFunc(m_strAnimationTag, m_pTransformCom, fTimeDelta, &m_fTrackPos, m_isRootMotion, m_isRootMotionRotate, m_isRootMotionTranslate, m_fRootMotionRate);
	if (isAnimFinished && m_CollideTrigger)
	{
		m_CollideTrigger = false;
		m_strAnimationTag = m_strOriginAnimationTag;
	}

	if (m_pColliderCom)
	{
		_vector vVelocity = m_pTransformCom->Get_Velocity();
		m_pColliderCom->Update(vVelocity / fTimeDelta);
	}
	if (m_pRigidbodyCom)
	{
		m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix() * XMMatrixTranslation(m_vDetectOffset.x, m_vDetectOffset.y, m_vDetectOffset.z), fTimeDelta);
	}
}

void CNPCCell::Late_Update(_float fTimeDelta)
{
	if (m_pColliderCom)
		m_pColliderCom->Sync_Position(m_pTransformCom);

	if (m_pUpdateAnimStateFunc)
	{
		if (m_MeshTypeIndices.empty())
			m_pUpdateAnimStateFunc(m_strAnimationTag, m_pTransformCom->Get_WorldMatrix(), m_iInstanceIndex, &m_fTrackPos, nullptr, m_iFaceIndex);
		else
			m_pUpdateAnimStateFunc(m_strAnimationTag, m_pTransformCom->Get_WorldMatrix(), m_iInstanceIndex, &m_fTrackPos, m_MeshTypeIndices.data(), m_iFaceIndex);
	}
}

void CNPCCell::Ready_Component(DUMMYCELL_DESC* pDesc)
{
	if (pDesc->isCollide)
	{
		// Com_Rigidbody
		CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
		RigidbodyDesc.eBodyType = CRigidbody::BODY;
		RigidbodyDesc.eShape = SHAPE::BOX;
		RigidbodyDesc.eType = EMotionType::Kinematic;
		RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
		RigidbodyDesc.vExtent = _float3(0.4f, 0.3f, 0.4f);
		XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

		if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
			TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
			CRASH("Rigidbody");

		if (pDesc->isCollide)
		{
			m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
				OnCollide_Enter(iLayer, pDesc, Manifold);
				});
		}
	}
	// Com_Collider
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 0.6f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NPC);
	ColliderDesc.fHeight = 1.0f;
	ColliderDesc.fRadius = 0.1f;
	ColliderDesc.fRayOffset = -0.17;
	Add_Component(ENUM_CLASS(LEVEL::STATIC),TEXT("Prototype_Component_Collider"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);
	m_pColliderCom->Set_Gravity(true);
	//if(pDesc->isCollide)
	//{
	//	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
	//		OnCollide_Enter(iLayer, pDesc, Manifold);
	//		});
	//}
	m_tCallBack.pTransform = m_pTransformCom;
	m_pColliderCom->Set_Desc(&m_tCallBack);

}

void CNPCCell::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		if (false == m_CollideTrigger)
		{
			m_CollideTrigger = true;
			//m_strAnimationTag = 어깨빵 애니메이션
#ifdef _DEBUG
			cout << m_iInstanceIndex << " 어깨빵!" << endl;
#endif // _DEBUG
		}
	}
}

CNPCCell* CNPCCell::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNPCCell* pInstance = new CNPCCell(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CNPCCell");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNPCCell::Clone(void* pArg)
{
	CNPCCell* pClone = new CNPCCell(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CNPCCell (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CNPCCell::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pRigidbodyCom);
}
