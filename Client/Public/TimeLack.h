#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CTimeLack final : public CBase
{
private:
	explicit CTimeLack();
	virtual ~CTimeLack() = default;

public:
	// TimeRate 변경
	void		Change_TimeRate(COLLISIONLAYER eLayer, _float fRate);
	// TimeRate 변경 (지속시간 설정 가능 => Duration)
	void		Change_TimeRate(COLLISIONLAYER eLayer, _float fRate, _float fDuration);
	// Layer에 따른 TimeRate 반환
	_float		TimeLack(COLLISIONLAYER eLayer);

public:
	void		Update(_float fTimeDelta);

private:
	_float		m_fTimeRates[ENUM_CLASS(COLLISIONLAYER::END)];
	_float		m_fDurations[ENUM_CLASS(COLLISIONLAYER::END)];

public:
	static		CTimeLack*	Create();
	virtual		void			Free() override;
};

NS_END