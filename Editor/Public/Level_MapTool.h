#pragma once
#include "Level.h"

NS_BEGIN(Editor)
class CLevel_MapTool final : public CLevel
{
	enum Menu { MENU_OBJECT, MENU_RANDSCAPE, MENU_LIGHT, END };
private:
	explicit CLevel_MapTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_MapTool() = default;

public:
	virtual HRESULT	Initialize() override;
	virtual void			Update(_float fTimeDelta) override;
	virtual HRESULT	Render() override;

	void Menu_Select();
	void Menu_Object();
	void Menu_RandSacpe();
	void Menu_Light();
private:
	HRESULT Ready_Static_Component();

private:
	Menu m_eMenu = { END };
public:
	static CLevel_MapTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free()override;
};

NS_END