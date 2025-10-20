#pragma once
#include "Player.h"

typedef struct tagPlayerSpec
{
	_wstring strActorTag = {};
	_wstring strShaderTag = {};
	_wstring strComputeShaderTag = {};
	_wstring strModelTag = {};
	_uint iShaderPath = {};
}PLAYER_SPEC;

namespace PlayerData
{
    static CPlayer::PLAYER_DESC GetAugustaData()
    {
        CPlayer::PLAYER_DESC Desc;

        // --- CActor::ACTOR_DESC (부모의 부모) ---
        Desc.strComputeShaderTag = TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh");
        Desc.strShaderTag = TEXT("Prototype_Component_Shader_VtxAnimMesh");
        Desc.strModelTag = TEXT("Prototype_Component_Model_Augusta");
        Desc.iShaderPath = ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX);
        Desc.fRotationPerSec = XMConvertToRadians(90.f);
        Desc.fSpeedPerSec = 10.f;
        // (eCurLevel, pController, wStrDataTag 등은 런타임에 주입됨)

        // --- AUGUSTA_DESC (자신) ---
        Desc.m_PartPrototypeTags = {
            L"Prototype_Weapon_Augusta_Sword",
            L"Prototype_Armor_Augusta_Shoulder"
        };

        return Desc;
    }
}
