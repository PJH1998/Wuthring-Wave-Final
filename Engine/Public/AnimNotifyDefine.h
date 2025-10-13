#pragma once
#include "EnginePch.h"
// 어떤 태그를 활성화 하는지에 대해서만 지정 

typedef struct tagBaseNotifiy
{
	_float  fTrackPosition = 0.f;
	virtual void Execute() = 0;
}BASENOTIFY;

typedef struct tagSoundNotify : BASENOTIFY
{
	_string strSoundTag;
	_float fVolume = 0.1f;
}SOUNDNOTIFY;

typedef struct tagEffectNotify : BASENOTIFY
{
	// 1. Effect Type
	_string strEffectTag;

	// 2. Bone 사용 여부
	_bool IsUseBone = { false };
	_string strBoneName = "";

	// 3. 자세한 설정값은 은비가 설정합시다.. => 클래스 설계 따라 너무 달라짐.
	
}EFFECTNOTIFY;

typedef struct tagColliderNotify : BASENOTIFY
{
	// 1. 끄고 켜고 기능만 있으면 될듯.
	_string strColliderTag;
	_bool IsActive = { false }; 
}COLLIDERNOTIFY;

typedef struct tagLightNotify : BASENOTIFY
{
	// 1. 끄고 켜고 기능만 있으면 될듯.
	_string strLightTag;
	_bool IsActive = { false };
}LIGHTNOTIFY;
