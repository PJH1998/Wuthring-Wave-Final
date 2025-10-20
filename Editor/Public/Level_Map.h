#pragma once
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Editor)

class CLevel_Map final : public CLevel
{
	enum Menu { MENU_OBJECT, MENU_RANDSCAPE, MENU_LIGHT, MENU_MAPSAVELOAD, MENU_OBJECTLOAD, END };
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
	void Menu_Model_Load();
	void Load_Objects();
	//?대씪?댁뼵?몄뿉 由ъ냼?ㅼ뿉 Map ?대뜑???덈뒗 .dat???쎌뼱???ㅻ툕?앺듃?ㅻ쭔) ?꾨Ⅴ硫??앹꽦?????덇쾶 ?섍린. ?앹꽦 ?꾩튂??萸?. ?뚯븘??

private:
	HRESULT Ready_Static_Component();
	void Ready_Event();
	void Make_MousePos();
	void Container_Info();

	void Load_Foliage();

public:
	static _float3 m_vWorldPos;
	static _float3 m_vWorldDir;
	static _float4 m_vPickedPos;
	
private:
	Menu m_eMenu = { END };
	_uint m_iLevel = ENUM_CLASS(LEVEL::MAP);
	class CEdit_MapObject* m_pChildObject = { nullptr };
	class CEdit_MapObject* m_pPickedObject = { nullptr };
	class CEdit_MapObject_Instance* m_pPickedInstanceObject = { nullptr };
	class CEdit_LightObject* m_pPickedLightObject = { nullptr };
	class CEdit_PreViewModel* m_pPreViewObject = { nullptr };

	class CEdit_Brush* m_pBrush = { nullptr };


	unordered_map<string, vector<CGameObject*>> m_SaveObjects;
	unordered_map<string, class CEdit_MapObject*> m_ContainerObjects;
	_float m_fNearDistance= { FLT_MAX };
	_float m_fNearDistance_Instance = {FLT_MAX};

	vector<_string> m_ModelPaths;
	vector<_string> m_FoliagePaths;
	_wstring m_szPreViewModelName;
	_bool m_LoadMenu = { false };
public:
	static		CLevel_Map*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END