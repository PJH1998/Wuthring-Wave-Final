#include"EnginePch.h"
#include"Mesh_Streaming.h"

CMesh_Streaming::CMesh_Streaming(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CComponent(pDevice,pContext)
{
}

CMesh_Streaming::CMesh_Streaming(const CMesh_Streaming& Prototype)
	:CComponent(Prototype), m_iNumMeshes{Prototype.m_iNumMeshes },
	m_pSharedVB{Prototype.m_pSharedVB},
	m_pSharedIB{Prototype.m_pSharedIB},
	m_Desc{Prototype.m_Desc }
{
	Safe_AddRef(m_pSharedVB);
	Safe_AddRef(m_pSharedIB);
}

HRESULT CMesh_Streaming::Initialize_Prototype(_uint iNumMeshes)
{
	m_iNumMeshes = iNumMeshes;
	m_Desc = new vector<CModel_Manager::SHARED_DATA_DESC>;
	//모델매니저에서 버퍼 풀 주소 받기.
	return S_OK;
}

HRESULT CMesh_Streaming::Initialize_Clone(void* pArg)
{
	return S_OK;
}

HRESULT CMesh_Streaming::Render(_uint iNumMeshIndex)
{
	//Model_Manager에서 바인딩 된 놈들만 한 번에 Draw. 디퍼드 컨텍스트 이용할 거면 나중에 따로 생성.
	if (m_Desc->empty())
		CRASH("Failed");

	if (iNumMeshIndex >= m_Desc->size())
		return E_FAIL;

	const CModel_Manager::SHARED_DATA_DESC& RenderDesc = (*m_Desc)[iNumMeshIndex];

	//임시 하드코딩
	m_pContext->DrawIndexed(RenderDesc.NumIndices, RenderDesc.IndexOffset / 4, RenderDesc.VertexOffset / sizeof(VTXMESH));
	return S_OK;
}

CMesh_Streaming* CMesh_Streaming::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumMeshes)
{
	CMesh_Streaming* pInstance = new CMesh_Streaming(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(iNumMeshes)))
	{
		MSG_BOX("Failed to Create : Mesh_Streaming");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CMesh_Streaming::Clone(void* pArg)
{
	CMesh_Streaming* pInstance = new CMesh_Streaming(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Mesh_Streaming (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMesh_Streaming::Free()
{
	__super::Free();
	Safe_Release(m_pSharedVB);
	Safe_Release(m_pSharedIB);
	if (!m_isClone)
		Safe_Delete(m_Desc);
}