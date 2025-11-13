#pragma once
#include "Base.h"
NS_BEGIN(Engine)
class CFreeList final : public CBase
{
private:
	CFreeList();
	virtual ~CFreeList() = default;

public:

	void Initialize(_uint iMemorySize);
	_uint Allocate(_uint iMemorySize);
	void Free(_uint iMemoryOffset, _uint iMemorySize);

private:
	map<_uint, _uint> m_FreeBlocks;

public:

	static CFreeList* Create();
	virtual void Free()override;
};

NS_END