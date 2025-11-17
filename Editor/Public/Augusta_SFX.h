#pragma once
#include "Edit_ScreenEffect.h"

NS_BEGIN(Engine)
class CTexture;
NS_END

NS_BEGIN(Editor)

class CAugusta_SFX final : public CEdit_ScreenEffect
{
private:
	CAugusta_SFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAugusta_SFX(const CAugusta_SFX& Prototype);
	virtual ~CAugusta_SFX() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override;
	virtual		void		Render() override;

	virtual		void		Play() override;
	virtual		void		Stop() override;
	virtual		void		Reset() override;

private:
	CTexture*				m_pBaseTexture = { nullptr };	//Tile_262
	CTexture*				m_pMaskTexture = { nullptr };	//Mask_300156
	CTexture*				m_pNoiseTexture = { nullptr };	//Tile_30010
	CTexture*				m_pSecondTexture = { nullptr };	//Tile_530002

	_bool	test = {};
private:
	HRESULT					Ready_Textures();


public:
	static CAugusta_SFX*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END