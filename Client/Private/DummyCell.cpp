#include "ClientPch.h"
#include "DummyCell.h"

CDummyCell::CDummyCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CDummyCell::CDummyCell(const CDummyCell& Prototype)
	: CGameObject{ Prototype }
{
}

HRESULT CDummyCell::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CDummyCell::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	DUMMYCELL_DESC* pDesc = static_cast<DUMMYCELL_DESC*>(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vStartPos), 1.f));
	//Ready_Component(pDesc);

	m_pUpdateRootFunc = pDesc->pUpdateRootFunc;
	m_pUpdateAnimStateFunc = pDesc->pUpdateAnimStateFunc;
	m_strAnimationTag = pDesc->szAnimationTag;
	m_iInstanceIndex = pDesc->iInstanceIndex;
	m_fTrackPos = pDesc->fTrackPos;
	m_fRootMotionRate = 1.f;
	m_isRootMotion = true;
	m_isRootMotionTranslate = true;
	if (pDesc->iNumMeshType > 0)
	{
		m_MeshTypeIndices.reserve(pDesc->iNumMeshType);
		memcpy(m_MeshTypeIndices.data(), pDesc->iMeshTypes, sizeof(_uint) * pDesc->iNumMeshType);
	}

    return S_OK;
}

void CDummyCell::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
}

void CDummyCell::Update(_float fTimeDelta)
{
	if(m_pUpdateRootFunc)
		m_pUpdateRootFunc(m_strAnimationTag, m_pTransformCom, fTimeDelta, &m_fTrackPos, m_isRootMotion, m_isRootMotionRotate, m_isRootMotionTranslate, m_fRootMotionRate);

	if (m_pColliderCom)
	{
		_vector vVelocity = m_pTransformCom->Get_Velocity();
		m_pColliderCom->Update(vVelocity / fTimeDelta);
	}
}

void CDummyCell::Late_Update(_float fTimeDelta)
{
	if (m_pColliderCom)
		m_pColliderCom->Sync_Position(m_pTransformCom);

	if (m_pUpdateAnimStateFunc)
	{
		if(m_MeshTypeIndices.empty())
			m_pUpdateAnimStateFunc(m_strAnimationTag, m_pTransformCom->Get_WorldMatrix(), m_iInstanceIndex, &m_fTrackPos, nullptr);
		else
			m_pUpdateAnimStateFunc(m_strAnimationTag, m_pTransformCom->Get_WorldMatrix(), m_iInstanceIndex, &m_fTrackPos, m_MeshTypeIndices.data());
	}
}

void CDummyCell::Ready_Component(DUMMYCELL_DESC* pDesc)
{
	// Com_Collider
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 0.7f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NPC);
	ColliderDesc.fHeight = 1.5f;
	ColliderDesc.fRadius = 0.5f;
	ColliderDesc.fRayOffset = -0.15f;
	Add_Component(ENUM_CLASS(LEVEL::STATIC),TEXT("Prototype_Component_Collider"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);
}

CDummyCell* CDummyCell::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDummyCell* pInstance = new CDummyCell(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CDummyCell");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDummyCell::Clone(void* pArg)
{
	CDummyCell* pClone = new CDummyCell(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CDummyCell (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CDummyCell::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
}
