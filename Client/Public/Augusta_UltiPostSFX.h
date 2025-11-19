#pragma once
#include "ScreenEffect.h"

NS_BEGIN(Engine)
class CTexture;
NS_END

NS_BEGIN(Client)

class CAugusta_UltiPostSFX final : public CScreenEffect
{
private:
	typedef struct tagSlashData
	{
		_float2 vSlashPoint0;
		_float2 vSlashPoint1;
		_float fOffset;
		_float fIntensity;
		_float2 Padding;
	}SLASH_DATA;

	typedef struct tagRadialData
	{
		_float fMinDistance;
		_float fMaxDistance;
		_float fLengthScale;
		_float fPadding0;
		_float2 vPivot;
		_float2 fPadding1;
	}RADIAL_DATA;

private:
	CAugusta_UltiPostSFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAugusta_UltiPostSFX(const CAugusta_UltiPostSFX& Prototype);
	virtual ~CAugusta_UltiPostSFX() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override;
	virtual		void		Render() override;
	virtual		void		Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	SLASH_DATA				m_SlashData = {};
	
	_float					m_fRadialLengthScale = {};
	RADIAL_DATA				m_RadialData = {};

	_float2					m_vEffectTime = {};
	_float					m_fCurrentTime = {};

	CTexture*				m_pNoiseTexture = { nullptr };

	ID3D11Buffer*			m_pBuffer = { nullptr };

private:
	HRESULT					Ready_Texture();
	HRESULT					Ready_Buffer();

public:
	static CAugusta_UltiPostSFX*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg);
	virtual void			Free() override;
};

NS_END