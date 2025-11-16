#include"EnginePch.h"
#include "FreeList.h"

CFreeList::CFreeList()
{

}

HRESULT CFreeList::Initialize(_uint iMemorySize)
{
	m_FreeBlocks.clear();
	m_FreeBlocks.emplace(0, iMemorySize * 1024 * 1024);
	return S_OK;
}

_uint CFreeList::Allocate(_uint iMemorySize)
{
	//Best-fit 알고리즘
	_uint BestFitOffset = -1;
	_uint BestFitSize = (_uint)-1;
	auto BestFitIter = m_FreeBlocks.end();

	for (auto iter = m_FreeBlocks.begin(); iter != m_FreeBlocks.end(); ++iter)
	{
		_uint CurrentBlockSize = iter->second;

		if (CurrentBlockSize >= iMemorySize)
		{
			if (CurrentBlockSize < BestFitSize)
			{
				BestFitOffset = iter->first;
				BestFitSize = CurrentBlockSize;
				BestFitIter = iter;
			}
		}
	}
	if (BestFitIter != m_FreeBlocks.end())
	{
		m_FreeBlocks.erase(BestFitIter);

		_uint RemainderSize = BestFitSize - iMemorySize;
		if (RemainderSize > 0)
		{
			_uint RemainOffset = BestFitOffset + iMemorySize;
			m_FreeBlocks.emplace(RemainOffset, RemainderSize);
		}
	}

    return BestFitOffset;
}

void CFreeList::Free(_uint iMemoryOffset, _uint iMemorySize)
{
	//내 메모리 바로 뒤에 블럭이 있나 확인
	auto nextBlockIter = m_FreeBlocks.find(iMemoryOffset + iMemorySize);
	if (nextBlockIter != m_FreeBlocks.end())
	{
		//있음. 병합
		iMemorySize += nextBlockIter->second;
		m_FreeBlocks.erase(nextBlockIter);
	}

	//내 메모리 바로 앞에 블럭이 있나 확인. lower_bound는 해당 이터레이터와 가장 가까운 뒷 블록 반환.
	auto it = m_FreeBlocks.lower_bound(iMemoryOffset);
	if (it != m_FreeBlocks.end())
	{
		auto PrevBlockIter = prev(it);

		//사이즈 비교
		if (PrevBlockIter->first + PrevBlockIter->second == iMemoryOffset)
		{

			iMemorySize += PrevBlockIter->second;
			//오프셋 앞으로 갱신.
			iMemoryOffset = PrevBlockIter->first;
			m_FreeBlocks.erase(PrevBlockIter);
		}
	}

	m_FreeBlocks.emplace(iMemoryOffset, iMemorySize);
}

CFreeList* CFreeList::Create(_uint iMemorySize)
{
	CFreeList* pInstance = new CFreeList;

	if (FAILED(pInstance->Initialize(iMemorySize)))
	{
		MSG_BOX("Failed to Create : Model_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFreeList::Free()
{
	__super::Free();

}
