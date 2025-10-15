#include "EnginePch.h"
#include "CubeCell.h"

#include "StaticObject.h"

CCubeCell::CCubeCell()
{
}

HRESULT CCubeCell::Initialize(_float3 vCenter, _float3 vExtent, _uint iDepth)
{
	_float3 Extent = vExtent;
	if (0 < iDepth)
	{
		XMStoreFloat3(&Extent, XMLoadFloat3(&Extent) * 2.f);
		m_pBoundingBox = new BoundingBox(vCenter, Extent);
	}
	else
		m_pBoundingBox = new BoundingBox(vCenter, vExtent);

	ASSERT_CRASH(m_pBoundingBox);

	if (MAX_DEPTH == iDepth)
		return S_OK;

	for (_uint i = 0; i < ENUM_CLASS(TYPE::END); ++i)
	{
		_float3 vOffset = {};
		vOffset.x = (i & 1) ? 0.25f : -0.25f;
		vOffset.y = (i & 4) ? -0.25f : 0.25f;
		vOffset.z = (i & 2) ? 0.25f : -0.25f;

		_float3 vChildCenter = _float3(
			vCenter.x + vOffset.x * vExtent.x,
			vCenter.y + vOffset.y * vExtent.y,
			vCenter.z + vOffset.z * vExtent.z
		);

		_float3 vChildExtent = _float3(
			vExtent.x * 0.5f,
			vExtent.y * 0.5f,
			vExtent.z * 0.5f
		);

		CCubeCell* pCubeCell = CCubeCell::Create(vChildCenter, vChildExtent, iDepth + 1);
		ASSERT_CRASH(pCubeCell);
		m_ChildCells.push_back(pCubeCell);
	}

    return S_OK;
}

void CCubeCell::Update(_float fTimeDelta)
{
}

void CCubeCell::Render()
{
}

CCubeCell* CCubeCell::Create(_float3 vCenter, _float3 vExtent, _uint iDepth)
{
	CCubeCell* pInstance = new CCubeCell();

	if (FAILED(pInstance->Initialize(vCenter, vExtent, iDepth)))
	{
		MSG_BOX("Failed to Create : CubeCell");
		Safe_Release(pInstance);
	}

    return pInstance;
}

void CCubeCell::Free()
{
	__super::Free();

	Safe_Delete(m_pBoundingBox);
	
	for (auto& Object : m_Objects)
		Safe_Release(Object);
	m_Objects.clear();

	for (auto& Child : m_ChildCells)
		Safe_Release(Child);
	m_ChildCells.clear();
}
