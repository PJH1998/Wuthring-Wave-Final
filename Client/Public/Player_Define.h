#pragma once
#include "Client_Define.h"
#include "Character.h"
#include "Weapon.h"

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
    static const _tchar* AUGUSTA_ACTOR_TAG = TEXT("Prototype_GameObject_Actor_Augusta");

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
        Desc.fSpeedPerSec = 10.f;
        Desc.vScale = vScale;
        Desc.vRotation = vRotation;
        Desc.vPosition = vPosition;
        Desc.eStat = { 100.f, 0.f, 100.f };
        

        // Desc.pController, pController는 런타임에 주입

        // Parts 정보
        Desc.PartPrototypes = {
            make_pair(L"Bayonet", L"Prototype_GameObject_Augusta_Bayonet"),
            make_pair(L"Shield", L"Prototype_Armor_Augusta_Shoulder")
        };

        return Desc;
    }

    static CWeapon::WEAPON_DESC GetAugustaBayonetCloneData(_float3 vScale, _float3 vRotation, _float3 vPosition, LEVEL eLevel)
    {
        CWeapon::WEAPON_DESC Desc{};
        Desc.fRotationPerSec = XMConvertToRadians(90.f);
        Desc.fSpeedPerSec = 10.f;
        Desc.pSocketMatrix = { nullptr }; // Augusta에서 채워줘야하는 데이터.
        Desc.pParentTransform = { nullptr }; // Augusta에서 채워줘야하는 데이터.
        Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
        Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
        Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Augusta_Bayonet"));
        Desc.rigidBodyData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Rigidbody"));
        Desc.strBoneName = "WeaponProp02";
        //Desc.strBoneName = "WeaponProp05";
        Desc.eWeaponType = WEAPONTYPE::ANIM;
        Desc.vScale = vScale;
        Desc.vRotation = vRotation;
        Desc.vPosition = vPosition;
        return Desc;
    }
}
