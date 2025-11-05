#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CShader;
class CStaticObject;

class CShadowMap final : public CBase
{
public:
	typedef struct tagShadowMapData {
		_int iNumSectorX;            
		_int iNumSectorToLayer;
		_float Padding0[2];
		_float2 vSectorWorldSize;
		_float Padding1[2];
		_float2 vMin;  
		_float Padding2[2];
	}SHADOW_MAP_DATA;

private:
	explicit CShadowMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CShadowMap() = default;

public:
	const _uint					Get_ShadowMapLayer(_uint iSectorIndex) { return (iSectorIndex / m_iNumSectorToLayer); }
	const vector<BoundingBox*>& Get_ShadowMapSectors() { return m_Boundings; }

public:
	HRESULT						Setting_ShadowMap(const SHADOW_MAP_DESC& MapDesc);
	HRESULT						Bind_ShadowMap_Resources(CShader* pShader, const _char* pViewName, const _char* pProjName, _uint iSector);	// STATIC_OBJECT
	HRESULT						Bind_ShadowMap_Resources(CShader* pShader);																	// RENDERER
	HRESULT						Bind_ShadowMap_Buffer(_uint iBufferIndex);

	HRESULT						Begin_ShadowMap();
	HRESULT						End_ShadowMap();

#ifdef _DEBUG
	void						Render(CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#endif

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	CGameInstance*				m_pGameInstance = { nullptr };

	SHADOW_MAP_DESC				m_MapDesc = {};

	ID3D11RenderTargetView*		m_pBackBuffer = { nullptr };
	ID3D11DepthStencilView*		m_pOriginalDSV = { nullptr };

	ID3D11DepthStencilView*		m_pShadowMapDSV = { nullptr };
	ID3D11ShaderResourceView*	m_pShadowMapSRV = { nullptr };

	ID3D11Buffer*				m_pConstantBuffer = { nullptr };
	
	_uint						m_iNumSector = {};
	_uint						m_iNumLayer = {};
	_uint						m_iNumSectorX_ToLayer = {};
	_uint						m_iNumSectorZ_ToLayer = {};
	_uint						m_iNumSectorToLayer = {};

	vector<_float4x4>			m_Matrices[ENUM_CLASS(D3DTS::END)];
	vector<BoundingBox*>		m_Boundings;

	vector<_float4>				m_SectorUV;

	_uint						m_iShadowMapSizeX = {};
	_uint						m_iShadowMapSizeY = {};

	_float2						m_vShadowMapSize = {};
	_float2						m_vSectorSize = {};

private:
	void						Setting_ShadowMapViewPort(_uint iSector);
	
	HRESULT						Ready_ShadowMap();
	HRESULT						Ready_Buffers();
	HRESULT						Ready_SectorUV();
	HRESULT						Ready_Matrices();

	_float3						Compute_CenterPos(_int iWeightX, _int iWeightZ, _float3 vOriginPos, _float3 vExtents);
	_float						Compute_MaxRadius(const BoundingBox* Bounding, _float3 vCenterPos);
	_float4x4					Make_ViewMatrix(_float3 vCenterPos, const BoundingBox* Bounding, _float3 vDir);
	_float4x4					Make_ProjMatrix(const BoundingBox* Bounding, _float4x4 ViewMatrix);


public:
	static CShadowMap*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void		Free() override;
};

NS_END