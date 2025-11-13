#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CSFX;
class CVIBuffer_Rect;
class CShader;

class CSFX_Hub final : public CBase
{
private:
	typedef map<SFX_TYPE, CSFX*> SFX;

private:
	CSFX_Hub(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CSFX_Hub() = default;

public:
	HRESULT					Initialize(_uint iWinSizeX, _uint iWinSizeY);
	void					Update_SFX(_float fTimeDelta);

	HRESULT					Begin_SFX_Toggle(SFX_TOGGLE eType);
	HRESULT					Begin_SFX_Time(SFX_TOGGLE eType, _float fTime);
	HRESULT					End_SFX();

	HRESULT					Render_SFX_Toggle(CVIBuffer_Rect* pVIBuffer, CShader* pShader);
	HRESULT					Render_SFX(SFX_TYPE eType, CVIBuffer_Rect* pVIBuffer, CShader* pShader);

private:
	CGameInstance*			m_pGameInstance = { nullptr };
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	
	_uint					m_iWinSizeX = {};
	_uint					m_iWinSizeY = {};

	SFX						m_SFXs;


	_bool					m_IsToggleOff = { false };
	_float					m_fToggleIntensity = {};
	_float					m_fIntensityBoost = {};
	CSFX*					m_pCurrentSFX = { nullptr };

private:
	CSFX*					Find_SFX(SFX_TYPE eType);

	HRESULT					Ready_SFX();
	HRESULT					Ready_SFX_CS();
	void					Update_ToggleIntensity(_float fTimeDleta);

public:
	static CSFX_Hub*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual void			Free() override;
};

NS_END