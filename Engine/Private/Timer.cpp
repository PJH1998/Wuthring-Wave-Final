#include "EnginePch.h"
#include "Timer.h"

CTimer::CTimer() 
{
	ZeroMemory(&m_FixTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_LastTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_FrameTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_CpuTick, sizeof(LARGE_INTEGER));
}


HRESULT CTimer::Initialize()
{
	// 현재 CPU 카운팅 숫자
	QueryPerformanceCounter(&m_FrameTime);			// 1077
	QueryPerformanceCounter(&m_LastTime);			// 1085
	QueryPerformanceCounter(&m_FixTime);			// 1090

	// 초당 CPU가 카운팅 할 수 있는 최대 숫자
	QueryPerformanceFrequency(&m_CpuTick);

	return S_OK;
}

void CTimer::Update_Timer()
{
	QueryPerformanceCounter(&m_FrameTime);			// 1500

	if (m_FrameTime.QuadPart - m_FixTime.QuadPart >= m_CpuTick.QuadPart)
	{
		QueryPerformanceFrequency(&m_CpuTick);
		m_FixTime = m_FrameTime;
	}

	m_fTimeDelta = (m_FrameTime.QuadPart - m_LastTime.QuadPart) / static_cast<_float>(m_CpuTick.QuadPart);

	m_LastTime = m_FrameTime;
}

CTimer* CTimer::Create()
{
	CTimer* pInstance = new CTimer();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : Timer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTimer::Free()
{
	__super::Free();
}
