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
	m_Desc{ Prototype.m_Desc }
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

HRESULT CMesh_Streaming::Render(_uint iMeshIndex)
{
	//Model_Manager에서 바인딩 된 놈들만 한 번에 Draw. 디퍼드 컨텍스트 이용할 거면 나중에 따로 생성.
	//LOD3번은 로딩 다 될 때까지 기다려야함. 
	if (m_Desc->empty())
		CRASH("Failed");

	if (iMeshIndex >= m_Desc->size())
		return E_FAIL;

	const CModel_Manager::SHARED_DATA_DESC& RenderDesc = (*m_Desc)[iMeshIndex];

	//임시 하드코딩
	//나중에 방식을 오프셋을 건드리기 vs 바인딩을 버퍼 하나에서 오프셋을 바꾸기에서 선택할것.
	m_pContext->DrawIndexed(RenderDesc.NumIndices, RenderDesc.IndexOffset / 4, RenderDesc.VertexOffset / sizeof(VTXMESH));
	//m_pContext->DrawIndexed(RenderDesc.NumIndices, 0, 0);
	return S_OK;
}

void CMesh_Streaming::Set_Buffers(ID3D11Buffer* pSharedVB, ID3D11Buffer* pSharedIB)
{
	m_pSharedVB = pSharedVB;
	m_pSharedIB = pSharedIB;
	Safe_AddRef(m_pSharedVB);
	Safe_AddRef(m_pSharedIB);
}

HRESULT CMesh_Streaming::Bind_Resources(_uint iMeshIndex)
{
	ID3D11Buffer* Buffers[] = {
		m_pSharedVB,
	};

	_uint Strides[] = {
		m_iVertexStride,
	};

	_uint Offsets[] = {
		0,
	};
	m_pContext->IASetVertexBuffers(0, 1, Buffers, Strides, Offsets);
	m_pContext->IASetIndexBuffer(m_pSharedIB, m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

HRESULT CMesh_Streaming::Render(_uint iMeshIndex, ID3D11DeviceContext* pDC)
{
	if (m_Desc->empty())
		CRASH("Failed");

	if (iMeshIndex >= m_Desc->size())
		return E_FAIL;

	const CModel_Manager::SHARED_DATA_DESC& RenderDesc = (*m_Desc)[iMeshIndex];

	//임시 하드코딩
	pDC->DrawIndexed(RenderDesc.NumIndices, RenderDesc.IndexOffset / 4, RenderDesc.VertexOffset / sizeof(VTXMESH));
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