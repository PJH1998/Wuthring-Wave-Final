#pragma once

namespace Client
{
	enum class LEVEL { STATIC, LOGO, GAMEPLAY, LOADING, TEST, END };
	enum class CHANNEL { BGM, PLAYER_ACTION, PLAYER_VOICE, ENEMY_ACTION, ENEMY_VOICE, EFFECT, END };
	enum class COLLISIONLAYER { MAP, PLAYER, CHARACTER, CAMERA, ATTACK, SKILL, ENEMY, ENEMY_ATTACK, ENEMY_SKILL, INTERACTION, DETECT, PARRY, GRAB, END };

	enum class SKILLBTN { LBTN, T, E, R, END };
	enum class SKILLICONID { DEFAULT, ZANNI, KAMOLA, LUPA, END };

	enum class KEYINPUT {
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
		SPACE = 1 << 13
	};

	enum class DIRECTION {
		FRONT, BACK, LEFT, RIGHT, UP, DOWN, END
	};

	enum class TEST_STATE{
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

		ATTACK_1			= 1 << 16,
		ATTACK_2			= 1 << 17,
		ATTACK_3			= 1 << 18,
		ATTACK_4			= 1 << 19,
		ATTACK_5			= 1 << 20,
		ATTACK_6			= 1 << 21,
		ATTACK_7			= 1 << 22,
		ATTACK_8			= 1 << 23,

		BLOCK				= 1 << 24,
		PARALYISIS			= 1 << 25,

		SPAWN = 1 << 29,
		ANIMATION_PLAYING = 1 << 30,
		DEAD = 1 << 31
	};

	enum class SHADER_ANIMMESH { DEFAULT_NORMAL, NORMAL_TEX, SHADOW, END };

}