#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CShader;

class ENGINE_DLL CShadowMap final : public CBase
{
public:
	typedef struct tagShadowMapDesc
	{
		_uint iSectorSizeX;
		_uint iSectorSizeY;
		_uint iNumSectorX;
		_uint iNumSectorY;

		_float3 vCenterPos;
		_float3 vExtents;
		_float3 vLightDir;
	}SHADOW_MAP_DESC;

private:
	explicit CShadowMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CShadowMap() = default;

public:
	HRESULT						Initialize(const SHADOW_MAP_DESC& MapDesc);
	HRESULT						Bind_ShadowMap_Resources(CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pLightDirName);


private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	CGameInstance*				m_pGameInstance = { nullptr };

	SHADOW_MAP_DESC				m_MapDesc = {};

	ID3D11DepthStencilView*		m_pShadowMapDSV = { nullptr };
	ID3D11ShaderResourceView*	m_pShadowMapSRV = { nullptr };

	ID3D11RenderTargetView*		m_pBackBuffer = { nullptr };
	ID3D11DepthStencilView*		m_pOriginalDSV = { nullptr };

	vector<_float4x4>			m_Matrices[ENUM_CLASS(D3DTS::END)];
	vector<BoundingBox>			m_Boundings;

private:
	HRESULT				Ready_ShadowMap();
	HRESULT				Ready_Matrices();

	_float3				Compute_CenterPos(_int iWeightX, _int iWieghtY, _float3 vOriginPos, _float3 vExtents);
	_float				Compute_MaxRadius(const BoundingBox& Bounding, _float3 vCenterPos);
	_float4x4			Make_ViewMatrix(_float3 vCenterPos, const BoundingBox& Bounding, _float3 vDir);
	_float4x4			Make_ProjMatrix(BoundingBox Bounding, _float4x4 ViewMatrix);

public:
	static CShadowMap*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const SHADOW_MAP_DESC& MapDesc);
	virtual void		Free() override;
};

NS_END