#include "EnginePch.h"
#include "OctoTree.h"

#include "CubeCell.h"

COctoTree::COctoTree()
{
}

void COctoTree::SetUp_OctoTree(_float3 vCenter, _float3 vExtent)
{
	Safe_Release(m_pRootCell);
	m_pRootCell = CCubeCell::Create(vCenter, vExtent, 0);
	ASSERT_CRASH(m_pRootCell);
}

void COctoTree::Add_To_OctoTree(CStaticObject* pObject, const BoundingBox* pBox)
{
	
}

COctoTree* COctoTree::Create()
{
    return new COctoTree();
}

void COctoTree::Free()
{
	__super::Free();

	Safe_Release(m_pRootCell);
}
