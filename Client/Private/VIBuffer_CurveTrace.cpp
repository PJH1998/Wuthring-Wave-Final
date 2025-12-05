#include "ClientPch.h"
#include "VIBuffer_CurveTrace.h"

CVIBuffer_CurveTrace::CVIBuffer_CurveTrace(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_CurveTrace::CVIBuffer_CurveTrace(const CVIBuffer_CurveTrace& Prototype)
	: CVIBuffer(Prototype)
	, m_iMaxSegmentCount(Prototype.m_iMaxSegmentCount)
	, m_iSegmentUsing(Prototype.m_iSegmentUsing)
	, m_iVertexCount(Prototype.m_iVertexCount)
{
}

CVIBuffer_CurveTrace::~CVIBuffer_CurveTrace(void)
{
}

HRESULT CVIBuffer_CurveTrace::Initialize_Prototype(_uint iMaxSegmentCount)
{
	if (iMaxSegmentCount == 0)
		iMaxSegmentCount = 1;

	m_iMaxSegmentCount = iMaxSegmentCount;


	const _uint iMaxVertexCount = (m_iMaxSegmentCount + 1) * 2; // 좌/우
	const _uint iMaxIndexCount = m_iMaxSegmentCount * 6;		// 삼각형 2개 * 3 

	m_iNumVertexBuffers = 1;

	//VB
	m_iMaxSegmentCount = iMaxSegmentCount;
	m_iVertexCount = iMaxVertexCount; // m_iMaxSegmentCount + 1;	// 세그먼트(직선) n개. 즉, vertex(점)은 n+1개

	m_iVertexStride = sizeof(VTXUICURVE);
	m_iNumVertices = m_iVertexCount;
	m_iNumIndices = iMaxIndexCount;	

	m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;

	// VB 생성
	D3D11_BUFFER_DESC tVBDesc = {};
	tVBDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	tVBDesc.Usage = D3D11_USAGE_DYNAMIC;				//
	tVBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	tVBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//


	if (FAILED(m_pDevice->CreateBuffer(&tVBDesc, nullptr, &m_pVB)))
		return E_FAIL;

	// IB
	vector<_uint> vecIndices;
	vecIndices.resize(iMaxIndexCount);

	_uint idx = 0;
	for (_uint seg = 0; seg < m_iMaxSegmentCount; ++seg)
	{
		_uint v0 = seg * 2;
		_uint v1 = seg * 2 + 1;
		_uint v2 = (seg + 1) * 2;
		_uint v3 = (seg + 1) * 2 + 1;

		// 삼각형 1: v0, v2, v1
		vecIndices[idx++] = v0;
		vecIndices[idx++] = v2;
		vecIndices[idx++] = v1;

		// 삼각형 2: v1, v2, v3
		vecIndices[idx++] = v1;
		vecIndices[idx++] = v2;
		vecIndices[idx++] = v3;
	}

	D3D11_BUFFER_DESC tIBDesc = {};
	tIBDesc.ByteWidth = sizeof(_uint) * iMaxIndexCount;
	tIBDesc.Usage = D3D11_USAGE_DYNAMIC;
	tIBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	tIBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA tIBData = {};
	tIBData.pSysMem = vecIndices.data();

	if (FAILED(m_pDevice->CreateBuffer(&tIBDesc, &tIBData, &m_pIB)))
		return E_FAIL;

	// 초기값
	m_iSegmentUsing = 0;
	m_iVertexCount = 0;
	m_iNumIndices = 0; // 초기에는 그릴 것 X

	return S_OK;
}

HRESULT CVIBuffer_CurveTrace::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_CurveTrace::UpdateVertices(const VTXUICURVE* pVertices, _uint iSegmentCount)
{
	if (nullptr == m_pVB || nullptr == pVertices)
		return E_FAIL;

	if (iSegmentCount == 0)
		return E_FAIL;

	if (iSegmentCount > m_iMaxSegmentCount)
		iSegmentCount = m_iMaxSegmentCount;

	m_iSegmentUsing = iSegmentCount;
	m_iVertexCount = (m_iSegmentUsing + 1) * 2;

	m_iNumVertices = m_iVertexCount;
	m_iNumIndices = m_iSegmentUsing * 6;    // 세그먼트마다 6개

	D3D11_MAPPED_SUBRESOURCE tMapped = {};

	if (FAILED(m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &tMapped)))
		return E_FAIL;

	memcpy(tMapped.pData, pVertices, sizeof(VTXUICURVE) * m_iVertexCount);

	m_pContext->Unmap(m_pVB, 0);

	return S_OK;
}

HRESULT CVIBuffer_CurveTrace::Render()
{
    if (nullptr == m_pVB || nullptr == m_pIB)
        return E_FAIL;

    if (m_iNumIndices == 0)
        return S_OK;

    m_pContext->DrawIndexed(m_iNumIndices, 0, 0);

    return S_OK;
}

CVIBuffer_CurveTrace* CVIBuffer_CurveTrace::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iMaxSegmentCount)
{
	CVIBuffer_CurveTrace* pInstance = new CVIBuffer_CurveTrace(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(iMaxSegmentCount)))
	{
		MSG_BOX("Failed to Create : VIBuffer_CurveTrace");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_CurveTrace::Clone(void* pArg)
{
	CVIBuffer_CurveTrace* pClone = new CVIBuffer_CurveTrace(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : VIBuffer_CurveTrace (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CVIBuffer_CurveTrace::Free(void)
{
	__super::Free();
}
