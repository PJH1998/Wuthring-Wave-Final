#pragma once
#include "Engine_Define.h"

namespace Client
{
	typedef struct tagCharacterStat
	{
#pragma region old

		_float fHp;
		_float fAttack;
		_float fSwitchGauge; // ���� ������
		_float fMaxSwitchGauge;
		
		_float fBurstGauge; // ����� Burst Gauge
		_float fMaxBurstGauge;
		
		_float fUniqueGauge; // ĳ���� Ư�� Gauge
		_float fMaxUniqueGauge;

#pragma endregion
		//// Player Index (Selected)
		//_uint	iSelectedCharIndex = 0;		// 현재 선택한 캐릭터의 인덱스.		일단은 0 방랑자 / 1 아우구스타 / 2 갈브레나를 상정하였음
		//_uint	iPlayerMode = {};			// 플레이어의 변신 모드.	기본 0 / 방랑자는 다크서지, 아우구스타는 궁극기 사용, 갈브레나는 악마의자격 진입 상태를 1로 상정하였음
		//
		//// Player Resources
		//_float	fPlayerMaxHP[3] = {};		// 최대 체력
		//_float	fPlayerCurHP[3] = {};		// 현재 체력
		//
		//_float	fPlayerMaxEnergy[3] = {};	// 최대 에너지(공명 회로 게이지)
		//_float	fPlayerCurEnergy[3] = {};	// 현재 에너지
		//
		//_float	fPlayerMaxElem[3] = {};		// 협주 에너지 최대 수치. (왼쪽 원소모양 둥글게 돌아가는 그것)
		//_float	fPlayerCurElem[3] = {};		// 협주 에너지 현재 수치.
		//
		//_float	fPlayerMaxSkillCD[3][2] = {};	// 플레이어의 스킬 최대 쿨타임 수치. 인덱스는 [캐릭터 인덱스][스킬 E, R] 을 의미
		//_float	fPlayerCurSkillCD[3][2] = {};	// 플레이어의 스킬 현재 쿨타임 수치. 인덱스는 [캐릭터 인덱스][스킬 E, R] 을 의미
		//
		//_float	fPlayerMaxChangeCD[3][2] = {};	// 플레이어의 전환 최대 쿨타임 수치.
		//_float	fPlayerCurChangeCD[3][2] = {};	// 플레이어의 전환 현재 쿨타임 수치.
		//
		//_float	fPlayerMaxUlt[3] = {};			// [아직 구현 X] 플레이어의 궁극기(공명해방) 자원 수치.
		//_float	fPlayerCurUlt[3] = {};			// [아직 구현 X] 플레이어의 궁극기(공명해방) 자원 수치.
		//
		//// Player Extra Resources
		//_float	fExtraMaxEnergy_Galbrena = {};		// 갈브레나용 특수(에코) 게이지 최대 수치. 50으로 상정하였음
		//_float	fExtraMinEnergy_Galbrena = {};		// 갈브레나용 특수(에코) 게이지 현재 수치. 10 단위로 오름을 상정하였음.
		//_float	fExtraMaxEnergy_GalbrenaDash = {};	// [아직 구현 X] 갈브레나용 특수(악마의자격 중 하단 대쉬 유지 게이지) 최대 수치. 
		//_float	fExtraMinEnergy_GalbrenaDash = {};	// [아직 구현 X] 갈브레나용 특수(악마의자격 중 하단 대쉬 유지 게이지) 현재 수치. 
		//
		//_float	fExtraMaxEnergy_AugustaUlt = {};	// 아우구스타용 특수(궁극기) 게이지 최대 수치. 100으로 상정하였음. 
		//_float	fExtraMinEnergy_AugustaUlt = {};	// 아우구스타용 특수(궁극기) 게이지 현재 수치. 100/7 단위로 오름을 상정하였음.
		//_float	fExtraMaxEnergy_AugustaPoint = {};	// 아우구스타용 특수(원형) 게이지 최대 수치. 100으로 상정하였음. 
		//_float	fExtraMinEnergy_AugustaPoint = {};	// 아우구스타용 특수(원형) 게이지 현재 수치.
		//_uint	iExtraMaxEnergy_AugustaSword = {};	// 아우구스타용 특수(칼) 게이지 현재 갯수. 2라고 상정하였음.
		//_uint	iExtraMinEnergy_AugustaSword = {};	// 아우구스타용 특수(칼) 게이지 현재 갯수. 

	}CHARACTER_STAT;

}
