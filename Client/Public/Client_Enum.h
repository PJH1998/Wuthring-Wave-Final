#pragma once
#include "Client_CharacterEnum.h"

namespace Client
{
	enum class LEVEL { STATIC, LOGO, GAMEPLAY, HEAVEN, LOADING, TEST, TEST_UI, END };
	enum class CHANNEL { BGM, PLAYER_ACTION, PLAYER_VOICE, ENEMY_ACTION, ENEMY_VOICE, EFFECT, END };
	enum class COLLISIONLAYER { NONE, MAP, QTE, PLAYER, ATTACK, SKILL, KNOCKBACK, ENEMY, ENEMY_ATTACK, ENEMY_HARDATTACK, ENEMY_SKILL, INTERACTION, DETECT, PARRY, GRAB, END };

	enum class SKILLBTN { LBTN, T, E, R, END };
	enum class SKILLICONID { DEFAULT, ZANNI, KAMOLA, LUPA, END };
	enum class OBJECTTYPE { DEFAULT, SONORA, INTERACTION, SPAWNOR, DESTRUCTION, NONRIGID, TRIGGERBOX, NONSONORA, NONSONORA_FLOOR, END };

	enum class ACTORDIR { U, RU, R, RD, D, LD, L, LU, END };
	enum class WEAPONTYPE { ANIM, NONANIM, END};
	enum class ATTACKRANGE : unsigned int 
	{ RANGE_CLOSE = 0, RANGE_MID, RANGE_FAR, END } ;

	enum class ATTACKVOULME_DIR : unsigned int
	{
		DEFAULT = 0, // 기본 값.
		UPPER,
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
		FL_ALPHA_EDITABLE	= 1 << 2,	// matExtra 11.


		FL_END				= 1 << 3
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
		, NORMAL_TEX // 1
		, AUGUSTA // 2
		, SHADOW // 3
		, OUNTLINE // 4
		, ROVER // 5
		, GALBRENA // 6
		, NORMAL_YELLOW // 7
		, LOGOROVER // 8
		, DISSOLVE_NORMAL // 9
		, END };

	enum class SHADER_PROPANIMMESH {
		DEFAULT_NORMAL = 0
		, NORMAL_TEX // 1
		, SHADOW // 2
		, OUNTLINE // 3
		, DEFAULT_WEAPON // 4
		, DISSOLVE_WEAPON // 5
		, END
	};

	enum class SHADER_SFX_BURST { AUGUSTA_SLASH, RADIAL_BLUR, GALBRENA_CIRCLE, GALBRENA_BLUR};
	enum class SHADER_SFX_BURST_INSTANCE { GALBRENA_SLASH, GALBRENA_STAR };

	enum class TEXT_COLOR_TYPE {
		NONE,	// 기본값. 지정 안했다고 가정, 마젠타 출력

		// 데미지용 색상
		HEAL,	// 회복
		DARK,	// 인멸
		ELEC,	// 전도
		FUSI,	// 용융

		// 텍스트용 색상
		TT_TITLE,
		TT_NORMAL,
		TT_PROGRESS,

		TT_BOSSNAME,
		TT_PLAYERHP,

		END
	};

	enum class TEXT_ALIGN_TYPE {
		LEFT, 
		CENTER, 
		RIGHT, 

		END 
	};
}