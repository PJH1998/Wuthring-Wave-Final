#pragma once
#include "Engine_Define.h"

namespace Client
{
	typedef struct tagCharacterStat
	{
		_float fHp;
		_float fAttack;
		_float fSwitchGauge; // ���� ������
		_float fMaxSwitchGauge;

		_float fBurstGauge; // ����� Burst Gauge
		_float fMaxBurstGauge;

		_float fUniqueGauge; // ĳ���� Ư�� Gauge
		_float fMaxUniqueGauge; 
	}CHARACTER_STAT;

	

}
