#include "EnginePch.h"
#include "Collider.h"
#include "GameInstance.h"

#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"

CCollider::CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{
}

CCollider::CCollider(const CCollider& Prototype)
	: CComponent { Prototype }
{
}

void CCollider::Sync_Position(CTransform* pTransform)
{
	Vec3 vPos = m_pCharacterVirtual->GetPosition();
	pTransform->Set_State(STATE::POSITION, XMVectorSet(vPos.GetX(), vPos.GetY(), vPos.GetZ(), 1.f));
}

HRESULT CCollider::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCollider::Initialize_Clone(void* pArg)
{
	ASSERT_CRASH(pArg);

	COLLIDER_DESC* pDesc = static_cast<COLLIDER_DESC*>(pArg);
	m_pOwner = pDesc->pOwner;
	m_iCollisionLayer = pDesc->iLayer;

	RefConst<Shape> BodyShape;

	using namespace JPH;
	BodyShape = new CapsuleShape(pDesc->fHeight * 0.5f, pDesc->fRadius);
	ASSERT_CRASH(BodyShape);

	CharacterVirtualSettings VirtualSetting;
	VirtualSetting.mShape = BodyShape;
	VirtualSetting.mInnerBodyLayer = ObjectLayer(pDesc->iLayer);
	VirtualSetting.mInnerBodyShape = BodyShape;

	m_pCharacterVirtual = m_pGameInstance->Register_Virtual(VirtualSetting, LoadVec3(pDesc->vPos), LoadQuat(pDesc->vQuat), m_pOwner);
	ASSERT_CRASH(m_pCharacterVirtual);

	m_pCharacterVirtual->SetUserData(reinterpret_cast<uint64>(m_pOwner));

    return S_OK;
}

void CCollider::Update(CTransform* pTransform)
{
	m_pCharacterVirtual->SetPosition(LoadVec3(pTransform->Get_State(STATE::POSITION)));
	m_pGameInstance->Add_Virtual(m_pCharacterVirtual, m_iCollisionLayer);
}

HRESULT CCollider::Render()
{
    return S_OK;
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
	__super::Free();

	Safe_Delete(m_pCharacterVirtual);
	m_pOwner = nullptr;
}
