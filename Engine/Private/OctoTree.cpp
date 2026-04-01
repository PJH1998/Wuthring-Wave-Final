#include "EnginePch.h"
#include "OctoTree.h"

#include "CubeCell.h"
#include "GameInstance.h"

COctoTree::COctoTree()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

void COctoTree::SetUp_OctoTree(const _float3& vCenter, const _float3& vExtent)
{
	Safe_Release(m_pRootCell);
	m_pRootCell = CCubeCell::Create(vCenter, vExtent, 0);
	ASSERT_CRASH(m_pRootCell);
}

void COctoTree::Add_To_OctoTree(CStaticObject* pObject, const BoundingBox* pBox)
{
	ASSERT_CRASH(m_pRootCell);

	_float fMinMax[ENUM_CLASS(CCubeCell::MINMAX::END)] = {};

	for (_int i = 0; i < ENUM_CLASS(CCubeCell::MINMAX::END); ++i)
		fMinMax[i] = *(reinterpret_cast<const _float*>(&(pBox->Center.x)) + i / 2) + 
							pow(-1.f, i + 1) * (*(reinterpret_cast<const _float*>(&(pBox->Extents.x)) + i / 2) * 0.5f);

	m_pRootCell->Add_Object(pObject, fMinMax);
}

void COctoTree::Clear_OctoTree()
{
	Safe_Release(m_pRootCell);
}

void COctoTree::Update()
{
	if (nullptr == m_pRootCell)
		return;

	vector<CStaticObject*> Container[4];
	Container[0].reserve(MAX_OBJECT_PER_LOD);
	Container[1].reserve(MAX_OBJECT_PER_LOD);
	Container[2].reserve(MAX_OBJECT_PER_LOD);
	Container[3].reserve(MAX_OBJECT_PER_LOD);

	m_pRootCell->Update(XMLoadFloat4(m_pGameInstance->Get_CamPos()), Container);
}

COctoTree* COctoTree::Create()
{
    return new COctoTree();
}

void COctoTree::Free()
{
	__super::Free();

	Safe_Release(m_pRootCell);

	Safe_Release(m_pGameInstance);
}
