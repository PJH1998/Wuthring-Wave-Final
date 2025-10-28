#include "EnginePch.h"
#include "Collider.h"
#include "GameInstance.h"

#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h"

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

	_vector vLerpPos = XMVectorLerp(pTransform->Get_State(STATE::POSITION), XMVectorSet(vPos.GetX(), vPos.GetY(), vPos.GetZ(), 1.f), 0.15f);

	//pTransform->Set_State(STATE::POSITION, XMVectorSet(vPos.GetX(), vPos.GetY(), vPos.GetZ(), 1.f));
	pTransform->Set_State(STATE::POSITION, XMVectorSetW(vLerpPos, 1.f));
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
	m_isClone = true;
	ASSERT_CRASH(pArg);

	COLLIDER_DESC* pDesc = static_cast<COLLIDER_DESC*>(pArg);
	m_iCollisionLayer = pDesc->iLayer;

	// Create Shape
	using namespace JPH;
	
	m_pShape = new CapsuleShape(pDesc->fHeight * 0.5f, pDesc->fRadius);
	ASSERT_CRASH(m_pShape);

	m_vOffset = pDesc->vOffset;
	// Virtual Setting
	CharacterVirtualSettings VirtualSetting;
	//VirtualSetting.mMaxSlopeAngle = XMConvertToRadians(89.9f);
	VirtualSetting.mMaxSlopeAngle = XMConvertToRadians(120.f);			// 허용 경사 각도
	VirtualSetting.mShape = m_pShape;											// Character Virtual Shape
	VirtualSetting.mShapeOffset = LoadVec3(m_vOffset);						// Shape Offset
	VirtualSetting.mMaxStrength = 10.f;											// 다른 Body를 밀 수 있는 최대 힘
	VirtualSetting.mCharacterPadding = 0.02f;									// (충돌 범위 Padding) => 여유 주는듯?
	VirtualSetting.mPenetrationRecoverySpeed = 0.f;							// 겹쳤을 때 복원 속도
	VirtualSetting.mPredictiveContactDistance = 0.02f;							// 미리 충돌 감지하는 범위
	VirtualSetting.mEnhancedInternalEdgeRemoval = true;					// 각진 부분 부드럽게
	
	VirtualSetting.mInnerBodyShape = m_pShape;
	VirtualSetting.mInnerBodyLayer = ObjectLayer(pDesc->iLayer);
	
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
		Velocity += XMVectorSet(0.f, -9.81f, 0.f, 0.f) * 0.7f;
	else
		Slide(Velocity);

	m_pCharacterVirtual->SetLinearVelocity(Velocity);
	m_pGameInstance->Add_Virtual(m_pCharacterVirtual, m_iCollisionLayer);
}

HRESULT CCollider::Render()
{
#ifdef _DEBUG
	if (nullptr != m_pShape)
	{
		RMat44 Matrix = RMat44::sIdentity();
		Vec3 vPos = m_pCharacterVirtual->GetPosition() + m_pCharacterVirtual->GetShapeOffset();
		Matrix.SetColumn4(3, Vec4(vPos.GetX(), vPos.GetY(), vPos.GetZ(), 1.f));
		m_pGameInstance->DrawShape(m_pShape, Matrix);
	}
#endif
    return S_OK;
}

Vec3 CCollider::Slide(const Vec3& Velocity)
{
	_vector vGroundNormal = XMVector3Normalize(StoreVector3(m_pCharacterVirtual->GetGroundNormal()));

	_float fDot = XMVectorGetX(XMVector3Dot(XMVectorSet(0.f, 1.f, 0.f, 0.f), vGroundNormal));
	if (fDot < XMConvertToRadians(70.f))
		return Velocity;

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
	if (true == m_isClone)
	{
		m_pGameInstance->Remove_Virtual(m_pCharacterVirtual);
		m_pCharacterVirtual = nullptr;
		m_pShape = nullptr;
	}

	__super::Free();

	m_tCollisionData.pComponent = nullptr;
	m_tCollisionData.pDesc = nullptr;
}
