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
	SPRINT,			// 전력질주 (Sprint_F, Super_Sprint_*, SpWalk_*, Stop_Sprint_L/R)
	ATTACK,			// 공격 (Attack01~04, Attack_*, SpAttack*)
	SKILL,			// 스킬 (Skill_Hack, Skill_Rise, SkillQTE, Burst*)
	GROUND_END
};

// Augusta Air 하위 상태
enum class EAugustaAirState : _uint
{
	JUMP = 0,		// 점프 (Jump_Loop, Jump_Run_*, Jump_Walk_*, Jump_Second_*)
	FALL,			// 낙하 (Fall_Loop, Fall_Loop_Fast, Fall_LeanPose_*)
	AIR_ATTACK,		// 공중 공격 (AirAttack_Start/Loop/End, AirAttack_HackDown_*)
	HOOK,			// 갈고리 (Hook_Up)
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
	HIT_SMALL = 0,		// 약한 피격 (Behit_S_L/R, Behit_B_L/R)
	HIT_FLY,			// 날아가는 피격 (Behit_Fly_Start/Loop/Fall)
	HIT_PUSH,			// 밀리는 피격 (Behit_Push_Start/Loop/Fall)
	HIT_HOVER,			// 공중 피격 (Behit_Hover, Behit_Press)
	HIT_CAPTURED,		// 포획 (Captured)
	HIT_DEATH,			// 사망 (Death, StandUp)
	HIT_END
};

#pragma endregion


#pragma region DEPTH2
// Animation 변경용 변수
enum class EIdleType : _uint
{
	STAND1_ACTION01 = 0,
	STAND1_ACTION02,
	STAND1_ACTION03,
	STAND1_TURN_L90D,
	STAND1_TURN_R90D,
	STAND2,
	STAND_CONTROL,
	STANDCHANGE,
	STANDUP,
	END
};

enum class ERunType : _uint
{
	RUN_B = 0, 
	RUN_BASEPOSE,
	RUN_F,
	RUN_LB,
	RUN_LF,
	RUN_POSE_F,
	RUN_POSE_L,
	RUN_POSE_R,
	RUN_RB,
	RUN_RF,
	RUN_TURNBACK,
	STOP_RUN_L,
	STOP_RUN_R,
	END
};

enum class ESkillType : _uint
{
	HACK = 0,
	RISE,
	STRIKE,
	QTE,
	BURST,
	END
};
#pragma endregion




NS_END
