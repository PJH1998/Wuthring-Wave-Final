#include "EnginePch.h"
#include "ContactListenerImpl.h"

#include "CollideComponent.h"

CContactListenerImpl::CContactListenerImpl()
{
}

CContactListenerImpl::CContactListenerImpl(BodyInterface* pInterface)
	: m_pBodyInterface { pInterface }
{
}

CContactListenerImpl::~CContactListenerImpl()
{
}

void CContactListenerImpl::Remove_Update()
{
	for (auto& Pair : m_RemoveIDs)
	{
		COLLISION_DATA* pSrcData = reinterpret_cast<COLLISION_DATA*>(m_pBodyInterface->GetUserData(Pair.first));
		COLLISION_DATA* pDstData = reinterpret_cast<COLLISION_DATA*>(m_pBodyInterface->GetUserData(Pair.second));

		ContactManifold Manifold;
		ZeroMemory(&Manifold, sizeof(ContactManifold));

		pSrcData->pComponent->OnCollide_Remove(m_pBodyInterface->GetObjectLayer(Pair.second), pDstData->pDesc, Manifold);
		pDstData->pComponent->OnCollide_Remove(m_pBodyInterface->GetObjectLayer(Pair.first), pSrcData->pDesc, Manifold);
	}
	m_RemoveIDs.clear();
}

ValidateResult CContactListenerImpl::OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult)
{
    return ValidateResult();
}

void CContactListenerImpl::OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	COLLISION_DATA* pSrcData = reinterpret_cast<COLLISION_DATA*>(inBody1.GetUserData());
	COLLISION_DATA* pDstData = reinterpret_cast<COLLISION_DATA*>(inBody2.GetUserData());

	pSrcData->pComponent->OnCollide_Enter(inBody2.GetObjectLayer(), pDstData->pDesc, inManifold);
	pDstData->pComponent->OnCollide_Enter(inBody1.GetObjectLayer(), pSrcData->pDesc, inManifold);
}

void CContactListenerImpl::OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	COLLISION_DATA* pSrcData = reinterpret_cast<COLLISION_DATA*>(inBody1.GetUserData());
	COLLISION_DATA* pDstData = reinterpret_cast<COLLISION_DATA*>(inBody2.GetUserData());

	pSrcData->pComponent->OnCollide_During(inBody2.GetObjectLayer(), pDstData->pDesc, inManifold);
	pDstData->pComponent->OnCollide_During(inBody1.GetObjectLayer(), pSrcData->pDesc, inManifold);
}

void CContactListenerImpl::OnContactRemoved(const SubShapeIDPair& inSubShapePair)
{
	pair<BodyID, BodyID> PairID;
	PairID.first = inSubShapePair.GetBody1ID();
	PairID.second = inSubShapePair.GetBody2ID();
	m_RemoveIDs.push_back(PairID);
}
