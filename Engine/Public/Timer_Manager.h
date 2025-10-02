#pragma once

#include "Timer.h"

NS_BEGIN(Engine)

class CTimer_Manager final : public CBase
{
private:
	explicit CTimer_Manager();
	virtual ~CTimer_Manager() = default;

public:
	_float			Get_TimeDelta(const _wstring& strTimerTag);
	void			Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate);

public:
	HRESULT		Add_Timer(const _wstring& strTimerTag);

private:		
	map<const _wstring, CTimer*>	m_Timers;

private:
	CTimer* Find_Timer(const _wstring& strTimerTag);

public:
	static		CTimer_Manager* Create();
	virtual		void					Free();
};

NS_END