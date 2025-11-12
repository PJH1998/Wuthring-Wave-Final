#pragma once
#include "Engine_Define.h"
#include "Client_Enum.h"

namespace Client
{

	typedef struct tagCharacterStat
	{
		// Player Resources
		_float	fMaxHP;
		_float	fCurHP;		// 현재 체력

		_float	fMaxEnergy;	// 최대 에너지(공명 회로 게이지)
		_float	fCurEnergy;	// 현재 에너지

		_float	fPlayerMaxElem;		// 협주 에너지 최대 수치. (왼쪽 원소모양 둥글게 돌아가는 그것)
		_float	fPlayerCurElem;		// 협주 에너지 현재 수치.

		_float	fPlayerMaxSkillCD[2] = {};	// 플레이어의 스킬 최대 쿨타임 수치. 인덱스는 [캐릭터 인덱스][스킬 E, R] 을 의미
		_float	fPlayerCurSkillCD[2] = {};	// 플레이어의 스킬 현재 쿨타임 수치. 인덱스는 [캐릭터 인덱스][스킬 E, R] 을 의미

		_float	fPlayerMaxChangeCD[2] = {};	// 플레이어의 전환 최대 쿨타임 수치.
		_float	fPlayerCurChangeCD[2] = {};	// 플레이어의 전환 현재 쿨타임 수치.

		_float	fPlayerMaxUlt = {};			// [아직 구현 X] 플레이어의 궁극기(공명해방) 자원 수치.
		_float	fPlayerCurUlt = {};			// [아직 구현 X] 플레이어의 궁극기(공명해방) 자원 수치.

		// Player Extra Resources
	}CHARACTER_STAT;

	/*
		_float	fExtraMaxEnergy_Galbrena = {};		// 갈브레나용 특수(에코) 게이지 최대 수치. 50으로 상정하였음
		_float	fExtraMinEnergy_Galbrena = {};		// 갈브레나용 특수(에코) 게이지 현재 수치. 10 단위로 오름을 상정하였음.
		_float	fExtraMaxEnergy_Galbrena = {};		// [아직 구현 X] 갈브레나용 특수(악마의자격 중 하단 대쉬 유지 게이지) 최대 수치. 
		_float	fExtraMinEnergy_Galbrena = {};		// [아직 구현 X] 갈브레나용 특수(악마의자격 중 하단 대쉬 유지 게이지) 현재 수치. 

		_float	fExtraMaxEnergy_AugustaUlt = {};	// 아우구스타용 특수(궁극기) 게이지 최대 수치. 100으로 상정하였음. 
		_float	fExtraMinEnergy_AugustaUlt = {};	// 아우구스타용 특수(궁극기) 게이지 현재 수치. 100/7 단위로 오름을 상정하였음.
		_float	fExtraMaxEnergy_AugustaPoint = {};	// 아우구스타용 특수(원형) 게이지 최대 수치. 100으로 상정하였음. 
		_float	fExtraMinEnergy_AugustaPoint = {};	// 아우구스타용 특수(원형) 게이지 현재 수치.
		_uint	iExtraMaxEnergy_AugustaSword = {};	// 아우구스타용 특수(칼) 게이지 현재 갯수. 2라고 상정하였음.
		_uint	iExtraMinEnergy_AugustaSword = {};	// 아우구스타용 특수(칼) 게이지 현재 갯수. 
	*/

	typedef struct tagPlayerStat
	{
		// Player Index (Selected)
		_uint	iSelectedCharIndex = 0;		// 현재 선택한 캐릭터의 인덱스.		일단은 0 방랑자 / 1 아우구스타 / 2 갈브레나를 상정하였음
		_uint	iPlayerMode = {};			// 플레이어의 변신 모드.	기본 0 / 방랑자는 다크서지, 아우구스타는 궁극기 사용, 갈브레나는 악마의자격 진입 상태를 1로 상정하였음
		
		CHARACTER_STAT m_CharacterStat[3] = {};
	}PLAYER_STAT;


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


	typedef struct tagMonsterInfo
	{
		_wstring wstrPoolTag;
		_string strName;
		_uint	iMonsterID;			//참조용 몬스터ID
		_float	fMaxHp;
		_float	fMaxStamina;		// 거랑급 이상 무력화 게이지
		_float	fAttack;			// 기본 공격 값.
		_float	fAttackAddMin;		// 공격 최소 값.
		_float	fAttackAddMax;		// 공격 최대 값.
		_float	fImpluseRate;		// 넉백 보정 수치(몬스터 무게, 가벼울수록 높게)

	}MONSTER_INFO;
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
	
	typedef struct tagCallBackClientDesc
	{
		void* pTransform = { nullptr };  // Transform;
		_float fAttack = { 0.f };			 // 공격력
		_uint* pCondition = {};			// 컨디션 Value
		_string strEffectTag = {};		// 호출할 이펙트 태그
		// Shaking이나, HitStop? 이런 거.
	}CALLBACK_CLIENT;

	typedef struct tagDelayedAction {
		enum class TYPE { HIT, PARRY, DODGE }; // 이벤트 타입.
		TYPE type;
		void* pData;  // HIT_DESC 등 데이터 (nullptr 가능)
		tagDelayedAction(TYPE t, void* data = nullptr) : type(t), pData(data) {}
	}DELAYED_ACTION;
}
