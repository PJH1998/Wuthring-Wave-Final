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

private:
	class CGameInstance*	m_pGameInstance = { nullptr };

	//class CUI_HUD*			m_pRootUI_HUD = { nullptr };

public:
	static CUI_ControlHelper* Create();
	virtual void Free() override;
};

NS_END
