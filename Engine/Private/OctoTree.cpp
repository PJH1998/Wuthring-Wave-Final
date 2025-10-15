#include "EnginePch.h"
#include "OctoTree.h"

COctoTree::COctoTree()
{
}

void COctoTree::SetUp_OctoTree(_float3 vCenter, _float3 vExtent, _uint iDepth)
{
}

void COctoTree::Add_ToOctoTree(CGameObject* pObject)
{
}

COctoTree* COctoTree::Create()
{
    return new COctoTree();
}

void COctoTree::Free()
{
	__super::Free();
}
