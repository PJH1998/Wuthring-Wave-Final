#pragma once

namespace Client
{
	enum class LEVEL { STATIC, LOGO, GAMEPLAY, LOADING, TEST, TEST_UI, END };
	enum class CHANNEL { BGM, PLAYER_ACTION, PLAYER_VOICE, ENEMY_ACTION, ENEMY_VOICE, EFFECT, END };
	enum class COLLISIONLAYER { NONE, MAP, PLAYER, CHARACTER, CAMERA, ATTACK, SKILL, ENEMY, ENEMY_ATTACK, ENEMY_SKILL, INTERACTION, DETECT, PARRY, GRAB, END };

	enum class SKILLBTN { LBTN, T, E, R, END };
	enum class SKILLICONID { DEFAULT, ZANNI, KAMOLA, LUPA, END };

	enum class ACTORDIR { U, RU, R, RD, D, LD, L, LU, END };

	enum class WEAPONTYPE { ANIM, NONANIM, END};

	enum class PLAYER_STATE : unsigned int {
		NONE = 0, IDLE, WALK, RUN, ATTACK,
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
	
	enum class  TEST_STATE : unsigned int
	{
		NONE				= 0,
		MOVE_FORWARD		= 1 << 0,
		MOVE_BACKWARD		= 1 << 1,
		MOVE_LEFT			= 1 << 2,
		MOVE_RIGHT			= 1 << 3,
		SPLINT				= 1 << 4,
		DODGE				= 1 << 5,

		JUMP				= 1 << 8,
		AIR					= 1 << 9,
		GLIDING				= 1 << 10,
		LAND				= 1 << 11,

		TURN				= 1 << 12,
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

	enum class SHADER_ANIMMESH { DEFAULT_NORMAL, NORMAL_TEX, AUGUSTA, SHADOW, END };

}