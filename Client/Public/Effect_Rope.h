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

class CEffect_Rope final : public CGameObject
{
private:
	CEffect_Rope(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect_Rope(const CEffect_Rope& Prototype);
	virtual ~CEffect_Rope() = default;

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

	_float						m_fCurrentTime = 0.f;
	_float						m_fMaskSpeed = 1.f;
	_float						m_fColorGamma = 1.2f;
	_float						m_fColorGain = 0.7f;

	_int						m_iShaderPass = 0;

	_float3						m_vPlayerPos = {};
	_float3						m_vRopeObjectPos = {};

	_int						m_SamleCount = 0;
	deque<SAMPLE_DESC>			m_Samples = {};

	const _float4x4*			m_pBoneMatrixPtr = nullptr;
	const _float4x4*			m_pPlayerMatrixPtr = nullptr;

	_bool*						m_pIsActive = nullptr;

	_bool						m_IsObectActive = false;

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CEffect_Rope* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END