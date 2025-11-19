#pragma once
#include "ScreenEffect.h"

NS_BEGIN(Engine)
class CVIBuffer_Rect;
class CShader;
class CTexture;
NS_END

NS_BEGIN(Client)

class CGalbrenaUlti_SFX_Circle final : public CScreenEffect
{
private:
	CGalbrenaUlti_SFX_Circle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGalbrenaUlti_SFX_Circle(const CGalbrenaUlti_SFX_Circle& Prototype);
	virtual ~CGalbrenaUlti_SFX_Circle() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) override;


private:
	CVIBuffer_Rect*				m_pVIBuffer = { nullptr };
	CShader*					m_pShader = { nullptr };
	
	_float2						m_vCenter = {};

	_float2						m_vUpdateTime = {};

	_float2						m_vSize = {};
	_float2						m_vCurrentSize = {};

	_float2						m_vScale = {};
	_float						m_fCurrentScale = {};

	_float						m_fCurrentRadius = {};
	_float						m_fCircleRadius = {};
	_float						m_fCircleWidth = {};

	_float3						m_vColor = {};

private:
	HRESULT						Ready_Components();
	

public:
	static CGalbrenaUlti_SFX_Circle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void					Free() override;
};

NS_END