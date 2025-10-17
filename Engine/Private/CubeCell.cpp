#include "EnginePch.h"
#include "CubeCell.h"

#include "GameInstance.h"
#include "StaticObject.h"

CCubeCell::CCubeCell()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
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

	m_pBoundingBox->GetCorners(m_Corners);
	Compute_MinMax();

	if (MAX_DEPTH == iDepth)
		return S_OK;

	for (_uint i = 0; i < ENUM_CLASS(CORNER::END); ++i)
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

void CCubeCell::Priority_Update(_float fTimeDelta)
{
	for (auto& pObject : m_Objects)
		pObject->Priority_Update(fTimeDelta);
}

void CCubeCell::Update(_float fTimeDelta)
{
	for (auto& pObject : m_Objects)
		pObject->Update(fTimeDelta);
}

void CCubeCell::Late_Update(_float fTimeDelta)
{
	for (auto& pObject : m_Objects)
		pObject->Late_Update(fTimeDelta);
}

void CCubeCell::Add_Object(CStaticObject* pObject, const _float* pMinMax)
{

}

_bool CCubeCell::isIn(const _float* pMinMax)
{
	//_float3 vCorners[ENUM_CLASS(CORNER::END)];
	//pBox->GetCorners(vCorners);
	//
	//pBox.
	//if(m_Corners[ENUM_CLASS(CORNER::LBU)])

	return _bool();
}

void CCubeCell::Compute_MinMax()
{
	m_MinMax[ENUM_CLASS(MINMAX::MIN_X)] = m_Corners[ENUM_CLASS(CORNER::LBD)].x;
	m_MinMax[ENUM_CLASS(MINMAX::MAX_X)] = m_Corners[ENUM_CLASS(CORNER::RBD)].x;

	m_MinMax[ENUM_CLASS(MINMAX::MIN_Y)] = m_Corners[ENUM_CLASS(CORNER::LBD)].y;
	m_MinMax[ENUM_CLASS(MINMAX::MAX_Y)] = m_Corners[ENUM_CLASS(CORNER::LBU)].y;

	m_MinMax[ENUM_CLASS(MINMAX::MIN_Z)] = m_Corners[ENUM_CLASS(CORNER::LBD)].z;
	m_MinMax[ENUM_CLASS(MINMAX::MAX_Z)] = m_Corners[ENUM_CLASS(CORNER::LFD)].z;
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

	Safe_Release(m_pGameInstance);
}
