#pragma once

#include "ScreenEffect.h"

NS_BEGIN(Engine)
class CVIBuffer_Rect;
class CShader;
NS_END

NS_BEGIN(Client)

class CSonoraChange final : public CScreenEffect
{
public:
	typedef struct tagSonoraChagne {
		_float fEffectTime = {};
		_float fRadialTime = {};
		_float fFadeTime = {};
	}SONORA_CHANGE_DESC;

private:
	CSonoraChange(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSonoraChange(const CSonoraChange& Prototype);
	virtual ~CSonoraChange() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override;
	virtual		void		Render() override;
	virtual		void		Reset(const _fmatrix& WorldMatrix, void* pArg) override;
	
private:
	_float					m_fCurrentTime = {};
	_float					m_fEffectTime = {};
	_float					m_fRadialTime = {};
	_float					m_fFadeTime = {};
	_float					m_fFadeIntensity = {};
	_float2					m_vRadialCenter = {};

	_float2					m_vRadialDistanceRange = {};
	_float2					m_vRadialIntensityRange = {};
	
	_float3					m_vFadeColor = {};

private:
	HRESULT					Bind_ShaderResources();

public:
	static CSonoraChange*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END