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

	HRESULT				HUD_FadeOut_BossHPBar();
	HRESULT				HUD_FadeIn_BossHPBar();

	void				HUD_Bind_BossStatus(_wstring strUIBosssName, const _char* pMonsterKey, _float* pCurBossHP, _float* pCurBossSA, _bool* pIsGroggy, _float* pGroggyLeftRatio);
	void				HUD_Toggle_BossStatusUI(_bool isOn);


	//void				Toggle_InteractUI(_bool isOn, _wstring strText);

	void				Show_InteractUI(_wstring strText);
	void				Hide_InteractUI(_bool isPressedAs = false);

	_bool				Get_InteractUI_Feedback(UI_EVENT_TYPE eEventInteractType);


	void				Attach_LockOnUI(CTransform* pTargetTransform);
	void				Detach_LockOnUI();


private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	class CGameSystem*		m_pGameSystem	= { nullptr };

	vector<CCustom_UI*>		m_vecInteractions = {};

public:
	static CUI_ControlHelper* Create();
	virtual void Free() override;
};

NS_END
