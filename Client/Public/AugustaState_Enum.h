#pragma once
#include "Client_Define.h"
#include "StateCategory_Enum.h"


NS_BEGIN(Client)

#pragma region DEPTH 1
// Augusta Ground 하위 상태
enum class EAugustaGroundState : _uint
{
	IDLE = 0,		// 대기 (Stand1, Stand2, Stand1_Action01~03, StandChange)
	WALK,			// 걷기 (Walk_F/B/LF/RF/LB/RB, Stop_Walk_L/R)
	RUN,			// 달리기 (Run_F/B/LF/RF/LB/RB, Stop_Run_L/R, Run_Turnback)
	DASH,			// 전력질주 (Sprint_F, Super_Sprint_*, SpWalk_*, Stop_Sprint_L/R)
	ATTACK,			// 공격 (Attack01~04, Attack_*, SpAttack*)
	SKILL,			// 스킬 (Skill)
	BURST,			// Burst 상태
	SPECIAL,		// Burst Special 상태.
	UNIQUE,			// 캐릭터 고유 상태. (Griffon 등등)
	LAND,			// 착지 (Land)
	QTE,			// 협주 스킬(Augusta의 경우 변주)
	GROUND_END
};

// Augusta Air 하위 상태
enum class EAugustaAirState : _uint
{
	JUMP = 0,		// 점프 (Jump_Loop, Jump_Run_*, Jump_Walk_*, Jump_Second_*)
	FALL,			// 낙하 (Fall_Loop, Fall_Loop_Fast, Fall_LeanPose_*)
	AIR_ATTACK,		// 공중 공격 (AirAttack_Start/Loop/End, AirAttack_HackDown_*)
	HOOK,			// 갈고리 (Hook_Up)
	AIR_SKILL,		// 공중 스킬 (Air
	FLY,			// 날기.
	AIR_END
};

// Augusta Climb 하위 상태
enum class EAugustaClimbState : _uint
{
	CLIMB_IDLE = 0,		// 등반 대기 (Climb_Stand, Climb_Move)
	CLIMB_MOVE,			// 등반 이동 (Climb_U/D/L/R_1/2, Climb_UL/UR/DL/DR_1/2, Climb_*_Stop)
	CLIMB_BOOST,		// 등반 가속 (Climb_Boost_*, Climb_Dash_*)
	CLIMB_EXIT,			// 등반 탈출 (Climb_OnTop, Climb_Vault, Climb_Start_Up/Down)
	CLIMB_END
};

// Augusta Hit 하위 상태
enum class EAugustaHitState : _uint
{
	HIT = 0,
	HIT_END
};

#pragma endregion


#pragma region DEPTH2

#pragma region GROUND
enum class EAugustaIdleType : _uint
{
	STAND1_ACTION01 = 0,
	STAND1_ACTION02,
	STAND1_ACTION03,
	STAND1_TURN_L90D,
	STAND1_TURN_R90D,
	STAND1,
	STAND2,
	STAND_CONTROL,
	STANDCHANGE,
	STANDUP,
	END
};

enum class EAugustaRunType : _uint
{
	RUN_B = 0,
	RUN_F,
	RUN_LB,
	RUN_LF,
	RUN_RB,
	RUN_RF,
	RUN_BASEPOSE,
	RUN_POSE_F,
	RUN_POSE_L,
	RUN_POSE_R,
	RUN_TURNBACK,
	SPRINT_F,
	STOP_RUN_L,
	STOP_RUN_R,
	STOP_SPRINT_L,
	STOP_SPRINT_R,
	END
};

enum class EAugustaLandType : _uint
{
	LAND_LIGHT = 0,     // 약한 착지
	LAND_HEAVY, // 강한 착지
	LAND_ROLL,      // 착지후 구르기.
	LANDSLIDE_F, // 매달린 애니메이션.
	LAND_ROLL_ATTACK01_2,
	LANDSLIDE_B,
	LANDSLIDE_SPRINT_LOOP,
	LANDSLIDE_SPRINT_POSE_F,
	LANDSLIDE_SPRINT_START,
	END
};


enum class EAugustaDashType : _uint
{
	MOVE_B = 0, 
	MOVE_F,
	MOVE_LIMIT_B,
	MOVE_LIMIT_F,
	END


};

enum class EAugustaAttackType : _uint
{
	ATTACK01 = 0,
	ATTACK02,
	ATTACK03,
	ATTACK04,
	ATTACK_HEAVYHACK,
	ATTACK_PENDING, // NormalAttack과 HeavyAttack 구별용도.
	ATTACK_PULL,
	ATTACK_SPEEDDRIVE,
	ATTACK_SPSKILL,
	SPATTACK01,
	SPATTACK02,
	SPATTACK03,
	SPATTACKOMNI,
	END
};

enum class EAugustaSkillType : _uint
{
	SKILL_HACK = 0,
	SKILL_RISE,
	SKILL_RISE_ZERO,
	SKILL_STRIKE,
	SKILLQTE,
	ATTACK_PULL,
	ATTACK_SPEEDDRIVE,
	ATTACK_SPSKILL,
	END
};

enum class EAugustaSpecialType : _uint
{
	SPATTACK01 = 0,
	SPATTACK02,
	SPATTACK03,
	SPATTACKOMNI,
	SPWALK_DASH,
	SPWALK_DASH_ROOT,
	SPWALK_F,
	SPWALK_STAND,
	SPWALK_STOP_L,
	SPWALK_STOP_R,
	END
};

