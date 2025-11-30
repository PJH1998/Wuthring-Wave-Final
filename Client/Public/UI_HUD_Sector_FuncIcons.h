#pragma once
#include "Custom_UI.h"

NS_BEGIN(Client)

class CUI_HUD_Sector_FuncIcons final : public CCustom_UI
{
//public:
//	typedef struct tHUDFunctionUIIDesc {
//		//?
//	} UI_HUD_MINIMAP_DESC;

public:
	explicit CUI_HUD_Sector_FuncIcons(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_HUD_Sector_FuncIcons(const CUI_HUD_Sector_FuncIcons& Prototype);
	virtual ~CUI_HUD_Sector_FuncIcons() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

private:
	HRESULT			Ready_Components(void* pArg);

	void			PreAssign_ChildUIs();
	void			PreAssign_Presets();

private:
	// 매 프레임 돌릴만한 건 캐싱..
	CCustom_UI*			m_pRUI_All			= { nullptr };

	CCustom_UI*			m_pUI_SectorRT		= { nullptr };
	CCustom_UI*			m_pUI_RT_InstIcons	= { nullptr };

private:
	//class CGameSystem*	m_pGameSystem		= { nullptr };

public:
	static CUI_HUD_Sector_FuncIcons*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*				Clone(void* pArg) override;
	virtual void						Free() override;
};

NS_END