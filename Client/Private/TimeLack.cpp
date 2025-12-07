#include "ClientPch.h"
#include "TimeLack.h"

CTimeLack::CTimeLack()
{
	for (_uint i = 0; i < ENUM_CLASS(COLLISIONLAYER::END); ++i)
	{
		m_fTimeRates[i] = 1.f;
		m_fDurations[i] = 0.f;
	}
}

void CTimeLack::Change_TimeRate(COLLISIONLAYER eLayer, _float fRate)
{
	if (ENUM_CLASS(eLayer) >= ENUM_CLASS(COLLISIONLAYER::END))
		return;

	m_fTimeRates[ENUM_CLASS(eLayer)] = fRate;
}

void CTimeLack::Change_TimeRate(COLLISIONLAYER eLayer, _float fRate, _float fDuration)
{
	if (ENUM_CLASS(eLayer) >= ENUM_CLASS(COLLISIONLAYER::END))
		return;

	m_fTimeRates[ENUM_CLASS(eLayer)] = fRate;
	m_fDurations[ENUM_CLASS(eLayer)] = fDuration;
}

_float CTimeLack::TimeLack(COLLISIONLAYER eLayer)
{
	if (ENUM_CLASS(eLayer) >= ENUM_CLASS(COLLISIONLAYER::END))
		return 1.f;

	return m_fTimeRates[ENUM_CLASS(eLayer)];
}

void CTimeLack::Update(_float fTimeDelta)
{
	for (_uint i = 0; i < ENUM_CLASS(COLLISIONLAYER::END); ++i)
	{
		if (m_fDurations[i] == 0.f)
			continue;

		if (m_fDurations[i] > 0.f)
			m_fDurations[i] -= fTimeDelta;

		if (m_fDurations[i] <= 0.f)
		{
			m_fDurations[i] = 0.f;
			m_fTimeRates[i] = 1.f;
		}
	}
}

CTimeLack* CTimeLack::Create()
{
	return new CTimeLack();
}

void CTimeLack::Free()
{
	__super::Free();
}
