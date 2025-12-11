#pragma once
#include "Client_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Spectrum;
//class CComputeShader;
NS_END

NS_BEGIN(Client)

class CSpectrum final : public CGameObject
{
public:
	typedef struct tagSpectrumDesc : Engine::EFFECT_DESC
	{
		_wstring strTextureTag;
		_wstring strColorTextureTag;
		_wstring strVIBufferTag;

		_int	iShaderPass = 0;

		_float	fLifeTime = 0.f;
		_float  fGeneration = 0.f;
		
		_float fMaskSpeed = 1.f;
		_float fAlpha = 1.f;

	}SPECTRUM_DESC;

private:
	CSpectrum(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSpectrum(const CSpectrum& Prototype);
	virtual ~CSpectrum() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize_Clone(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual void Render();
	

public:
	virtual		void	Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	void Update_Position();

private:
	CShader*					m_pShaderCom = { nullptr };
	CTexture*					m_pTextureCom = { nullptr };
	CTexture*					m_pColorTextureCom = { nullptr };
	CVIBuffer_Spectrum*			m_pVIBufferCom = { nullptr };
	//CComputeShader*			m_pComputeShaderCom = { nullptr };

	SPECTRUM_DESC				m_tDesc = {};

	_float						m_fCurrentTime = 0.f;
	_float						m_fSpawnTimer = 0.f;
	_float						m_fMaskSpeed = 1.f;
	_float						m_fGeneration = 0.f;

	_int						m_iShaderPass = 0;
	_float						m_fLifeTime = {};
	_float						m_fMinDistance = 0.1f;

	_float3						m_vPreviousPos = {};

	_int						m_SamleCount = 0;
	deque<SAMPLE_DESC>			m_Samples = {};

	const _float4x4*			m_pBoneMatrixPtr = nullptr;
	const _float4x4*			m_pObjectMatrixPtr = nullptr;
	_vector						m_UpdatePosition = {};
	_bool*						m_pIsActive = nullptr;

	_bool						m_IsObectActive = false;

private:
	HRESULT Ready_Components(SPECTRUM_DESC& Desc);
	HRESULT Bind_ShaderResources();

public:
	static CSpectrum* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END