#include "EnginePch.h"
#include "CharacterContactListenerImpl.h"

CharacterContactListenerImpl::CharacterContactListenerImpl()
{
}

void CharacterContactListenerImpl::OnContactAdded(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	int a = 0;
}

void CharacterContactListenerImpl::OnContactPersisted(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	int a = 0;
}

void CharacterContactListenerImpl::OnContactRemoved(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2)
{
	int a = 0;
}

void CharacterContactListenerImpl::OnCharacterContactAdded(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	int a = 0;
}

void CharacterContactListenerImpl::OnCharacterContactPersisted(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	int a = 0;
}

void CharacterContactListenerImpl::OnCharacterContactRemoved(const CharacterVirtual* inCharacter, const CharacterID& inOtherCharacterID, const SubShapeID& inSubShapeID2)
{
	int a = 0;
}
