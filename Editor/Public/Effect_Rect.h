#pragma once
#include "Editor_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Point;
NS_END

NS_BEGIN(Editor)

class CEffect_Rect final : public CGameObject
{
public:
	typedef struct tagEffectRectDesc : Engine::EFFECT_DESC
	{
		_wstring strTextureTag;
		//_wstring strVIBufferTag;

		_int	fShaderPass = 0;
		_float3	vSize = { 1.f, 1.f, 1.f };
		_float3 vPos = { 0.f, 0.f, 0.f };
		_float4 vColor = { 0.f, 0.f, 0.f, 0.f };
		_float2	vLifeTime = { 0.f, 10.f};

	}FXRECT_DESC;

private:
	CEffect_Rect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect_Rect(const CEffect_Rect& Prototype);
	virtual ~CEffect_Rect() = default;

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
	void Root_Transform(_fmatrix WorldMatrix);
	void Bind_CS_SpriteInfo();
	

private:
	CShader*					m_pShaderCom = { nullptr };
	CTexture*					m_pTextureCom = { nullptr };
	CVIBuffer_Point*	m_pVIBufferCom = { nullptr };

	//FXRECT_DESC					m_tDesc = {};

	_int						m_iShaderPass = 0;
	_float3						m_vPos = {};
	_float4						m_vColor = {};
	_float2						m_vLifeTime = {};

private:
	HRESULT Ready_Components(FXRECT_DESC& Desc);
	HRESULT Bind_ShaderResources();

public:
	static CEffect_Rect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END