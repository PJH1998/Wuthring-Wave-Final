#include "EnginePch.h"
#include "ContactListenerImpl.h"

#include "GameObject.h"

CContactListenerImpl::CContactListenerImpl()
{
}

CContactListenerImpl::~CContactListenerImpl()
{
}

ValidateResult CContactListenerImpl::OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult)
{
    return ValidateResult();
}

void CContactListenerImpl::OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	CGameObject* pSrc = reinterpret_cast<CGameObject*>(inBody1.GetUserData());
	CGameObject* pDst = reinterpret_cast<CGameObject*>(inBody2.GetUserData());

	if(nullptr != pSrc)
		pSrc->OnCollide_Enter(inBody2.GetObjectLayer(), pDst, inManifold);
	if(nullptr != pDst)
		pDst->OnCollide_Enter(inBody1.GetObjectLayer(), pSrc, inManifold);
}

void CContactListenerImpl::OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	CGameObject* pSrc = reinterpret_cast<CGameObject*>(inBody1.GetUserData());
	CGameObject* pDst = reinterpret_cast<CGameObject*>(inBody2.GetUserData());

	if (nullptr != pSrc)
		pSrc->OnCollide_OnGoing(inBody2.GetObjectLayer(), pDst, inManifold);
	if (nullptr != pDst)
		pDst->OnCollide_OnGoing(inBody1.GetObjectLayer(), pSrc, inManifold);
}

void CContactListenerImpl::OnContactRemoved(const SubShapeIDPair& inSubShapePair)
{
	
}
