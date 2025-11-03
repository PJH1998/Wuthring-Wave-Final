#pragma once
#include "Client_Define.h"
#include "Character.h"
#include "Prop.h"

typedef struct tagPlayerSpec
{
	_wstring strActorTag = {};
    CCharacter::CHARACTER_DESC CharacterDesc{};
}PLAYER_SPEC;

typedef struct tagPartSpec
{
    _wstring strPartName;
    LEVEL eLevel;
    _wstring strPartPrototypeName;
}PART_SPEC;

namespace PlayerData
{

#pragma region AUGUSTA
    static CCharacter::CHARACTER_DESC GetAugustaCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
    {
        CCharacter::CHARACTER_DESC Desc;
        Desc.eCurLevel = eLevel;
        Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
        Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
        Desc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
        Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Augusta"));
        Desc.stateMachineData = make_pair(eLevel, TEXT("Prototype_Component_StateMachine_Augusta"));
        //Desc.controllerData = make_pair(eLevel, TEXT("Prototype_Component_Controller_Augusta"));
        Desc.fRotationPerSec = XMConvertToRadians(90.f);
        Desc.strFolderPath = "../Bin/Resource/Model/Player/Augusta/Notify/";
        Desc.strAbilityFolderPath = "../Bin/Resource/Model/Player/Augusta/Ability/"; // 스탯 정보 폴더.
        Desc.fSpeedPerSec = 10.f;
        Desc.vScale = vScale;
        Desc.vRotation = vRotation;
        Desc.vPosition = vPosition;
        Desc.eStat = { 100.f, 100.f, 0.f, 100.f, 0.f, 100.f, 0.f, 100.f };

        // Parts
        Desc.PartPrototypes = {
            make_pair(L"Bayonet", L"Prototype_GameObject_Augusta_Bayonet"),
            make_pair(L"SkillWeapon", L"Prototype_GameObject_Augusta_SkillWeapon"),
            make_pair(L"Griffon", L"Prototype_GameObject_Augusta_Griffon"),
			make_pair(L"Wing", L"Prototype_GameObject_Wing")
        };

        return Desc;
    }

    static CProp::PROP_DESC GetAugustaBayonetCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
    {
        CProp::PROP_DESC Desc{};
        Desc.fRotationPerSec = XMConvertToRadians(90.f);
        Desc.fSpeedPerSec = 10.f;
        Desc.pSocketMatrix = { nullptr }; // Augusta
        Desc.pParentTransform = { nullptr }; // Augusta
        Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
        Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
        Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Augusta_Bayonet"));
        Desc.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
        Desc.strFolderPath = "../Bin/Resource/Model/Player/Augusta/Weapon/Bayonet/Notify/";
        Desc.strBoneName = "WeaponProp02";
        //Desc.strBoneName = "WeaponProp05";
        Desc.eWeaponType = WEAPONTYPE::ANIM;
        Desc.vScale = vScale;
        Desc.vRotation = vRotation;
        Desc.vPosition = vPosition;
        return Desc;
    }

    static CProp::PROP_DESC GetAugustaSkillWeaponCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
    {
        CProp::PROP_DESC Desc{};
        Desc.fRotationPerSec = XMConvertToRadians(90.f);
        Desc.fSpeedPerSec = 10.f;
        Desc.pSocketMatrix = { nullptr }; // Augusta
        Desc.pParentTransform = { nullptr }; // Augusta
        Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
        Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
        Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Augusta_SkillWeapon"));
        Desc.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
        Desc.strBoneName = "WeaponProp02";
        Desc.eWeaponType = WEAPONTYPE::ANIM;
        Desc.vScale = vScale;
        Desc.vRotation = vRotation;
        Desc.vPosition = vPosition;
        return Desc;
    }

    static CProp::PROP_DESC GetAugustaGriffonCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
    {
        CProp::PROP_DESC Desc{};
        Desc.fRotationPerSec = XMConvertToRadians(90.f);
        Desc.fSpeedPerSec = 10.f;
        Desc.pSocketMatrix = { nullptr }; // Augusta���� ä������ϴ� ������.
        Desc.pParentTransform = { nullptr }; // Augusta���� ä������ϴ� ������.
        Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
        Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
        Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Augusta_Griffon"));
        Desc.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
        Desc.strBoneName = "WeaponProp02";
        Desc.eWeaponType = WEAPONTYPE::ANIM;
        Desc.vScale = vScale;
        Desc.vRotation = vRotation;
        Desc.vPosition = vPosition;
        return Desc;
    }
#pragma endregion

#pragma region ROVER
    static CCharacter::CHARACTER_DESC GetRoverCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
    {
        CCharacter::CHARACTER_DESC Desc;
        Desc.eCurLevel = eLevel;
        Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
        Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
        Desc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
        Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Rover"));
        Desc.stateMachineData = make_pair(eLevel, TEXT("Prototype_Component_StateMachine_Rover"));
        Desc.fRotationPerSec = XMConvertToRadians(90.f);
        Desc.strFolderPath = "../Bin/Resource/Model/Player/Rover/Notify/";
        Desc.fSpeedPerSec = 10.f;
        Desc.vScale = vScale;
        Desc.vRotation = vRotation;
        Desc.vPosition = vPosition;
        Desc.eStat = { 100.f, 100.f, 0.f, 100.f, 0.f, 100.f, 0.f, 100.f };

        Desc.PartPrototypes = {
            make_pair(L"Sword", L"Prototype_GameObject_Rover_Sword"),
			make_pair(L"Wing", L"Prototype_GameObject_Wing")
        };

        return Desc;
    }

    static CProp::PROP_DESC GetRoverWeaponCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
    {
        CProp::PROP_DESC Desc{};
        Desc.fRotationPerSec = XMConvertToRadians(90.f);
        Desc.fSpeedPerSec = 10.f;
        Desc.pSocketMatrix = { nullptr }; 
        Desc.pParentTransform = { nullptr };
        Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
        Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
        Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Rover_Sword"));
        Desc.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
        Desc.strFolderPath = "../Bin/Resource/Model/Player/Rover/Weapon/Sword/Notify/";
        Desc.strBoneName = "WeaponProp02";
        Desc.eWeaponType = WEAPONTYPE::ANIM;
        Desc.vScale = vScale;
        Desc.vRotation = vRotation;
        Desc.vPosition = vPosition;
        return Desc;
    }
#pragma endregion

#pragma region COMMON PROP
	static CProp::PROP_DESC GetWingCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
	{
		CProp::PROP_DESC Desc{};
		Desc.fRotationPerSec = XMConvertToRadians(90.f);
		Desc.fSpeedPerSec = 10.f;
		Desc.pSocketMatrix = { nullptr };
		Desc.pParentTransform = { nullptr };
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
		Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
		Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Wing"));
		Desc.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
		Desc.strFolderPath = "../Bin/Resource/Model/Player/Wing/Notify/";
		Desc.strBoneName = "WingCase";
		Desc.eWeaponType = WEAPONTYPE::ANIM;
		Desc.vScale = vScale;
		Desc.vRotation = vRotation;
		Desc.vPosition = vPosition;
		return Desc;
	}
#pragma endregion
}
