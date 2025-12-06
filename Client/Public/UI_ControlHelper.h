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
	void				PreAssign_TargetUIs();

public:
	class CCustom_UI*	Find_RootUI(_wstring strName);
	class CCustom_UI*	Find_ChildUI(_wstring strRootUIName, _wstring strChildUIName);

	HRESULT				HUD_FadeOut(_bool isForceChange);
	HRESULT				HUD_FadeIn(_bool isForceChange);

	HRESULT				HUD_FadeOut_BossHPBar(_bool isForceChange);
	HRESULT				HUD_FadeIn_BossHPBar(_bool isForceChange);

	void				HUD_Bind_BossStatus(_wstring strUIBosssName, const _char* pMonsterKey, _float* pCurBossHP, _float* pCurBossSA, _bool* pIsGroggy, _float* pGroggyLeftRatio);
	void				HUD_Toggle_BossStatusUI(_bool isOn, _bool isForceChange);

	 
	//void				Toggle_InteractUI(_bool isOn, _wstring strText);

	void				Show_InteractUI(_wstring strText);
	void				Hide_InteractUI(_bool isPressedAs = false);
	void				Req_Render_InteractUI(_wstring xstrText, _bool isPressedAs);

	_bool				Get_InteractUI_Feedback(UI_EVENT_TYPE eEventInteractType);


	void				Attach_LockOnUI(_float3* pTargetPos);
	void				Detach_LockOnUI();

	void				Attach_Parry(_float3* pTargetPos);
	void				Enable_Parried();

	void				Update_MobStatus(const UI_MOBINFO_DESC& tDesc);

	void				Show_TabUtilityUI(_uint iCurSelectedUtilityIndex);
	_uint				HideNGet_TabUtilityUI();

	void				Open_Game_OverflowPalette(_uint iTargetLevel);
	void				Close_Game_OverflowPalette();

	void				Attach_GrafflePoint(_float3* pTargetPos);		// 외부 값 받아올거면, 그래플 UI 헤더, cpp에 최상단 define 해제 필요

	void				Play_QTE(_float2 vSpawnPos, UI_QTE_TYPE eQTEType, UI_QTE_BTN eIconIndex, _float2 vScale);

	void				Bind_ObjectPos_PerFrame_ToMinimap(const _float3& vPosition, UI_MINIMAP_OBJTYPE eType);
	void				Attach_ObjectPos_ToMinimap(const _float3& pPosition, UI_MINIMAP_OBJTYPE eType, void* pOwner);
	void				Detach_ObjectPos_ToMinimap(void* pOwner);

	void				Req_Render_CurveTrace(	_float3& vStartPos,
												_float3& vStartVelocity,
												_float3& vAcceleration,
												_float fMaxTime,
												_uint iSegmentCount,
												_float fRibbonWidth,
												_bool isUseCustomColor,
												_float4 vBaseColor,
												_float4 vHeadColor,
												_float4 vTailColor);

private:
	CCustom_UI*			m_pRootUI_HUD						= { nullptr };
	CCustom_UI*			m_pUI_HUD_SectorR_PartyFrame		= { nullptr };
	CCustom_UI*			m_pUI_HUD_SectorB_Status			= { nullptr };
	CCustom_UI*			m_pUI_HUD_SectorA					= { nullptr };
	CCustom_UI*			m_pUI_HUD_SectorRB_SkillIcons		= { nullptr };
	CCustom_UI*			m_pUI_HUD_SectorT_BossStatus		= { nullptr };


	CCustom_UI*			m_pRootUI_HUD_Minimap				= { nullptr };
	CCustom_UI*			m_pUI_UHD_SectorA_Minimap_All		= { nullptr };
	CCustom_UI*			m_pUI_UHD_SectorA_FuncIcons_All		= { nullptr };


	CCustom_UI*			m_pRootUI_Interact					= { nullptr };
	CCustom_UI*			m_pTextUI_Interact					= { nullptr };
	CCustom_UI*			m_pUI_Interact_Normal				= { nullptr };
	CCustom_UI*			m_pUI_Interact_Multiplier			= { nullptr };
	CCustom_UI*			m_pUI_Interact_Pressed				= { nullptr };
	CCustom_UI*			m_pUI_Interact_Focused				= { nullptr };
	CCustom_UI*			m_pRootUI_LockOn					= { nullptr };
	CCustom_UI*			m_pRootUI_Parry						= { nullptr };
	CCustom_UI*			m_pRootUI_MobHPBar					= { nullptr };
	CCustom_UI*			m_pRootUI_TabUtility				= { nullptr };
	CCustom_UI*			m_pRootUI_GrafflePoint				= { nullptr };
	//CCustom_UI*			m_pRootUI_QTE						= { nullptr };

	CCustom_UI*			m_pRootUI_Ovfl_Palette				= { nullptr };

	CCustom_UI*			m_pRootUI_CurveTrace				= { nullptr };

private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	class CGameSystem*		m_pGameSystem	= { nullptr };

	vector<CCustom_UI*>		m_vecInteractions = {};

public:
	static CUI_ControlHelper* Create();
	virtual void Free() override;
};

NS_END
