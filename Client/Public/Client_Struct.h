#pragma once
#include "Engine_Define.h"
#include "Client_Enum.h"

namespace Engine
{
	class CTransform;
}

namespace Client
{
	
#pragma region CHARACTER
	typedef struct SLIDE_DATA
	{
		vector<_float3> WayPoints;
		_float fSpeed = 5.f;

		void Reset()
		{
			WayPoints.clear();
		}
	}SLIDE_DATA;

	typedef struct tagSkillInfo
	{
		_string strSkillName;   // 스킬 이름 (Key)
		_string strPrevName;    // 이전 스킬 이름 (체인용)

		_string strKeyInput;    // 키 입력 (enum으로 관리하면 더 좋음)
		_string strKeyType;		// 키 타입.

		SKILL_TYPE eSkillType;    // SKill 타입.
		_float  fCost;			// Cost 소모
		COST_TYPE eCostType;    // CostStat이 어떤 타입인지?
		_float  fCoolDown;      // 쿨타임 (숫자로 변환하여 저장)
		_float  fDamage;			// 스킬 데미지
		_string strDescription; // 설명
	}SKILL_INFO;

	typedef struct tagChracterInfo
	{
		_string strName;
		_float fHp = { 0.f };
		_float fMaxHp;
		_float fStamina = { 0.f };
		_float fMaxStamina;
		_float fAttack;		 // 기본 공격 값.
		_float fAttackAddMin;	 // 공격 최소 값.
		_float fAttackAddMax;	 // 공격 최대 값.
		_float fHarmonyGauge;       // 협주 게이지
		_float fMaxHarmonyGauge;    // 협주 게이지 최대값.
	}CHARACTER_INFO;

	// 스킬에 대한 Slot을 제공할것이니까 Cost는 상관 없음 State 다 결정해서 제공. 
	typedef struct tagUISkillSlot {
		_string strKeyInput;      // "LB", "E", "Q" 등
		_uint iCharacterType;	  // 캐릭터 타입. => Rover, Augusta, Galbrena => UI_CHARACTERTYPE
		_uint iStateType;		  // 캐릭터에 따른 State => Character Type을 확인하고 그에 맞게 캐스팅해서 사용. 
		// => iStateType은 UI_AUGUSTA_STATE 또는 ROVER_STATE GABRENA_STATE
		 // ex) AUGUSTA면 UI_AUGUSTA_STATE eState = static_cast<UI_AUGUSTA_STATE>(iStateType);

		_float  fCurrentCoolTime; // 현재 쿨타임.
		_float  fMaxCoolTime;     // Max 쿨타임
		_string  strSkillName; // Skill Name 디버그 용도.
	} UISKILL_SLOT;

	typedef struct tagDelayedAction {
		enum class TYPE { HIT, PARRY, DODGE, GRAB }; // 이벤트 타입.
		TYPE type;
		void* pData;  // HIT_DESC 등 데이터 (nullptr 가능)
		tagDelayedAction(TYPE t, void* data = nullptr) : type(t), pData(data) {}
	}DELAYED_ACTION;

	typedef struct tagGrappleInfo {
		CTransform* pTransform = { nullptr };
		OBJECTTYPE eObjectType;
		uint* pTriggerIndex = { nullptr };
		_bool IsActive = { false };
		void Reset()
		{
			pTransform = nullptr;
			eObjectType = OBJECTTYPE::END;
			pTriggerIndex = { nullptr };
			IsActive = { false };
		}
	}GRAPPLE_INFO;


	typedef struct tagThrowInfo {
		CTransform* pTransform = { nullptr };
		OBJECTTYPE eObjectType;
		_bool* pGrabbed = { nullptr }; // Grab (객체를 손으로 끌어오는 역할) => true 시 Throw false 로 설정.
		_bool* pThrow = { nullptr }; // Throw (객체를 손에서 던지는 역할) => true 시 Grab False로 설정.

		const _float4x4** ppRefBoneMatrix = { nullptr };
		const _float4x4** ppRefWorldMatrix = { nullptr };
		
		_bool IsActive = { false };
		void Reset()
		{
			pTransform = nullptr;
			pGrabbed = nullptr;
			pThrow = nullptr;
			ppRefBoneMatrix = nullptr;
			ppRefWorldMatrix = nullptr;
			eObjectType = OBJECTTYPE::END;
			IsActive = false;
		}
	}THROW_INFO;

	//typedef struct tagQTEInteraction
	//{
	//	
	//}QTE_INTERACTION;

#pragma endregion

