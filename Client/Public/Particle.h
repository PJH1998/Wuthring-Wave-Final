#pragma once
#include "Client_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Point_Instance;
class CComputeShader;
NS_END

NS_BEGIN(Client)

class CParticle final : public CGameObject
{
public:
	typedef struct tagParticleDesc : Engine::EFFECT_DESC
	{
		_wstring strTextureTag;
		_wstring strVIBufferTag;

		_int	iShaderPass = 0.f;
		_float3	vSize = { 1.f, 1.f, 1.f };
		_float3 vPos = { 0.f, 0.f, 0.f };
		_float4 vColor = { 0.f, 0.f, 0.f, 0.f };
		_float2	vLifeTime = { 0.f, 10.f};

		_bool	IsSprite = false;
		_int    iRows = 0;
		_int	iCols = 0;
	}PARTICLE_DESC;

private:
	CParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticle(const CParticle& Prototype);
	virtual ~CParticle() = default;

public:
	virtual HRESULT Initialize_Prototype(const PARTICLE_DESC* pDesc);
	virtual HRESULT Initialize_Clone(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual void Render();
	
public:
	virtual		void	Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	void Root_Transform(_fmatrix WorldMatrix);
	void Bind_CS_SpriteInfo();
	

private:
	CShader*					m_pShaderCom = { nullptr };
	CTexture*					m_pTextureCom = { nullptr };
	CVIBuffer_Point_Instance*	m_pVIBufferCom = { nullptr };
	CComputeShader*				m_pComputeShader = { nullptr };

	//원형이 가지고 있을 정보
	PARTICLE_DESC				m_tDesc = {};

	_int						m_iShaderPass = 0;
	_float3						m_vPos = {};
	_float4						m_vColor = {};
	_float2						m_vLifeTime = {};

	_bool						m_IsSprite = false;
	_int						m_iRow = {};
	_int						m_iCol = {};

private:
	HRESULT Ready_Components(PARTICLE_DESC& Desc);
	HRESULT Bind_ShaderResources();

public:
	static CParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const PARTICLE_DESC* pDesc);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END