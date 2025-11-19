#pragma once
#include "ScreenEffect.h"

NS_BEGIN(Engine)
class CTexture;
class CShader;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CAugusta_UltiPostSFX final : public CScreenEffect
{
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
	CVIBuffer_Rect*			m_pVIBuffer_Rect = { nullptr };
	CShader*				m_pShader = { nullptr };
	CTexture*				m_pNoiseTexture = { nullptr };
	ID3D11Buffer*			m_pBuffer = { nullptr };

	_float					m_fRadialLengthScale = {};
	SFX_RADIAL_DATA			m_RadialData = {};

private:
	HRESULT					Ready_Components();
	HRESULT					Ready_Buffer();

public:
	static CAugusta_UltiPostSFX*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg);
	virtual void			Free() override;
};

NS_END