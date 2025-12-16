#pragma once
#include "ScreenEffect.h"

NS_BEGIN(Engine)
class CTexture;
class CVIBuffer_Rect;
class CShader;
NS_END


NS_BEGIN(Client)

class CExcute_PostSFX : public CScreenEffect
{
private:
	CExcute_PostSFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CExcute_PostSFX(const CExcute_PostSFX& Prototype);
	virtual ~CExcute_PostSFX() = default;

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
	ID3D11Buffer*			m_pBuffer = { nullptr };

	SFX_SLASH_DATA			m_SlashData = {};
	_uint					m_iPrevLutIndex = {};
	_float					m_fPrevLutIntensity = {};
	_bool					m_PrevIsDynamicLUT = { };

	_uint					m_iLUT_Index = {};

	_float					m_fIntensityTIme = {};
	_float					m_fCurrentLUTIntensity = {};
	

private:
	HRESULT					Ready_Components();
	HRESULT					Ready_Buffer();

public:
	static CExcute_PostSFX* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END