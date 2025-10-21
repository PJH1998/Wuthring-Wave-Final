#pragma once
#include "Client_Define.h"
#include "Player.h"

typedef struct tagPlayerSpec
{
	_wstring strActorTag = {};
    CPlayer::PLAYER_DESC PlayerDesc{};
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

    static CPlayer::PLAYER_DESC GetAugustaCloneData(_float3 vScale, _float3 vRotation, _float3 vPostion, LEVEL eLevel)
    {
        CPlayer::PLAYER_DESC Desc;
        Desc.eCurLevel = eLevel;
        Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
        Desc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
        Desc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
        Desc.modelData = make_pair(eLevel, TEXT("Prototype_Component_Model_Augusta"));
        Desc.fRotationPerSec = XMConvertToRadians(90.f);
        Desc.fSpeedPerSec = 10.f;
        Desc.vScale = vScale;
        Desc.vRotation = vRotation;
        Desc.vPostion = vPostion;
        Desc.eStat = { 100.f, 0.f, 100.f };
        

        // Desc.pController, pController는 런타임에 주입

        // Parts 정보
        Desc.PartPrototypes = {
            make_pair(L"Sword", L"Prototype_Weapon_Augusta_Sword"),
            make_pair(L"Shield", L"Prototype_Armor_Augusta_Shoulder")
        };

        return Desc;
    }
}
