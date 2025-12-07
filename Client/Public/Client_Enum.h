#pragma once
#include "Client_CharacterEnum.h"

namespace Client
{
	enum class LEVEL { STATIC, LOGO, GAMEPLAY, HEAVEN, LOADING, TEST, TEST_UI, END };
	enum class CHANNEL { BGM, PLAYER_ACTION, PLAYER_VOICE, ENEMY_ACTION, ENEMY_VOICE, EFFECT, END };
	enum class COLLISIONLAYER { NONE, MAP, QTE, PLAYER, ATTACK, SKILL, KNOCKBACK, ENEMY, ENEMY_ATTACK, ENEMY_HARDATTACK, ENEMY_SKILL, INTERACTION, GRAPPLE, DETECT, PARRY, GRAB, NPC, ALTER, SEQUENCE, SLIDE, END };

	enum class SKILLBTN { LBTN, T, E, R, END };
	enum class SKILLICONID { DEFAULT, ZANNI, KAMOLA, LUPA, END };
	enum class OBJECTTYPE { DEFAULT, SONORA, INTERACTION, SPAWNOR, DESTRUCTION, NONRIGID, TRIGGERBOX, NONSONORA, NONSONORA_FLOOR, WATER, COLLAPS, ROPE_ANCHOR, ROPE_PULL, END };
	enum class INSTANCETYPE { DEFAULT, SONORO, NONSONORO, END };

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
		NONE, CLICK_ENTER, CLICKING, CLICK_EXIT, HOVER_ENTER, HOVERING, HOVER_EXIT, END
	};

	enum class UI_VARIANT_FLAG {			// * UI용 짬통셰이더 플래그 지정용. 필요할때마다 만들고 여기에 추가한다.
		UIFLAG_ERROR,				// default. outputs magenta
		UIFLAG_COOLDOWN_CIRCLE,		// 원형 쿨타임 (skill)
		UIFLAG_COOLDOWN_RECT,		// 사각형 쿨타임 (partyframe)
		UIFLAG_PLAYER_HP,			// 플레이어 체력바용
		UIFLAG_PLAYER_TRANSMIT,		// 공명 회로 일렁임
		UIFLAG_SIMPLEMASK,			// 단순 마스킹용
		UIFLAG_ACTIVEFEEDBACK,		// 조작 피드백 (스케일 커지며 사라지는 것)
		UIFLAG_ENEMY_HP,			// 적 체력바용
		UIFLAG_OVFL_PALETTE,		// 다채화용 각 박스에 사용
		UIFLAG_SIMPLE_COLORIZE,		// 단순 이미지 색상 평균값을 통한 색상화
		UIFLAG_WAVECIRCLE,			// 원의 중점으로부터 바깥쪽 방향으로의 파동 효과.

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

	enum class SHADER_MESH { DEFAULTPASS = 0, ALPHANOTDISCARD = 1, ALPHANOTDISCARDNONCULL = 2, END };
	
	enum class SHADER_ANIMMESH { DEFAULT_NORMAL = 0, NORMAL_TEX, AUGUSTA, SHADOW, OUNTLINE, ROVER, GALBRENA, NORMAL_YELLOW, LOGOROVER, DISSOLVE_NORMAL, END };

	enum class SHADER_ANIMMESH_CHARACTER { 
		DEFAULT_NORMAL = 0, NORMAL_TEX, AUGUSTA, SHADOW, OUNTLINE, ROVER, GALBRENA, NORMAL_YELLOW, LOGOROVER
		, GALBRENABACK, DISSOLVE_CHARACTER, GALBRENAEYE, ROVERMASK, NONMORPH_CHARACTER, END };

	enum class SHADER_PROPANIMMESH {
		DEFAULT_NORMAL = 0, NORMAL_TEX, SHADOW, OUNTLINE, DEFAULT_WEAPON, DISSOLVE_GALBRENAWEAPON, ENERGY_BLADE, AUGUSTA_HEADPROP
		, DISSOLVE_AUGUSTAWEAPON, DISSOLVE_ROVERWEAPON, AUGUSTA_BURSTWEAPON_EFFECT, AUGUSTA_BURSTWEAPON, YUNO_WEAPON, END};

	enum class SHADER_ANIMINST { DEFAULT_NORMAL = 0, NORMAL_TEX, AUGUSTA, SHADOW, OUNTLINE, FACE, END };

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
		TT_TABUTIL,
		TT_SKILLCD,

		END
	};

	enum class TEXT_ALIGN_TYPE {
		LEFT, 
		CENTER, 
		RIGHT, 

		END 
	};

	enum class UI_TAB_UTILITY : unsigned int {
		GRAPPLE = 0,	// 그래플
		SENSOR,			// 스캔
		FLIGHT,			// 활공
		LEVITATOR,		// 컨트롤

		NOTHING			// UI OFF 시 아무것도 선택되지 않음
	};

	enum class UI_QTE_BTN {
		F, E, Q, R, T, END
	};

	enum class UI_QTE_TYPE {
		FILLGUAGE, TRIGGER_ROPE, TRIGGER_EXECUTE, END
	};

	enum class UI_MINIMAP_OBJTYPE {
		MONSTER, END	// 나중에 더 필요한 것 있으면 추가? 상호작용 요소..
	};

	enum class UI_GRAPPLE_TYPE {	// 다른점? 색깔..
		MOVEABLE,		// 이동용
		PULLABLE,		// 벽 부수는 등의 상호작용 용도
		END 
	};

}