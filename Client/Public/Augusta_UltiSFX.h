#pragma once
#include "ScreenEffect.h"

NS_BEGIN(Engine)
class CTexture;
class CShader;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CAugusta_UltiSFX final : public CScreenEffect
{
private:
	CAugusta_UltiSFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAugusta_UltiSFX(const CAugusta_UltiSFX& Prototype);
	virtual ~CAugusta_UltiSFX() = default;

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
	CTexture*				m_pMaskTexture = { nullptr };	//Mask_300156

	_float3					m_vColor = { _float3(1.f, 1.f, 1.f) };
	
	_float2					m_vLutTime = {};

	_float2					m_vPos = {};
	_float2					m_vScale = {};

	_uint					m_iSFXLutIndex = {};
	_float2					m_vSFXLutIntensity = {};
	_float					m_fCurrentLutIntensity = {};

	_uint					m_iPrevLutIndex = {};
	_float					m_fPrevLutIntensity = {};
	_bool					m_PrevIsDynamicLUT = { };


private:
	HRESULT					Ready_Components();
	

public:
	static CAugusta_UltiSFX*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};

NS_END