	typedef struct tagSFX_RadialData {
		_float fMinDistance;
		_float fMaxDistance;
		_float fLengthScale;
		_float fPadding0;
		_float2 vPivot;
		_float2 fPadding1;
	}SFX_RADIAL_DATA;
	
	typedef struct tagSFX_SlashData {
		_float2 vSlashPoint0;
		_float2 vSlashPoint1;
		_float2 vScreenSize;
		_float fOffset;
		_float fIntensity;
	}SFX_SLASH_DATA;

	


	typedef struct tagMonsterInfo
	{
		_wstring wstrPoolTag;
		_wstring wstrUIText;
		_string strName;
		_uint	iMonsterID;			//참조용 몬스터ID
		_float	fMaxHp;
		_float	fMaxStamina;		// 거랑급 이상 무력화 게이지
		_float	fAttack;			// 기본 공격 값.
		_float	fAttackAddMin;		// 공격 최소 값.
		_float	fAttackAddMax;		// 공격 최대 값.
		_float	fImpluseRate;		// 넉백 보정 수치(몬스터 무게, 가벼울수록 높게)

	}MONSTER_INFO;

	typedef struct tagNPCInfo
	{
		_float3 vPosition;
		_float3 vRotation;
		_int MeshtypeIndices[3];	//몸통, 얼굴, 머리 인덱스
		_bool isCollide;
		_string strAnimTag;
	}NPCINFO;

	typedef struct tagScanInfo
	{
		_vector vCenterPos;
		_float fRadius;
	}SCAN_INFO;
	
	typedef struct tagCallBackClientDesc
	{
		void* pTransform = { nullptr };  // Transform;
		_float fAttack = { 0.f };			 // 공격력
		_uint* pCondition = {};			// 컨디션 Value
		_wstring strEffectTag = {};		// 호출할 이펙트 태그
		TEXT_COLOR_TYPE eType{};		// 공격자 속성
		ATTACKVOULME_DIR eDir{};
		OBJECTTYPE eObjectType{ OBJECTTYPE::END }; // 어떤 오브젝트인지 넣어서 판단하게. ANCHOR(고정), PULL(당긴다)
		// Shaking이나, HitStop? 이런 거.
		const _float4x4* pSocketMatrix = { nullptr }; // Grap 시 플레이어가 붙을 Matrix Pointer?

		SLIDE_DATA eSlideData;
		_bool		IsStart = { false };
		_bool*		IsGrab  = { nullptr };
		_bool*		IsThrow = { nullptr };
		const _float4x4** ppRefBoneMatrix = { nullptr };
		const _float4x4** ppRefWorldMatrix = { nullptr };

		_wstring strSoundTag = {};
	}CALLBACK_CLIENT;

	typedef struct tagTargetInfo
	{
		CTransform* pTransform = nullptr;
		const _float4x4* pSocketMatrix = nullptr;
		_bool IsActive = { false };
		// 초기화
		void Reset() { pTransform = nullptr; pSocketMatrix = nullptr; IsActive = false; }

		_bool operator== (const tagTargetInfo& target) const
		{
			return this->pTransform == target.pTransform && this->pSocketMatrix == target.pSocketMatrix;
		}
	}TARGET_INFO;

	

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

	typedef struct tagSceneCameraFrame {
		_float4			vQuaternion{};
		_float3			vPosition{};
		_float				fStartFrame{};
		_float				fFovy{};
		_bool				isLerp = { true };
	}SCENE_CAMERA_FRAME;

	typedef struct tagSQCameraData : public SEQUENCE_ITEM_DATA {
		vector<SCENE_CAMERA_FRAME> Frames;
		tagSQCameraData(_float _fStartFrame, _float _fEndFrame, _float _fTrackPerSec, const vector<SCENE_CAMERA_FRAME> _Frames)
			: SEQUENCE_ITEM_DATA{ _fStartFrame, _fEndFrame, _fTrackPerSec }
		{
			for (auto& pData : _Frames)
				Frames.push_back(pData);
			//memcpy(Frames.data(), _Frames.data(), sizeof(SCENE_CAMERA_FRAME) * _Frames.size());
		}
		virtual ~tagSQCameraData() {};
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


#pragma region UI

	typedef struct tUIMobsInfoDesc
	{
		void*	pMonsterPtrKey = {};		// 오브젝트의 고유한 키로써 사용할 목적

		_bool	isAtkedCurFrame = false;


		_float	fMobCurHP = 500.f;
		_float	fMobMaxHP = 500.f;

		_float3 vMobPos = _float3{ 0.f, -10.f, 0.f };

	} UI_MOBINFO_DESC;

#pragma endregion





}
