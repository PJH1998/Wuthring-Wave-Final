#pragma once

namespace Client
{
	enum class LEVEL { STATIC, LOGO, GAMEPLAY, LOADING, TEST, TEST_UI, END };
	enum class CHANNEL { BGM, PLAYER_ACTION, PLAYER_VOICE, ENEMY_ACTION, ENEMY_VOICE, EFFECT, END };
	enum class COLLISIONLAYER { NONE, MAP, PLAYER, ATTACK, SKILL, KNOCKBACK, ENEMY, ENEMY_ATTACK, ENEMY_HARDATTACK, ENEMY_SKILL, INTERACTION, DETECT, PARRY, GRAB, SPAWN, END };

	enum class SKILLBTN { LBTN, T, E, R, END };
	enum class SKILLICONID { DEFAULT, ZANNI, KAMOLA, LUPA, END };
	enum class OBJECTTYPE { DEFAULT, SONORA, INTERACTION, SPAWNOR, DESTRUCTION, NONRIGID, TRIGGERBOX, NONSONORA, NONSONORA_FLOOR, END };

	enum class ACTORDIR { U, RU, R, RD, D, LD, L, LU, END };
	enum class WEAPONTYPE { ANIM, NONANIM, END};
	enum class CHARACTER_TRANSITIONTYPE { IDLE, RUN, ATTACK, QTE, END }; // Character 전환시
	
	enum class ATTACKRANGE : unsigned int 
	{ RANGE_CLOSE = 0, RANGE_MID, RANGE_FAR, END } ;

	enum class UI_CHARACTERTYPE : unsigned int {
		ROVER = 0,
		AUGUSTA,
		GALBRENA,
		END
	};

	enum class UI_AUGUSTA_STATE : unsigned int {
		DEFAULT = 0,				// 기본?
		LB_STRONG_READY = 1,		// 강공 실행 가능
		LB_SWORD_READY = 2,         // SWORD LB 아이콘 준비됨.(SpAttack 상태? => 나궁썼어)
		T_INTERACTION_READY = 3,	// T 실행 가능.  => 활공, 기타등등
		T_INTERACTION_FAILED = 4,	// T 실행 불가.  => 활공, 기타 등등
		E_GRIFFON_READY = 5,		// 그리폰 E 실행 가능 => GRIFFON_E_READY = 
		E_RISE_READY = 6,			// Rise E 실행 가능.
		E_DEFAULT_READY = 7,		// 기본 E 실행 가능.
		Q_ECHO_READY = 8,			// Echo 실행 가능.
		Q_ECHO_FAILED = 9,	    // Echo 실행 불가.
		R_ULTI_READY = 10,			// LB_RESONANCE기본 R 실행 가능
		R_SWORD_READY = 11,			// SWORD R 아이콘 출력 가능.
		R_SWORD_ULTI_READY = 12,	// SWORD R 궁극기 아이콘 출력 가능.
		END
	};
	

	enum class UI_AUGUSTA_CONDITION : unsigned int {
		LB_SP_ATTACK = 1 << 0, // Special Attack (궁 쓸수 있는 상태)	// 강화 궁 진입한 상태 (LB도 생김)
		LB_RESONANCE = 1 << 1, // 강공									// -
		E_GRIFFON = 1 << 2, // 그리폰.									// 그리폰 사용 가능 상태 (3 0)
		E_RISE = 1 << 3, // 그리폰 Rise									// 그 다음 (옆에)
		R_SP_ATTACK = 1 << 4, // 최종 궁 이전 상태 사용 가능.			// -
		R_SP_ATTACKOMNI = 1 << 5, // Special Attack 최종 궁 사용 가능	// -
		END
	};


	enum class UI_ROVER_STATE : unsigned int {
		DEFAULT = 0,
		E_BURST_READY = 1,
		E_DEFAULT_READY = 2,
		R_READY = 3,
		END
	};

	// UI에서 사용하기 위해서 State Machine에서 전달.
	enum class UI_ROVER_CONDITION : unsigned int {
		BURST_ACTIVE = 1 << 0, // Burst ACTIVE
		END
	};

	enum class UI_GABRENA_STATE : unsigned int {
		ROVER_READY = 1,
		END
	};

	enum class UI_GALBRENA_CONDITION : unsigned int {
		BURST = 0,

		END
	};

	enum class KEYINPUT : unsigned int {
		NONE = 1 << 0, 
		W = 1 << 1, 
		S = 1 << 2,
		A = 1 << 3,
		D = 1 << 4, 
		LB = 1 << 5, 
		RB = 1 << 6, 
		WB = 1 << 7, 
		Q = 1 << 8, 
		E = 1 << 9, 
		R = 1 << 10, 
		T = 1 << 11,
		LSHIFT = 1 << 12,
		SPACE = 1 << 13,
		D1 = 1 << 14,
		D2 = 1 << 15,
		D3 = 1 << 16,
		D4 = 1 << 17,
		D5 = 1 << 18,
		D6 = 1 << 19
	};

