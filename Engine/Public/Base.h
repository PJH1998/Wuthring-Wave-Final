#pragma once

#include "EnginePch.h"

NS_BEGIN(Engine)

class ENGINE_DLL CBase
{
protected:
	explicit CBase();
	virtual ~CBase() = default;

public:
	_uint		AddRef(); // 李몄“ ???덊띁?곗뒪 移댁슫??利앷?
	_uint		Release(); // 李몄“瑜??앸궪 ???덊띁?곗뒪 移댁슫??媛먯냼 諛?0????媛앹껜 ??젣

private:
	_uint		m_iRefCnt = {}; 	// ?덊띁?곗뒪 移댁슫??

public:
	virtual void Free();
};

NS_END