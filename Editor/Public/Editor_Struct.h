#pragma once

namespace Editor
{
#pragma region EFFECT
	typedef struct tagEffectActorDesc
	{
		class CAnimationActor* pAnimActor = { nullptr }; // Animation 객체 주소
		float fDuration = {}; // 현재선택한 Animation Duration
	}EFFECTACTOR_DESC;
#pragma endregion

#pragma region SEQUENCE
	typedef struct tagAnimData {
		_float3			vScale{};
		_float4			vQuat{};
		_float3			vTranslation{};
		_string			strAnimation;
	}ANIM_DATA;
	typedef struct tagSQActorData : public SEQUENCE_ITEM_DATA {
		_wstring						strActorTag;
		vector<ANIM_DATA>		strAnimDatas;
	}SQ_ACTOR_DATA;

	typedef struct tagSQCameraData : public SEQUENCE_ITEM_DATA {
		vector<CAMERA_FRAME> Frames;
	}SQ_CAMERA_DATA;

	typedef struct tagSQAudioData : public SEQUENCE_ITEM_DATA {
		_wstring				strSoundTag;
		_float					fVolume;
		_bool					isBGM;
	}SQ_AUDIO_DATA;

	typedef struct tagSQEffectData : public SEQUENCE_ITEM_DATA {
		_wstring				strEffectTag;
		_float3				vScale{};
		_float4				vQuat{};
		_float3				vTranslation{};
		// TODO
	}SQ_EFFECT_DATA;

	typedef struct tagSQSFXData : public SEQUENCE_ITEM_DATA {
		SFX_TYPE			eSFXType;
	}SQ_SFX_DATA;
#pragma endregion

}