	typedef enum class ESkillType : _uint
	{
		NONE = 0,	    // (비용 없음)
		RESONANCE,		// 공명 게이지
		AUGUSTA_POINT,	// 특수 원형 게이지
		AUGUSTA_ULTI,	// 기본 궁극기 게이지
		AUGUSTA_SWORD,	// 특수 칼 게이지 (갯수)
		AUGUSTA_UITI_SWORD, // 칼 사용 후 썼을때 공격 게이지?
		SKILL_TYPE_END // CSV 파싱 실패 등을 위한 END
	}SKILL_TYPE;

	typedef enum class ECostType : unsigned int
	{
		NONE = 0, // Stat 아님.
		COST1, // 1 RESONANCE
		COST2, // 2 AUGUSTA POINT
		COST3, // 3 AUGUSTA_SWORD
		COST4, // 4 AUGUSTA_ULTI_SWORD
		COST5, // 5. AUGUTA_ULTI
		STAMINA, // 5
		COST_TYPE_END
	}COST_TYPE;



	enum class SKILL_STATE : unsigned int
	{
		READY = 0,		 // 사용 가능
		COOLING_DOWN,	 // 쿨타임
		NOT_ENOUGH_COST, // 자원 부족
		NOT_EXIST,		 // 정보가 없음.
		END
	};

	// 어떤걸 올려줘야하는가?
	enum class HITTYPE : unsigned int
	{
		NONE = 0, // 없음
		HEAVY,    // 강공
		ULTI,     // 궁극기
		END
	};

	enum class DIRECTION {
		FRONT, BACK, LEFT, RIGHT, UP, DOWN, END
	};

	enum class UI_EVENT_TYPE {
		NONE, CLICK_ENTER, CLICKING, CLICK_EXIT, HOVER_ENTER, HOVERING, HOVER_EXIT, SCROLL, END
	};

	enum class UI_VARIANT_FLAG {			// * UI용 짬통셰이더 플래그 지정용
		UIFLAG_ERROR,				// default. outputs magenta
		UIFLAG_COOLDOWN_CIRCLE,		// 원형 쿨타임 (skill)
		UIFLAG_COOLDOWN_RECT,		// 사각형 쿨타임 (partyframe)
		UIFLAG_PLAYER_HP,
		UIFLAG_PLAYER_TRANSMIT,
		UIFLAG_SIMPLEMASK,
		UIFLAG_ACTIVEFEEDBACK,
		UIFLAG_END
	};

	enum class FONT_FLAG
	{
		FL_NONE				= 0,
		FL_OUTLINE			= 1 << 0,
		FL_GRAD				= 1 << 1,

		FL_FIXED			= 1 << 2,	// world fixed. uses world pos

		FL_END				= 1 << 3
	};

	enum class FONT_DMG_PRESET
	{
		HEAL,
		DARK,
		ELECTRO,
		FUSION,

		END
	};
	
	enum class  TEST_STATE : unsigned int
	{
		NONE				= 0,
		MOVE_FORWARD		= 1 << 0,
		MOVE_BACKWARD		= 1 << 1,
		MOVE_LEFT			= 1 << 2,
		MOVE_RIGHT			= 1 << 3,
		SPLINT				= 1 << 4,
		DODGE				= 1 << 5,
		TURN				= 1 << 6,
		JUMP				= 1 << 8,
		AIR					= 1 << 9,
		GLIDING				= 1 << 10,
		LAND				= 1 << 11,

		STRIKE				= 1 << 12,
		BEHIT				= 1 << 13,
		BLOCK				= 1 << 14,
		PARALYSIS			= 1 << 15,

		ATTACK_1			= 1 << 16,
		ATTACK_2			= 1 << 17,
		ATTACK_3			= 1 << 18,
		ATTACK_4			= 1 << 19,
		ATTACK_5			= 1 << 20,
		ATTACK_6			= 1 << 21,
		ATTACK_7			= 1 << 22,
		ATTACK_8			= 1 << 23,
		ATTACK_9			= 1 << 24,
		ATTACK_10			= 1 << 25,
		ATTACK_11			= 1 << 26,
		SPAWN				= 1 << 27,
		ANIMATION_PLAYING	= 1 << 28,
		DEAD				= 1 << 29
	};

	
	enum class SHADER_ANIMMESH { 
		DEFAULT_NORMAL = 0
		, NORMAL_TEX
		, AUGUSTA
		, SHADOW
		, OUNTLINE
		, END };

}