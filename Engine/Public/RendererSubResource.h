#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CTexture;
class CShader;
class CGameInstance;

class CRendererSubResource final : public CBase
{
private:
	typedef vector<pair<_int, vector<_float>>> BLUR_WEIGHTS;

private:
	typedef struct tagSSaoBlurData {
		_float  fSSAO_MinDepthDistance;
		_float	fWidth;
		_float	fHeight;
		_float  Paddingblur;
	}SSAO_BLUR_DATA;
	
	typedef struct tagBlurData {
		_float2  vSize;
		_int	 iRadius;
		_float   Padding;
	}BLUR_DATA;

	typedef struct tagBloomUpData {
		_float2  vSize;
		_float	fIntensity;
		_float   Padding;
	}BLOOM_UP_DATA;

	typedef struct tagMotionBlurData {
		_float fLimitVelocity;
		_float fLimitDepth;
		_float fLengthScale;
		_float PaddingMotion;
	}MOTION_BLUR_DATA;

private:
	explicit CRendererSubResource(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRendererSubResource() = default;

#ifdef _DEBUG
public:
	void Setting_SSAO(_float fRadius, _float fMaxDistance) { m_fRadius = fRadius, m_fMaxDistance = fMaxDistance; }
	void Setting_Fog(_float2 vDepthDistance, _float2 vHeightDistance, _float4 vColor) { m_vFogDepthDistance = vDepthDistance, m_vFogHeightDistance = vHeightDistance, m_vFogColor = vColor; }
	void SetBloomIntensity(_float fIntensity) { m_fIntensity = fIntensity; }
	void SetDof(_float fDepth, _float fRange, _float fScale) { m_fDofDepth = fDepth, m_fDofRange = fRange, m_fDofScale = fScale; }
	void SetMotionBlur(_float fLimitVelocity, _float fLimitDepth, _float fDistance) { m_fLimitVelocity = fLimitVelocity, m_fLimitDepth = fLimitDepth, m_fLengthScale = fDistance; }
#endif

public:
	HRESULT						Initialize();
	HRESULT						Bind_Ramp_Texture(CShader* pShader, const _char* pConstantName, _uint iTextureIndex);
	HRESULT						Bind_LUT_Texture(CShader* pShader, _uint iLUT_Index);
	HRESULT						Bind_SSAO_Resources(CShader* pShader);
	HRESULT						Bind_LimitVelocity(CShader* pShader);

	HRESULT						Bind_Fog_Resources(CShader* pShader);
	HRESULT						Bind_Dof_Resource(CShader* pShader);

	HRESULT						Add_SSAO_Blur_BufferData(const _wstring& strRCSTag, _float fWidth, _float fHeight);
	HRESULT						Add_Blur_BufferData(const _wstring& strRCSTag, _float fWidth, _float fHeight, _uint iBlurWeight = 1);
	HRESULT						Add_Bloom_BufferData(const _wstring& strRCSTag, _float fWidth, _float fHeight, _uint iUpIndex);
	HRESULT						Add_MotionBlur_BufferData(const _wstring& strRCSTag);
	HRESULT						Set_DefalutSampler(const _wstring& strRCSTag, _uint iSlot);

private:
	CGameInstance*				m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	
#pragma region RAMP
	CTexture*					m_pRampTexture = { nullptr };
#pragma endregion

#pragma region LUT
	CTexture*					m_pLUT_Texture = { nullptr };
	ID3D11ShaderResourceView*	m_pLUT_SRV = { nullptr };
	_uint						m_iNumLUT_Textures = {};
#pragma endregion

#pragma region SSAO
	CTexture*					m_pNoiseTexture = { nullptr };
	vector<_vector>				m_SSAO_SampleVector;
	_uint						m_iNumKernel = {};
	_float						m_fRadius = {};
	_float						m_fMaxDistance = {};
	_float						m_fOutDistance = {};

	_float						m_fSSAO_MinDepthDistance = {};
#pragma endregion

#pragma region Sampler
	ID3D11SamplerState*			m_pDefaultSampler = { nullptr };
	ID3D11SamplerState*			m_pPointClampSampler = { nullptr };
	ID3D11SamplerState*			m_pNoiseSampler = { nullptr };
#pragma endregion

#pragma region POG
	CTexture*					m_pFogNoiseTexture = { nullptr };
	_float2						m_vFogDepthDistance = {};
	_float2						m_vFogHeightDistance = {};
	_float						m_fFogTime = {};
	_float4						m_vFogColor = {};
	CTexture*					m_pHighCloudTexture = { nullptr };
#pragma endregion

#pragma region BLUR
	_uint								m_iNumWeights = {};
	BLUR_WEIGHTS						m_Weights;
	vector<ID3D11Buffer*>				m_WeightBuffers;
	vector<ID3D11ShaderResourceView*>	m_WeightSRVs;
	_float								m_fIntensity;
#pragma endregion

#pragma region DOF
	_float						m_fDofDepth = {};
	_float						m_fDofRange = {};
	_float						m_fDofScale = {};
#pragma endregion

#pragma region MOTION_BLUR		
	_float						m_fLimitVelocity = {};
	_float						m_fLimitDepth = {};
	_float						m_fLengthScale = {};
#pragma endregion

private:
	HRESULT						Ready_Shader_Filters();
	HRESULT						Ready_LUT_SRV();
	HRESULT						Ready_SSAO_SampleVector();
	HRESULT						Ready_CS_Sampler();
	HRESULT						Ready_BlurWeights();
	HRESULT						Create_BlurBuffer(const vector<_float>& Weights, _uint iRadius);

public:
	static CRendererSubResource*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void						Free() override;
};

NS_END