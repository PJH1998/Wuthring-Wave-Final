#include "EnginePch.h"
#include "CubeCell.h"

CCubeCell::CCubeCell()
{
}

HRESULT CCubeCell::Initialize(_float3 vCenter, _float3 vExtent, _uint iDepth)
{
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
}
