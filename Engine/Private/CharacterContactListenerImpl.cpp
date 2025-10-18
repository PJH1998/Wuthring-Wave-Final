#include "EnginePch.h"
#include "CharacterContactListenerImpl.h"

CharacterContactListenerImpl::CharacterContactListenerImpl()
{
}

CharacterContactListenerImpl::CharacterContactListenerImpl(BodyInterface* pInterface)
	: m_pBodyInterface { pInterface }
{
}

void CharacterContactListenerImpl::OnContactAdded(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	if(EMotionType::Static ==  m_pBodyInterface->GetMotionType(inBodyID2))
		ioSettings.mCanPushCharacter = true;
	else
		ioSettings.mCanPushCharacter = false;
}

void CharacterContactListenerImpl::OnContactPersisted(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	if (EMotionType::Static == m_pBodyInterface->GetMotionType(inBodyID2))
		ioSettings.mCanPushCharacter = true;
	else
		ioSettings.mCanPushCharacter = false;
}

void CharacterContactListenerImpl::OnContactRemoved(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2)
{
	int a = 0;
}

void CharacterContactListenerImpl::OnCharacterContactAdded(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	ioSettings.mCanPushCharacter = false;
}

void CharacterContactListenerImpl::OnCharacterContactPersisted(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	ioSettings.mCanPushCharacter = false;
}

void CharacterContactListenerImpl::OnCharacterContactRemoved(const CharacterVirtual* inCharacter, const CharacterID& inOtherCharacterID, const SubShapeID& inSubShapeID2)
{
	int a = 0;
}
