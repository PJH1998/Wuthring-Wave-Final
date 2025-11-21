#pragma once
#include "ScreenEffect.h"

NS_BEGIN(Engine)
class CTexture;
class CVIBuffer_Rect;
class CShader;
NS_END

NS_BEGIN(Client)

class CGalbrenaUlti_PostSFX final : public CScreenEffect
{
private:
	CGalbrenaUlti_PostSFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGalbrenaUlti_PostSFX(const CGalbrenaUlti_PostSFX& Prototype);
	virtual ~CGalbrenaUlti_PostSFX() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override;
	virtual		void		Render() override;
	virtual		void		Reset(const _fmatrix& WorldMatrix, void* pArg) override;


private:
	CVIBuffer_Rect*			m_pVIBuffer_Rect = { nullptr };
	CShader*				m_pShader = { nullptr };
	CTexture*				m_pNoiseTexture = { nullptr };
	ID3D11Buffer*			m_pBuffer = { nullptr };

	SFX_RADIAL_DATA			m_RadialData = {};

	_float2					m_vLengthScale = {};
	_float2					m_vMaxDistance = {};

	_float2					m_vRadialTime = {};

	_float2					m_fFadeTime = {};

	_float2					m_vReverseTime[2] = {};
	_bool					m_IsReverse = {};

	_uint					m_iLUT_Index = {};
	_float					m_fCurrentLutIntensity;
	_float2					m_vLutIntensity = {};

	_uint					m_iPrevLutIndex = {};
	_float					m_fPrevLutIntensity = {};
	_bool					m_PrevIsDynamicLUT = { };

private:
	HRESULT					Ready_Components();

public:
	static CGalbrenaUlti_PostSFX*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*			Clone(void* pArg) override;
	virtual void					Free() override;
};

NS_END