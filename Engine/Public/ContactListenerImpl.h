#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

class CContactListenerImpl : public ContactListener
{
public:
	explicit CContactListenerImpl();
	virtual ~CContactListenerImpl();

public:
	virtual		ValidateResult	OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override;
	virtual		void				OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override;
	virtual		void				OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override;
	virtual		void				OnContactRemoved(const SubShapeIDPair& inSubShapePair) override;
};

NS_END