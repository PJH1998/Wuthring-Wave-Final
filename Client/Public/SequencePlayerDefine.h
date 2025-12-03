#pragma once
#include "Client_Define.h"
#include "Character.h"
#include "Prop.h"

NS_BEGIN(Client)
class CCharacter;
NS_END


typedef struct tagSequencePlayerSpec
{
	_wstring strActorTag = {};
	CCharacter::CHARACTER_DESC CharacterDesc{};
}SEQUENCEPLAYER_SPEC;

namespace SeqPlayerData
{
#pragma region YUNO
	static CCharacter::CHARACTER_DESC GetYunoCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
	{
		CCharacter::CHARACTER_DESC Desc;
		Desc.eCurLevel = eLevel;
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMeshCharacter"));
		Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshCharacter"));
		Desc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
		Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Yuno"));
		Desc.stateMachineData = make_pair(eLevel, TEXT("Prototype_Component_StateMachine_Yuno"));
		Desc.flyComputeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshFly"));
		Desc.facialComputeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMorph"));
		Desc.fRotationPerSec = XMConvertToRadians(90.f);
		Desc.strFolderPath = "../Bin/Resource/Model/SequencePlayer/YunoFacial/Notify/";
		Desc.fSpeedPerSec = 10.f;
		Desc.vScale = vScale;
		Desc.vRotation = vRotation;
		Desc.vPosition = vPosition;

		Desc.PartPrototypes = {
		   make_pair(L"Moon", L"Prototype_GameObject_Yuno_Moon")
		};
	}

	static CProp::PROP_DESC GetYunoMoonCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
	{
		CProp::PROP_DESC Desc{};
		Desc.fRotationPerSec = XMConvertToRadians(90.f);
		Desc.fSpeedPerSec = 10.f;
		Desc.pSocketMatrix = { nullptr }; // Moon
		Desc.pParentTransform = { nullptr }; // Moon
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxPropAnimMesh"));
		Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
		Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Yuno_Moon"));
		Desc.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
		Desc.strFolderPath = "../Bin/Resource/Model/SequencePlayer/YunoFacial/Weapon/Moon/Notify/";
		Desc.strBoneName = "WeaponProp03";
		Desc.eWeaponType = WEAPONTYPE::ANIM;
		Desc.vScale = vScale;
		Desc.vRotation = vRotation;
		Desc.vPosition = vPosition;
		return Desc;
	}

#pragma endregion


}