#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CUI_ControlHelper final : public CBase
{
private:
	explicit CUI_ControlHelper();
	virtual ~CUI_ControlHelper() = default;

public:
	HRESULT				Initialize();

public:
	class CCustom_UI*	Find_RootUI(_wstring strName);
	class CCustom_UI*	Find_ChildUI(_wstring strRootUIName, _wstring strChildUIName);

	HRESULT				HUD_FadeOut();
	HRESULT				HUD_FadeIn();

	void				Render_InteractUI(_wstring strText);
	_bool				Get_InteractUI_Feedback(UI_EVENT_TYPE eEventInteractType);

private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	class CGameSystem*		m_pGameSystem	= { nullptr };

	vector<CCustom_UI*>		m_vecInteractions = {};

public:
	static CUI_ControlHelper* Create();
	virtual void Free() override;
};

NS_END
