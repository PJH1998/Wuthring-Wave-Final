#pragma once


namespace AbilityConst
{
	namespace Config
	{
		inline constexpr const _char* SkillFileName = "Skill.csv";
		inline constexpr const _char* StatFileName = "Stat.csv";
	}

	namespace Value
	{
		inline constexpr _float fRecoveryStemina = 10.f;
		inline constexpr _float fDecreaseCost = 8.f;
	}

	namespace SkillNames
	{
		inline constexpr const _char* RoverBurstE = "Ex_Skill02";
		inline constexpr const _char* RoverDefaultE = "Skill02";
		inline constexpr const _char* RoverBurstR = "Burst01_Ulti";

		inline constexpr const _char* AugustaDefaultE = "Skill_Hack";
		inline constexpr const _char* AugustaChargeE = "Attack_HeavyHack";
		inline constexpr const _char* AugustaGriffonE = "Skill_Strike";
		inline constexpr const _char* AugustaESkillRiseZero = "Skill_Rise_Zero";
		inline constexpr const _char* AugustaESkillRise = "Skill_Rise";
		inline constexpr const _char* AugustaSkill_AirAttackEnd = "AirAttack_HackDown_Sp_End";

		inline constexpr const _char* AugustaAirAttackStart = "AirAttack_HackDown_Start";
		inline constexpr const _char* AugustaAirAttackEnd = "AirAttack_HackDown_End";

		
		
		inline constexpr const _char* AugustaDefaultR = "Attack_SpeedDrive";
		inline constexpr const _char* AugustaBurstR = "Burst01";
		inline constexpr const _char* AugustaSpecialAttackFirst = "SpAttack01";
		inline constexpr const _char* AugustaSpecialAttackSecond = "SpAttack02";
		inline constexpr const _char* AugustaSpecialAttackThird = "SpAttack03";
		inline constexpr const _char* AugustaSpecialAttackEnd = "SpAttackOmni";

		inline constexpr const _char* GalbrenaBurstE = "Skill01";
		inline constexpr const _char* GalbrenaDefaultE = "Attack_Jump_Start";
		inline constexpr const _char* GalbrenaDefaultR = "Burst01";
	}

	namespace SkillTypes
	{
		inline constexpr const _char* None = "NONE";
		inline constexpr const _char* Resonanace = "RESONANCE";
		inline constexpr const _char* AugustaPoint = "AUGUSTA_POINT";
		inline constexpr const _char* AugustaUlti = "AUGUSTA_ULTI";
		inline constexpr const _char* AugustaSword = "AUGUSTA_SWORD";
	}

	namespace KeyType
	{
		inline constexpr const _char* E = "E";
		inline constexpr const _char* R = "R";
		inline constexpr const _char* LB = "LB";
		inline constexpr const _char* T = "T";
		inline constexpr const _char* Q = "Q";
	}
}
