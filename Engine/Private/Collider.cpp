#include "EnginePch.h"
#include "Collider.h"
#include "GameInstance.h"

#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"

CCollider::CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCollideComponent { pDevice, pContext }
{
}

CCollider::CCollider(const CCollider& Prototype)
	: CCollideComponent{ Prototype }
{
}

void CCollider::Sync_Position(CTransform* pTransform)
{
	Vec3 vPos = m_pCharacterVirtual->GetPosition();
	pTransform->Set_State(STATE::POSITION, XMVectorSet(vPos.GetX(), vPos.GetY(), vPos.GetZ(), 1.f));
}

_bool CCollider::IsLand(_float3* pNormalOut)
{
	if (nullptr == m_pCharacterVirtual)
		return false;

	if (nullptr != pNormalOut)
		*pNormalOut = StoreFloat3(m_pCharacterVirtual->GetGroundNormal());

	return m_pCharacterVirtual->IsSupported();
}

HRESULT CCollider::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCollider::Initialize_Clone(void* pArg)
{
	ASSERT_CRASH(pArg);

	COLLIDER_DESC* pDesc = static_cast<COLLIDER_DESC*>(pArg);
	m_iCollisionLayer = pDesc->iLayer;

	RefConst<Shape> BodyShape;

	// Create Shape
	using namespace JPH;
	BodyShape = new CapsuleShape(pDesc->fHeight * 0.5f, pDesc->fRadius);
	ASSERT_CRASH(BodyShape);

	// SetUp CharacterVitual
	CharacterVirtualSettings VirtualSetting;
	VirtualSetting.mShape = BodyShape;
	VirtualSetting.mInnerBodyLayer = ObjectLayer(pDesc->iLayer);
	VirtualSetting.mInnerBodyShape = BodyShape;
	VirtualSetting.mMaxSlopeAngle = XMConvertToRadians(89.9f);

	// Create CharacterVirtual
	m_tCollisionData.pComponent = this;
	m_pCharacterVirtual = m_pGameInstance->Register_Virtual(VirtualSetting, LoadVec3(pDesc->vPos), LoadQuat(pDesc->vQuat), &m_tCollisionData);
	ASSERT_CRASH(m_pCharacterVirtual);

    return S_OK;
}

void CCollider::Update(const _fvector& vVelocity)
{
	Vec3 Velocity = LoadVec3(vVelocity);
	if (false == m_pCharacterVirtual->IsSupported() && true == m_isGravity)
		Velocity += XMVectorSet(0.f, -9.81f, 0.f, 0.f);
	else
		Slide(Velocity);

	m_pCharacterVirtual->SetLinearVelocity(Velocity);
	m_pGameInstance->Add_Virtual(m_pCharacterVirtual, m_iCollisionLayer);
}

HRESULT CCollider::Render()
{
    return S_OK;
}

Vec3 CCollider::Slide(const Vec3& Velocity)
{
	_vector vGroundNormal = XMVector3Normalize(StoreVector3(m_pCharacterVirtual->GetGroundNormal()));

	_vector vVelocity = StoreVector3(Velocity);
	
	_float fLength = XMVectorGetX(XMVector3Dot(vVelocity, vGroundNormal));

	_vector vSlide = vVelocity + -1.f * vGroundNormal * fLength;

	return LoadVec3(vSlide);
}

CCollider* CCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCollider* pInstance = new CCollider(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Collider");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CCollider::Clone(void* pArg)
{
	CCollider* pClone = new CCollider(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Collider (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CCollider::Free()
{
	m_pGameInstance->Remove_Virtual(m_pCharacterVirtual);

	__super::Free();

	m_pCharacterVirtual = nullptr;
	m_tCollisionData.pComponent = nullptr;
	m_tCollisionData.pDesc = nullptr;
}
