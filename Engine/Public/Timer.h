#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CTimer final : public CBase
{
private:
	explicit CTimer();
	virtual ~CTimer() = default;

public:
	_float	Get_TimeDelta() const { return m_fTimeDelta * m_fTimeRate; }
	void	Change_TimeRate(_float fTimeRate) { m_fTimeRate = fTimeRate; }

public:
	HRESULT		Initialize();
	void			Update_Timer();

private:
	LARGE_INTEGER		m_FrameTime = {};
	LARGE_INTEGER		m_FixTime = {};
	LARGE_INTEGER		m_LastTime = {};
	LARGE_INTEGER		m_CpuTick = {};

	_float						m_fTimeDelta = {};
	_float						m_fTimeRate = { 1.f };

public:
	static CTimer* Create();
	virtual void Free() override;
};

NS_END

