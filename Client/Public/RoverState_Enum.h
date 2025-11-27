#pragma once

#include "Client_Define.h"
#include "StateCategory_Enum.h"

NS_BEGIN(Client)

#pragma region DEPTH 1
// Rover Ground 
enum class ERoverGroundState : _uint
{
	IDLE = 0,		
	WALK,			
	RUN,
	SPRINT,
	DASH,			
	DODGE,
	ATTACK,			
	SKILL,			
	BURST,			
	SPECIAL,		
	UNIQUE,			
	LAND,		
	QTE,
	GROUND_END
};

// Rover Air 
enum class ERoverAirState : _uint
{
	JUMP = 0,		// 
	FALL,			// 
	AIR_ATTACK,		// 
	HOOK,			// 
	AIR_SKILL,		// 
	FLY,
	AIR_END
};

// Rover Climb 
enum class ERoverClimbState : _uint
{
	CLIMB_IDLE = 0,		
	CLIMB_MOVE,			
	CLIMB_BOOST,		
	CLIMB_EXIT,			
	CLIMB_END
};

// Rover Hit
enum class ERoverHitState : _uint
{
	HIT = 0,
	HIT_END
};

// Galbrena Interaction
enum class ERoverInteractionState : _uint
{
	ROPE = 0, // Rope Action
	INTERACTION_END
};

#pragma endregion

#pragma region DEPTH2

#pragma region GROUND
enum class ERoverIdleType : _uint
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

enum class ERoverRunType : _uint
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

enum class ERoverSprintType : _uint
{
	SPRINT_F = 0,
	STOP_RUN_L,
	STOP_RUN_R,
	STOP_SPRINT_L,
	STOP_SPRINT_R,
	END
};

enum class ERoverLandType : _uint
{
	LAND_LIGHT = 0,     
	LAND_HEAVY,			
	LAND_ROLL,			
	LANDSLIDE_F,		
	LAND_ROLL_ATTACK01_2,
	LANDSLIDE_B,
	LANDSLIDE_SPRINT_LOOP,
	LANDSLIDE_SPRINT_POSE_F,
	LANDSLIDE_SPRINT_START,
	END
};


enum class ERoverDashType : _uint
{
	MOVE_B = 0,
	MOVE_F,
	MOVE_LIMIT_B,
	MOVE_LIMIT_F,
	END


};

enum class ERoverDodgeType : _uint
{
	MOVE_LIMIT_F = 0,
	MOVE_LIMIT_B,
	END
};

enum class ERoverAttackType : _uint
{
	ATTACK01 = 0,
	ATTACK02,
	ATTACK03,
	ATTACK05,
	ATTACK04,
	END
};

enum class ERoverSkillType : _uint
{
	SKILL02 = 0,
	EX_SKILL02,
	END
};

enum class ERoverSpecialType : _uint
{
	EX_ATTACK01 = 0,
	EX_ATTACK02,
	EX_ATTACK03,
	EX_ATTACK04,
	EX_ATTACK05,
	EX_ATTACK05_UP,
	END
};

enum class ERoverBurstType : _uint // 강공.
{
	BURST01 = 0,
	END
};

enum class ERoverUniqueType : _uint // 
{
	SKILL_STRIKE = 0,
	SKILL_RISE,
	END
};

enum class ERoverQTEType : _uint // 
{
	SKILL_QTE = 0,
	END
};

#pragma endregion

#pragma region AIR
enum class ERoverJumpType : _uint
{
	JUMP_LOOP = 0,
	JUMP_RUN_LF,
	JUMP_RUN_RF,
	JUMP_SECOND_B,
	JUMP_SECOND_F,
	JUMP_WALK_LF, 
	JUMP_WALK_RF,
	END
};

enum class ERoverFallType : _uint
{
	FALL_LOOP = 0,
	FALL_LOOP_FAST,
	END

};


enum class ERoverAirAttackType : _uint
{
	AIRATTACK_START = 0,
	AIRATTACK_LOOP,
	AIRATTACK_END,
	END
};


enum class ERoverAirFlyType : _uint
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

enum class ERoverClimbIdleType : _uint
{

	END
};

enum class ERoverClimbMoveType : _uint
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

enum class ERoverClimbBoostType : _uint
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

enum class ERoverClimbExitType : _uint
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
	CLIMB_ONTOP, 
	CLIMB_MOVE,  
	CLIMB_VAULT,
	END
};

#pragma endregion


#pragma region HIT


enum class ERoverHitType : _uint
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


#pragma region INTREACTION
enum class ERoverRopeType : _uint
{
	FIXHOOK_END = 0,
	FIXHOOK_END_FAST,
	FIXHOOK_LOOP_D,
	FIXHOOK_LOOP_F,
	FIXHOOK_LOOP_L,
	FIXHOOK_LOOP_R,
	FIXHOOK_LOOP_U,
	FIXHOOK_START01_D,
	FIXHOOK_START01_F,
	FIXHOOK_START01_U,
	FIXHOOK_START02_D,
	FIXHOOK_START02_F,
	FIXHOOK_START02_U,
	HOOK_UP,
	DRAG_END,
	DRAG_LOOP_D,
	DRAG_LOOP_F,
	DRAG_LOOP_U,
	DRAG_START_D,
	DRAG_START_F,
	DRAG_START_U,
	END
};
#pragma endregion

#pragma endregion

NS_END