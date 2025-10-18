#pragma once

namespace Client
{
	enum class LEVEL { STATIC, LOGO, GAMEPLAY, LOADING, TEST, END };
	enum class CHANNEL { BGM, PLAYER_ACTION, PLAYER_VOICE, ENEMY_ACTION, ENEMY_VOICE, EFFECT, END };
	enum class COLLISIONLAYER { MAP, PLAYER, CHARACTER, ATTACK, SKILL, ENEMY, ENEMY_ATTACK, ENEMY_SKILL, INTERACTION, DETECT, PARRY, GRAB, END };

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

	enum class UI_EVENT_TYPE {
		CLICK, HOVER, SCROLL, END
	};
}