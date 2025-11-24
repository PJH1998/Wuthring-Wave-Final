#pragma once
#include "Custom_UI.h"

NS_BEGIN(Client)

class CUI_Ovfl_Palette final : public CCustom_UI
{
public:


private:
	enum PALETTE_COLOR { PCOLOR_RED, PCOLOR_GREEN, PCOLOR_BLUE, PCOLOR_YELLOW, PCOLOR_END };

	typedef struct tUIPaletteDesc {

		array<_uint, 2>		arrIndex = {};
		PALETTE_COLOR		eColor = PCOLOR_END;

	} UI_PALETTE_DESC;

public:
	explicit CUI_Ovfl_Palette(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_Ovfl_Palette(const CUI_Ovfl_Palette& Prototype);
	virtual ~CUI_Ovfl_Palette() = default;

public:
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;

	HRESULT			Ready_Components(void* pArg);

private:
	void			PreAssign_Presets();
	void			PreAssign_ChildUIs();

private:
	void			Update_PalettesInfo();


private:
	// UI Caching..
	CCustom_UI*					m_pRUI_All = { nullptr };


private:
	array<array<UI_PALETTE_DESC, 10>, 8>	arrPalettesInfo = {};		

	vector<UI_PALETTE_DESC>					vecTargetPalettes = {};


private:
	_bool						m_IsGoinDisabled	= { };
	_float						m_fDisableTimer		= { };
	_uint						m_iAnimOrder		= { };

	class CGameSystem*			m_pGameSystem		= { nullptr };

public:
	static CUI_Ovfl_Palette*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};

NS_END