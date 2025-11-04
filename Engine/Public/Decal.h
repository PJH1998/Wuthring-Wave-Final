#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class CTexture;
class CShader;
class CVIBuffer_Rect;

class CDecal final : public CBase
{
public:
	typedef struct tagDecalDesc {
		_float3 vScale;
		_float3 vRotation;
		_float3 vPosition;
		_float  fLifeTime;
	}DECAL_DESC;

	typedef struct tagDecalData {
		_float4x4	WorldMatrixInv;
		_float2		vLifeTime;
		_float		Padding[3];
	}DECAL_DATA;

private:
	explicit CDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CDecal() = default;

public:
	HRESULT						Initialize(CTexture* pTexture, _uint iMaxDecal);
	void						Update(_float fTimeDelta);
	HRESULT						Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
	void						Add_Decal(const DECAL_DESC& DecalDesc);

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	_uint						m_iMaxDecal = {};
	_uint						m_iNumDecals = {};
	list<DECAL_DATA>			m_DecalDatas = {};


	CTexture*					m_pDecalTexture = { nullptr };
	ID3D11Buffer*				m_pBuffer = { nullptr };
	ID3D11ShaderResourceView*	m_pSRV = { nullptr };

private:
	HRESULT						Ready_Buffers();
	void						Update_Buffer();
	
public:
	static CDecal*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CTexture* pTexture, _uint iMaxDecal);
	virtual void				Free() override;
};

NS_END