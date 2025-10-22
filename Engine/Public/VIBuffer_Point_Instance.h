#pragma once

#include "VIBuffer_Instance.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Point_Instance final : public CVIBuffer_Instance
{
public:
	typedef struct tagPointInstanceDesc : public CVIBuffer_Instance::INSTANCE_DESC
	{
		_float3		vPivot;
		_float2		vSpeed;
		_float2		vLifeTime;
		_bool		IsLoop;
		// if 0 ~ 1
		_float		fSpreadWeight = 0;
		_float		fDropWeight = 0;
		_float		fRotationWeight = 0;
		_float		fGravity = 9.8f;	//?섏튂怨좊?
	}POINT_INSTANCE_DESC;

private:
	CVIBuffer_Point_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Point_Instance(const CVIBuffer_Point_Instance& Prototype);
	virtual ~CVIBuffer_Point_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype(const INSTANCE_DESC* pDesc) override;
	virtual HRESULT Initialize_Clone(void* pArg) override;
	virtual HRESULT Bind_Resources() override;
	virtual HRESULT Render() override;

	void Bind_CSResources(class CComputeShader* pCShader, _float fTimeDelta);

public:
	void Spread(_float fTimeDelta);
	void Drop(_float fTimeDelta);
	void Rotation(_float fTimeDelta);

private:
	_float3					m_vPivot = {};
	_float*					m_pSpeeds = {};
	_bool					m_isLoop = {};

	ID3D11Buffer*		m_pCBBuffer = {};
	ID3D11Buffer*		m_pSRVBuffer = {};
	ID3D11Buffer*       m_pUABuffer = {};

	ID3D11ShaderResourceView*	m_pSRV = {};
	ID3D11UnorderedAccessView*	m_pUAV = {};

public:
	static CVIBuffer_Point_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const INSTANCE_DESC* pDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END