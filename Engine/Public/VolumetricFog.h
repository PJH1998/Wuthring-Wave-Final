#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CComputeShader;

class CVolumetricFog final : public CBase
{
private:
	typedef struct tagVF_Data{
		_float4x4 ViewMatrix;
		_float4x4 ProjMatrix;
		_float4x4 InvViewMatrix;
		_float4x4 InvProjMatrix;
		_float fNear;
		_float fFar;
		_uint iSliceCount;
		_uint iLightCount;
		_float3 vFroxelSize;
		_float Padding;
		_float fCamNear;
		_float fCamFar;
		_float fWinSizeX;
		_float fWinSizeY;
	}VF_DATA;

	enum class SRV { VF, LIGHT, END };
	enum class BUFFER { DATA, LIGHT, END};

private:
	CVolumetricFog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVolumetricFog() = default;

public:
	HRESULT						Initialize(_uint iWinSizeX, _uint iWinSizeY);
	HRESULT						SetUp_FogNF();
	void						Add_LightData(const VF_LIGHT& LightData);

	void						Render();

private:
	_float3						m_vFroxelSize = {};
	_float2						m_vFogRange = {};
	_float						m_vFov = {};
	_float3						m_vDefinition = {};
	VF_DATA						m_VF_Data = {};

	_uint						m_iMaxLight = {};
	vector<VF_LIGHT>			m_LightDatas;

	CGameInstance*				m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	
	CComputeShader*				m_pCS = { nullptr };

	ID3D11UnorderedAccessView*	m_pUAV = { nullptr };

	ID3D11ShaderResourceView*	m_pSRVs[ENUM_CLASS(SRV::END)];

	ID3D11Buffer*				m_pBuffers[ENUM_CLASS(BUFFER::END)] = {nullptr};

private:
	void						Update_HZB();
	void						Update_Buffer();
	HRESULT						Ready_FroxelVolume();
	HRESULT						Ready_Texture();
	HRESULT						Ready_Buffer();
	HRESULT						Ready_ComputeShader();


public:
	static CVolumetricFog*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, uint iWinSizeX, _uint iWinSizeY);
	virtual void				Free();
};

NS_END