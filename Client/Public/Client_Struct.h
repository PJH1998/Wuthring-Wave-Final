#pragma once
#include "Engine_Define.h"

namespace Client
{
	typedef struct tagCharacterStat
	{
		_float fHp;
		_float fAttack;
		_float fSwitchGauge; // 협주 게이지
		_float fMaxSwitchGauge;

		_float fBurstGauge; // 노란색 Burst Gauge
		_float fMaxBurstGauge;

		_float fUniqueGauge; // 캐릭터 특수 Gauge
		_float fMaxUniqueGauge; 
	}CHARACTER_STAT;

	

}
