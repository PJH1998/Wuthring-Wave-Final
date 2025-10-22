#pragma once
#include "Editor_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Point_Instance;
class CComputeShader;
NS_END

NS_BEGIN(Editor)

class CParticle final : public CGameObject
{
public:
	typedef struct tagParticleDesc : Engine::EFFECT_DESC
	{
		_wstring strTextureTag;
		//_wstring strShaderTag;
		_wstring strVIBufferTag;

		_int	fShaderPass = 0.f;
		_float3	vSize = { 1.f, 1.f, 1.f };
		_float3 vPos = { 0.f, 0.f, 0.f };
		_float3 vColor = { 0.f, 0.f, 0.f };
		_float2	vLifeTime = { 5.f, 10.f};

		//_bool	bSpread = false;
		//_bool	bDrop = false;
	}PARTICLE_DESC;

private:
	CParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticle(const CParticle& Prototype);
	virtual ~CParticle() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize_Clone(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual void Render();

private:
	CShader*					m_pShaderCom = { nullptr };
	CTexture*					m_pTextureCom = { nullptr };
	CVIBuffer_Point_Instance*	m_pVIBufferCom = { nullptr };
	CComputeShader*				m_pComputeShader = { nullptr };

	_float						m_fShaderPass = 0;
	_float3						m_vPos = {};
	_float3						m_vColor = {};
	_float2						m_vLifeTime = {};

	//?곗궛?? 媛以묒튂? ?쇰떒 ?쇰ℓ濡?遺덊??낆쑝濡??吏곸씠寃??숈옉 泥섎━留?
	//_bool						m_IsSpread = false;
	//_bool						m_IsDrop = false;

private:
	HRESULT Ready_Components(PARTICLE_DESC& Desc);
	HRESULT Bind_ShaderResources();

public:
	static CParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END