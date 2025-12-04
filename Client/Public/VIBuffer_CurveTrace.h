#pragma once
#include "VIBuffer.h"


NS_BEGIN(Client)

class CVIBuffer_CurveTrace : public CVIBuffer
{
private:
	explicit CVIBuffer_CurveTrace(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_CurveTrace(const CVIBuffer_CurveTrace& Prototype);
	virtual ~CVIBuffer_CurveTrace(void);

public:
	virtual HRESULT Initialize_Prototype(_uint iMaxSegmentCount);
	virtual HRESULT Initialize_Clone(void* Prototype) override;
	virtual HRESULT Render(void) override;

public:
	// 궤적 버텍스를 외부에서 채워서 넘길 때 사용
	HRESULT UpdateVertices(const VTXUICURVE* pVertices, _uint iVertexCount);
	_uint Get_VertexCount(void) const { return m_iVertexCount; }

#ifdef _DEBUG
	ID3D11Buffer* Get_VB() { return m_pVB; };
#endif // _DEBUG


private:
	_uint m_iMaxSegmentCount = 0;	// 최대 세그먼트(선분) 수
	_uint m_iSegmentUsing = 0;
	_uint m_iVertexCount = 0;		// 실제 사용 중인 버텍스 수
	//D3D11_PRIMITIVE_TOPOLOGY m_ePrimitiveType = {};

private:
	virtual void Free(void) override;

public:
	static CVIBuffer_CurveTrace* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iMaxSegmentCount);
	virtual CComponent* Clone(void* pArg) override;
};

NS_END