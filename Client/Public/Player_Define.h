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
        Desc.flyComputeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshFly"));
        //Desc.abilityData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Ability"));
        Desc.fRotationPerSec = XMConvertToRadians(90.f);
        Desc.strFolderPath = "../Bin/Resource/Model/Player/Augusta/Notify/";
        //Desc.strAbilityFolderPath = "../Bin/Resource/Model/Player/Augusta/Ability/"; // 스탯 정보 폴더.
        Desc.fSpeedPerSec = 10.f;
        Desc.vScale = vScale;
        Desc.vRotation = vRotation;
        Desc.vPosition = vPosition;

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
        //Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxPropAnimMesh"));
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
        //Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxPropAnimMesh"));
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
        Desc.pSocketMatrix = { nullptr }; 
        Desc.pParentTransform = { nullptr };
        //Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxPropAnimMesh"));
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
		Desc.flyComputeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshFly"));
        Desc.fRotationPerSec = XMConvertToRadians(90.f);
        Desc.strFolderPath = "../Bin/Resource/Model/Player/Rover/Notify/";
        Desc.fSpeedPerSec = 10.f;
        Desc.vScale = vScale;
        Desc.vRotation = vRotation;
        Desc.vPosition = vPosition;
        //Desc.eStat = { 100.f, 100.f, 0.f, 100.f, 0.f, 100.f, 0.f, 100.f };

        Desc.PartPrototypes = {
            make_pair(L"Sword", L"Prototype_GameObject_Rover_Sword"),
			make_pair(L"DarkWing", L"Prototype_GameObject_Rover_DarkWing"),
			make_pair(L"DarkScythe", L"Prototype_GameObject_Rover_DarkScythe"),
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
        //Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxPropAnimMesh"));
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

	static CProp::PROP_DESC GetRoverDarkWingCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
	{
		CProp::PROP_DESC Desc{};
		Desc.fRotationPerSec = XMConvertToRadians(90.f);
		Desc.fSpeedPerSec = 10.f;
		Desc.pSocketMatrix = { nullptr };
		Desc.pParentTransform = { nullptr };
		//Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxPropAnimMesh"));
		Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
		Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Rover_DarkWing"));
		Desc.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
		Desc.strFolderPath = "../Bin/Resource/Model/Player/Rover/Weapon/DarkWing/Notify/";
		Desc.strBoneName = "WingCase";
		Desc.eWeaponType = WEAPONTYPE::ANIM;
		Desc.vScale = vScale;
		Desc.vRotation = vRotation;
		Desc.vPosition = vPosition;
		return Desc;
	}

	static CProp::PROP_DESC GetRoverDarkScytheCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
	{
		CProp::PROP_DESC Desc{};
		Desc.fRotationPerSec = XMConvertToRadians(90.f);
		Desc.fSpeedPerSec = 10.f;
		Desc.pSocketMatrix = { nullptr };
		Desc.pParentTransform = { nullptr };
		//Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxPropAnimMesh"));
		Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
		Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Rover_DarkScythe"));
		Desc.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
		Desc.strFolderPath = "../Bin/Resource/Model/Player/Rover/Weapon/DarkScythe/Notify/";
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

#pragma region GALBRENA
	static CCharacter::CHARACTER_DESC GetGalbrenaCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
	{
		CCharacter::CHARACTER_DESC Desc;
		Desc.eCurLevel = eLevel;
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
		Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
		Desc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
		Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Galbrena"));
		Desc.stateMachineData = make_pair(eLevel, TEXT("Prototype_Component_StateMachine_Galbrena"));
		Desc.flyComputeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshFly"));
		Desc.fRotationPerSec = XMConvertToRadians(90.f);
		Desc.strFolderPath = "../Bin/Resource/Model/Player/Galbrena/Notify/";
		Desc.fSpeedPerSec = 10.f;
		Desc.vScale = vScale;
		Desc.vRotation = vRotation;
		Desc.vPosition = vPosition;

		// Parts
		Desc.PartPrototypes = {
			make_pair(L"FirstGun", L"Prototype_GameObject_Galbrena_FirstGun"),
			make_pair(L"SecondGun", L"Prototype_GameObject_Galbrena_SecondGun"),
			make_pair(L"Lion", L"Prototype_GameObject_Galbrena_Lion"),
			make_pair(L"Wing", L"Prototype_GameObject_Wing")
		};

		return Desc;
	}

	static CProp::PROP_DESC GetGalbrenaFirstShotGunCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
	{
		CProp::PROP_DESC Desc{};
		Desc.fRotationPerSec = XMConvertToRadians(90.f);
		Desc.fSpeedPerSec = 10.f;
		Desc.pSocketMatrix = { nullptr }; // Augusta
		Desc.pParentTransform = { nullptr }; // Augusta
		//Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxPropAnimMesh"));
		Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
		Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Galbrena_ShotGun"));
		Desc.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
		Desc.strFolderPath = "../Bin/Resource/Model/Player/Galbrena/Weapon/ShotGun/Notify/";
		Desc.strBoneName = "WeaponProp01";
		Desc.eWeaponType = WEAPONTYPE::ANIM;
		Desc.vScale = vScale;
		Desc.vRotation = vRotation;
		Desc.vPosition = vPosition;
		return Desc;
	}

	static CProp::PROP_DESC GetGalbrenaSecondShotGunCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
	{
		CProp::PROP_DESC Desc{};
		Desc.fRotationPerSec = XMConvertToRadians(90.f);
		Desc.fSpeedPerSec = 10.f;
		Desc.pSocketMatrix = { nullptr }; // Augusta
		Desc.pParentTransform = { nullptr }; // Augusta
		//Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxPropAnimMesh"));
		Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
		Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Galbrena_ShotGun"));
		Desc.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
		Desc.strFolderPath = "../Bin/Resource/Model/Player/Galbrena/Weapon/ShotGun/Notify/";
		Desc.strBoneName = "WeaponProp02";
		Desc.eWeaponType = WEAPONTYPE::ANIM;
		Desc.vScale = vScale;
		Desc.vRotation = vRotation;
		Desc.vPosition = vPosition;
		return Desc;
	}
#pragma endregion

#pragma region LOGO
	static CCharacter::CHARACTER_DESC GetLogoMaleRoverCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
	{
		CCharacter::CHARACTER_DESC Desc;
		Desc.eCurLevel = eLevel;
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
		Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
		Desc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
		Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_MaleRover"));
		Desc.stateMachineData = make_pair(eLevel, TEXT("Prototype_Component_StateMachine_MaleRover"));
		Desc.flyComputeShaderData = {};
		Desc.fRotationPerSec = XMConvertToRadians(90.f);
		Desc.strFolderPath = "../Bin/Resource/Model/Player/Logo/Male/Notify/";
		Desc.fSpeedPerSec = 10.f;
		Desc.vScale = vScale;
		Desc.vRotation = vRotation;
		Desc.vPosition = vPosition;
		Desc.eStat = { 100.f, 100.f, 0.f, 100.f, 0.f, 100.f, 0.f, 100.f };

		Desc.PartPrototypes = {};

		return Desc;
	}
	
	static CCharacter::CHARACTER_DESC GetLogoFemaleRoverCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
	{
		CCharacter::CHARACTER_DESC Desc;
		Desc.eCurLevel = eLevel;
		Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
		Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
		Desc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
		Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_FemaleRover"));
		Desc.stateMachineData = make_pair(eLevel, TEXT("Prototype_Component_StateMachine_FemaleRover"));
		Desc.flyComputeShaderData = {};
		Desc.fRotationPerSec = XMConvertToRadians(90.f);
		Desc.strFolderPath = "../Bin/Resource/Model/Player/Logo/Female/Notify/";
		Desc.fSpeedPerSec = 10.f;
		Desc.vScale = vScale;
		Desc.vRotation = vRotation;
		Desc.vPosition = vPosition;
		Desc.eStat = { 100.f, 100.f, 0.f, 100.f, 0.f, 100.f, 0.f, 100.f };

		Desc.PartPrototypes = {};

		return Desc;
	}
#pragma endregion




}
