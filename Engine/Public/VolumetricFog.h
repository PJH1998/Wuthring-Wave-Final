#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CComputeShader;
class CShader;
class CTexture;

class CVolumetricFog final : public CBase
{
private:
	typedef struct tagVF_Data{
		_float4x4 PrevViewMatrix;	
		_float4x4 PrevProjMatrix;
		_float4x4 InvViewMatrix;
		_float4x4 InvProjMatrix;
		_float fNear;				// 0
		_float fFar;				// 4
		_uint iSliceCount;			// 8
		_uint iLightCount;			// 12
		_float3 vFroxelSize;		// 16
		_float Padding;				// 28
		_float fCamNear;			// 32
		_float fCamFar;				// 36
		_float fWinSizeX;			// 40
		_float fWinSizeY;			// 44
		_float fLightIntensity;		// 48
		_float fDensity;			// 52
		_float fPhaseFunctionG;		// 56
		_float fDensityScale;		// 60
		_float fFogMaxHeight;		// 64
		_float fFogMinHeight;		// 68
		_float fHegihtFallOff;		// 72
		_float fDistanceFallOff;	// 76
		_float fGroundFallOff;		// 80
		_float fNoiseScale;			// 84
		_float fNoiseTimeDelta;
		_bool  IsTemporal; 
		_float4 vCamPos;
		_float3 vFogColor;			// 96
		_uint iRandCount;
	}VF_DATA;

	enum class CS { VF_LIGHT, VF_BEER, VF_NOISE, END};
	enum class UAV { VF_LIGHT_FIRST, VF_LIGHT_SECOND, VF_BEER, VF_NOISE, END};
	enum class SRV { VF_LIGHT_FIRST, VF_LIGHT_SECOND, VF_BEER, VF_NOISE, LIGHT, END };
	enum class BUFFER { DATA, LIGHT, END};
	
private:
	CVolumetricFog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVolumetricFog() = default;

public:
	HRESULT						Initialize(_uint iWinSizeX, _uint iWinSizeY);
	HRESULT						SetUp_FogNF();

	void						Begin_VF();
	void						Clear();
	void						Update_VF(_float fTimeDelta);

	HRESULT						Bind_VF_Resource(CShader* pShader, const _char* pTextureName, const _char* pFogRangeName);

//#ifdef _DEBUG
public:
	void						Setting_VF();
	_float TestScale = {  };
//#endif

private:
	_float3						m_vFroxelSize = {};
	_float2						m_vFogRange = {};
	_float						m_vFov = {};
	_float3						m_vDefinition = {};
	VF_DATA						m_VF_Data = {};

	_bool						m_IsUpdate = { false };
	_bool						m_IsFirst = { false };
	_float3						m_vNoiseSize = {};

	_uint						m_iMaxLight = {};
	vector<LIGHT_DATA>			m_LightDatas;

	CGameInstance*				m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	
	CComputeShader*				m_pCS[ENUM_CLASS(CS::END)] = { nullptr };

	ID3D11UnorderedAccessView*	m_pUAVs[ENUM_CLASS(UAV::END)] = { nullptr };

	ID3D11ShaderResourceView*	m_pSRVs[ENUM_CLASS(SRV::END)] = { nullptr };

	ID3D11Buffer*				m_pBuffers[ENUM_CLASS(BUFFER::END)] = {nullptr};

	_uint						m_iWriteIndex = {};
	_uint						m_iReadIndex = {};

	ID3D11SamplerState*			m_pDefaultSampler = { nullptr };
	ID3D11SamplerState*			m_pShadowSampler = { nullptr };

private:
	void						Update_Buffer(_float fTimeDelta);
	HRESULT						Ready_FroxelVolume();
	HRESULT						Ready_FogTexture(_uint iTextureIndex);
	HRESULT						Ready_NoiseTexture();
	HRESULT						Ready_Buffer();
	HRESULT						Ready_ComputeShader();
	HRESULT						Ready_Sampler();

	void						Make_NoiseTexture();

public:
	static CVolumetricFog*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, uint iWinSizeX, _uint iWinSizeY);
	virtual void				Free();
};

NS_END