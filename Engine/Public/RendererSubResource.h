#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CTexture;
class CShader;
class CGameInstance;

class CRendererSubResource final : public CBase
{
private:
	typedef struct tagSSaoData {
		_float4		vSampleVector[16];
		_float4x4	CamViewMatrix;
		_float4x4	CamProjMatrix;
		_float4x4	ProjMatrixInv;
		_float		fWidth;
		_float		fHeight;
		_int		iSampleSize;
		_float		fSSAO_Radius;
		_float		fSSAO_MaxDistance;
		_float3		Padding;
	}SSAO_DATA;

private:
	CRendererSubResource(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRendererSubResource() = default;

public:
	HRESULT						Initialize();
	HRESULT						Bind_Ramp_Texture(CShader* pShader, const _char* pConstantName, _uint iTextureIndex);
	HRESULT						Bind_LUT_Texture(CShader* pShader, const _char* pConstantName, _uint iLUT_Index, const _char* pIndexConstantName);
	HRESULT						Bind_Noise_Texture(CShader* pShader, const _char* pConstantName);
	HRESULT						Bind_Sample_Vector(CShader* pShader, const _char* pConstantName);
	
	HRESULT						Add_SSAO_BufferData(const _wstring& strRCSTag, _float fWidth, _float fHeight);
	HRESULT						Add_SSAO_NoiseTexture(const _wstring& strRCSTag, const _char* pConstantName);
private:
	CGameInstance*				m_pGameInstance = { nullptr };

	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	CTexture*					m_pRampTexture = { nullptr };

	CTexture*					m_pLUT_Texture = { nullptr };
	ID3D11ShaderResourceView*	m_pLUT_SRV = { nullptr };
	_uint						m_iNumLUT_Textures = {};

	CTexture*					m_pNoiseTexture = { nullptr };
	vector<_vector>				m_SSAO_SampleVector;
	_uint						m_iNumKernel = {};

	ID3D11SamplerState*			m_pDefaultSampler = { nullptr };
	ID3D11SamplerState*			m_pPointClampSampler = { nullptr };
	ID3D11SamplerState*			m_pNoiseSampler = { nullptr };

private:
	HRESULT						Ready_Shader_Filters();
	HRESULT						Ready_LUT_SRV();
	HRESULT						Ready_SSAO_SampleVector();
	HRESULT						Ready_CS_Sampler();

public:
	static CRendererSubResource*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void				Free() override;
};

NS_END