#pragma once
#include "Engine_Define.h"

class CharacterContactListenerImpl : public CharacterContactListener
{
public:
	explicit CharacterContactListenerImpl();
	virtual ~CharacterContactListenerImpl() = default;
	// 캐릭터가 인식하는 물체의 속도를 조정할 때 사용
	// 컨베이어 벨트 위에 있을 때 캐릭터가 벨트 이동속도를 반영해 움직일 때 사용
	virtual void						OnAdjustBodyVelocity(const CharacterVirtual* inCharacter, const Body& inBody2, Vec3& ioLinearVelocity, Vec3& ioAngularVelocity) { /* Do nothing, the linear and angular velocity are already filled in */ }
	// 특정 Body와 충돌 시, True 반환
	virtual _bool						OnContactValidate(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2) { return true; }
	// 특정 Character와 충돌 시, True 반환
	virtual _bool						OnCharacterContactValidate(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2) { return true; }
	
	// Character가 Body와 충돌 Begin시 호출
	virtual void						OnContactAdded(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings) override;
	// Character가 Body와 충돌 OnGoing시 호출
	virtual void						OnContactPersisted(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings) override;
	// Character가 Body와 충돌 End시 호출
	virtual void						OnContactRemoved(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2) override;

	// Character가 Character와 충돌 Begin시 호출
	virtual void						OnCharacterContactAdded(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings) override;
	// Character가 Character와 충돌 OnGoing시 호출
	virtual void						OnCharacterContactPersisted(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings) override;
	// Character가 Character와 충돌 End시 호출
	virtual void						OnCharacterContactRemoved(const CharacterVirtual* inCharacter, const CharacterID& inOtherCharacterID, const SubShapeID& inSubShapeID2) override;

	// Character와 Body가 충돌 했을 때, 물리 연산 시 들어오는 함수
	virtual void						OnContactSolve(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, Vec3Arg inContactVelocity, const PhysicsMaterial* inContactMaterial, Vec3Arg inCharacterVelocity, Vec3& ioNewCharacterVelocity) { /* Default do nothing */ }
	// Character와 Character가 충돌 했을 때, 물리 연산 시 들어오는 함수
	virtual void						OnCharacterContactSolve(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, Vec3Arg inContactVelocity, const PhysicsMaterial* inContactMaterial, Vec3Arg inCharacterVelocity, Vec3& ioNewCharacterVelocity) { /* Default do nothing */ }
};

