#pragma once
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Editor)

class CLevel_Map final : public CLevel
{
	enum Menu { MENU_OBJECT, MENU_RANDSCAPE, MENU_LIGHT, MENU_SAVELOAD, END };
private:
	explicit CLevel_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Map() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Render() override;

	void Menu_Select();
	void Menu_Object();
	void Menu_RandSacpe();
	void Menu_Light();
	void Menu_Save_Load();

private:
	HRESULT Ready_Static_Component();
	void Ready_Event();

private:
	Menu m_eMenu = { END };
	_uint m_iLevel = ENUM_CLASS(LEVEL::MAP);
	class CMapObject* m_pPickedObject = { nullptr };
	class CMapObject_Instance* m_pPickedInstanceObject = { nullptr };
	unordered_map<string, CGameObject*> m_SaveObjects;



	_bool m_LoadMenu = { false };
public:
	static		CLevel_Map*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END