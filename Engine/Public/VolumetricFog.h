#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CComputeShader;
class CShader;

class CVolumetricFog final : public CBase
{
private:
	typedef struct tagVF_Data{
		_float4x4 ViewMatrix;	
		_float4x4 ProjMatrix;
		_float4x4 InvViewMatrix;
		_float4x4 InvProjMatrix;
		_float fNear;				// 4
		_float fFar;				// 8
		_uint iSliceCount;			// 12
		_uint iLightCount;			// 16
		_float3 vFroxelSize;		// 32
		_float Padding;
		_float fCamNear;			// 36
		_float fCamFar;				// 40
		_float fWinSizeX;			// 44
		_float fWinSizeY;			// 48
		_float fLightIntensity;		// 52
		_float fDensity;			// 56
		_float fPhaseFunctionG;		// 60
		_float fDensityScale;		// 64
		_float fFogMaxHeight;		// 68
		_float fFogMinHeight;		// 72
		_float fHegihtFallOff;		// 76
		_float fDistanceFallOff;	// 80
		_float fGroundFallOff;		// 84
		_float3 Padding1;
		_float3 vFogColor;			// 96
		_float Padding2;
	}VF_DATA;

	enum class CS { VF_LIGHT, VF_BEER, END};
	enum class UAV { VF_LIGHT, VF_BEER, END};
	enum class SRV { VF_LIGHT, VF_BEER, LIGHT, END };
	enum class BUFFER { DATA, LIGHT, END};

private:
	CVolumetricFog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVolumetricFog() = default;

public:
	HRESULT						Initialize(_uint iWinSizeX, _uint iWinSizeY);
	HRESULT						SetUp_FogNF();
	void						Add_LightData(const VF_LIGHT& LightData);

	void						Update_VF();

	HRESULT						Bind_VF_Resource(CShader* pShader, const _char* pTextureName, const _char* pFogRangeName);

#ifdef _DEBUG
public:
	void						Setting_VF();
#endif

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
	
	CComputeShader*				m_pCS[ENUM_CLASS(CS::END)] = { nullptr };

	ID3D11UnorderedAccessView*	m_pUAVs[ENUM_CLASS(UAV::END)] = { nullptr };

	ID3D11ShaderResourceView*	m_pSRVs[ENUM_CLASS(SRV::END)];

	ID3D11Buffer*				m_pBuffers[ENUM_CLASS(BUFFER::END)] = {nullptr};

	ID3D11SamplerState*			m_pDefaultSampler = { nullptr };
	ID3D11SamplerState*			m_pShadowSampler = { nullptr };

private:
	void						Update_Buffer();
	HRESULT						Ready_FroxelVolume();
	HRESULT						Ready_Texture(_uint iTextureIndex);
	HRESULT						Ready_Buffer();
	HRESULT						Ready_ComputeShader();
	HRESULT						Ready_Sampler();

public:
	static CVolumetricFog*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, uint iWinSizeX, _uint iWinSizeY);
	virtual void				Free();
};

NS_END