enum class EAugustaBurstType : _uint
{
	BURST01 = 0,
	BURST_STAND,
	END
};

enum class EAugustaQTEType : _uint
{
	SKILLQTE = 0,
	END
};

enum class EAugustaUniqueType : _uint // 그리폰 등등..
{
	SKILL_STRIKE = 0,
	SKILL_RISE,
	END
};


#pragma endregion

#pragma region AIR
enum class EAugustaJumpType : _uint
{
	JUMP_LOOP = 0,
	JUMP_RUN_LF, // 앞으로
	JUMP_RUN_RF,
	JUMP_SECOND_B, // 더블 점프
	JUMP_SECOND_F,
	JUMP_WALK_LF, // 제자리
	JUMP_WALK_RF,
	END
};

enum class EAugustaFallType : _uint
{
	FALL_LOOP = 0,
	FALL_LOOP_FAST,
	END

};

enum class EAugustaAirAttackType : _uint
{
	AIRATTACK_END = 0,
	AIRATTACK_HACKDOWN_LOOP,
	AIRATTACK_HACKDOWN_SP_END,
	AIRATTACK_HACKDOWN_START,
	AIRATTACK_LOOP,
	AIRATTACK_START,
	END
};

enum class EAugustaAirSkillType : _uint
{

	END
};

// Fly Type
enum class EAugustaAirFlyType : _uint
{
	XA_LOOP_U = 0,
	XA_LOOP_D,
	XA_LOOP_L,
	XA_LOOP_R,
	XA_LOOP_RL_MID,
	XA_LOOP_STAND,
	XA_SHAKE_LOOP,
	XA_START,
	END
};


#pragma endregion

#pragma region CLIMB

enum class EAugustaClimbIdleType : _uint
{
	
	END
};

enum class EAugustaClimbMoveType : _uint
{
	CLIMB_D_1 = 0,
	CLIMB_D_2,
	CLIMB_DL_1,
	CLIMB_DL_2,
	CLIMB_DR_1,
	CLIMB_DR_2,
	CLIMB_L_1,
	CLIMB_L_2,
	CLIMB_R_1,
	CLIMB_R_2,
	CLIMB_U_1,
	CLIMB_U_2,
	CLIMB_UL_1,
	CLIMB_UL_2,
	CLIMB_UR_1,
	CLIMB_UR_2,
	END
};

enum class EAugustaClimbBoostType : _uint
{
	CLIMB_BOOST_L = 0,
	CLIMB_BOOST_L_START,
	CLIMB_BOOST_L_STOP,
	CLIMB_BOOST_ONTOP_LF,
	CLIMB_BOOST_ONTOP_RF,
	CLIMB_BOOST_ONTOPTOSPRINT,
	CLIMB_BOOST_ONTOPTOSPRINT_LF,
	CLIMB_BOOST_ONTOPTOSPRINT_RF,
	CLIMB_BOOST_R,
	CLIMB_BOOST_R_START,
	CLIMB_BOOST_R_STOP,
	CLIMB_BOOST_SHORTVAULT_LF,
	CLIMB_BOOST_SHORTVAULT_RF,
	CLIMB_BOOST_SHORTVAULTTOSPRINT_LF,
	CLIMB_BOOST_SHORTVAULTTOSPRINT_RF,
	CLIMB_BOOST_U,
	CLIMB_BOOST_U_START,
	CLIMB_BOOST_U_STOP,
	CLIMB_BOOST_VAULT,
	CLIMB_BOOST_VAULTTOFALL,
	CLIMB_BOOST_VAULTTOP,
	END
};

enum class EAugustaClimbExitType : _uint
{
	CLIMB_D1_STOP = 0,
	CLIMB_D2_STOP,
	CLIMB_DL1_STOP,
	CLIMB_DL2_STOP,
	CLIMB_DR1_STOP,
	CLIMB_DR2_STOP,
	CLIMB_L1_STOP,
	CLIMB_L2_STOP,
	CLIMB_R1_STOP,
	CLIMB_R2_STOP,
	CLIMB_U1_STOP,
	CLIMB_U2_STOP,
	CLIMB_UL1_STOP,
	CLIMB_UL2_STOP,
	CLIMB_UR1_STOP,
	CLIMB_UR2_STOP,
	CLIMB_ONTOP, // 벽 넘는 탈출.(위에서)
	CLIMB_MOVE, // 뒷 점프 탈출
	CLIMB_VAULT,
	END
};

#pragma endregion



#pragma region HIT

/*
*   HIT_SMALL = 0,		// 약한 피격 (Behit_S_L/R, Behit_B_L/R)
	HIT_FLY,			// 날아가는 피격 (Behit_Fly_Start/Loop/Fall)
	HIT_PUSH,			// 밀리는 피격 (Behit_Push_Start/Loop/Fall)
	HIT_HOVER,			// 공중 피격 (Behit_Hover, Behit_Press)
	HIT_CAPTURED,		// 포획 (Captured)
	HIT_DEATH,			// 사망 (Death, StandUp)
*/
enum class EAugustaHitType : _uint
{
	BEHIT_B_L = 0,
	BEHIT_B_R,
	BEHIT_FLY_FALL,
	BEHIT_FLY_LOOP,
	BEHIT_FLY_START,
	BEHIT_HOVER,
	BEHIT_PRESS,
	BEHIT_PUSH_FALL,
	BEHIT_PUSH_LOOP,
	BEHIT_PUSH_START,
	BEHIT_S_L,
	BEHIT_S_R,
	END
};

#pragma endregion


#pragma endregion




NS_END
