#pragma once

#include "EnginePch.h"

NS_BEGIN(Engine)

class ENGINE_DLL CBase
{
protected:
	explicit CBase();
	virtual ~CBase() = default;

public:
	_uint		AddRef(); // 참조 시 레퍼런스 카운트 증가
	_uint		Release(); // 참조를 끝낼 시 레퍼런스 카운트 감소 및 0일 때 객체 삭제

private:
	_uint		m_iRefCnt = {}; 	// 레퍼런스 카운트

public:
	virtual void Free();
};

NS_END