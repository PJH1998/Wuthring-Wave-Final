#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)

class CVIBuffer_Decal final : public CVIBuffer
{
private:
	explicit CVIBuffer_Decal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Decal(const CVIBuffer_Decal& Prototype) = delete;
	virtual ~CVIBuffer_Decal() = default;

public:
	virtual HRESULT		Initialize();
	virtual HRESULT		Render() override;
	virtual HRESULT		Bind_Resources() override;

	HRESULT				Update_Buffer(const list<VTXINSTANCE_DECAL>& Datas);
	void				Clear();
private:
	ID3D11Buffer*	m_pVBInstance = { nullptr };

	_uint			m_iNumInstance = {};
	_uint			m_iNumIndexPerInstance = {};
	_uint			m_iInstanceVertexStride = {};

public:
	static CVIBuffer_Decal*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*		Clone(void* pArg) { return nullptr; }
	virtual void			Free()override;
};

NS_END