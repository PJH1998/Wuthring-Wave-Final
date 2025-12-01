#pragma once

#include "Custom_UI.h"

NS_BEGIN(Engine)
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CUI_HUD final : public CCustom_UI
{
private:
	enum HUD_CHAR_INDEX			{ CH_ROVER, CH_AUGUSTA, CH_GALBRENA, CH_END };
	enum HUD_SKILL_INDEX		{ SK_E, SK_R, SK_END };
	enum HUD_PLAYER_HPBAR		{ PLHP_BACK, PLHP_NORMAL, PLHP_END };
	enum HUD_BOSS_HPBAR			{ BOHP_BACK, BOHP_NORMAL, BOHP_END };
	enum HUD_BOSS_SABAR			{ BOSA_BACK, BOSA_NORMAL, BOSA_END };
	enum HUD_BTN_INDEX			{ BTN_E, BTN_R, BTN_LB /* 아우구스타, 갈브만 존재 */, BTN_END };

private:
	explicit				CUI_HUD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit				CUI_HUD(const CUI_HUD& Prototype);
	virtual					~CUI_HUD() = default;

public:
	virtual HRESULT			Initialize_Prototype()					override;
	virtual HRESULT			Initialize_Clone(void* pArg)			override;
	virtual void			Priority_Update(_float fTimeDelta)		override;
	virtual void			Update(_float fTimeDelta)				override;
	virtual void			Late_Update(_float fTimeDelta)			override;
	virtual void			Render()								override;

public:
	void					PreAssign_ChildUIs();

	void					Bind_BossStatus(_wstring strUIBosssName, const _char* pMonsterKey, _float* pCurBossHP, _float* pCurBossSA, _bool* pIsGroggy, _float* pGroggyLeftRatio);
	void					Toggle_BossStatusUI(_bool isOn) { m_isOn_BossStatus = isOn; }


private:
	HRESULT					Ready_Components(void* pArg);
	HRESULT					Ready_ChildExtraComponents();
	HRESULT					Ready_Presets();

	HRESULT					Ready_BossUINameText();
	HRESULT					Ready_PlayerHPText();

private:					// �ڽ� UI�� ���� ��� ������ �ش� �����̳� UI�� ����.
	void					Update_UI_SkillSection(_float fTimeDelta);
	void					Update_UI_SkillSection_Wave(_float fTimeDelta);
	void					Update_UI_SkillSection_Utility(_float fTimeDelta);
	void					Update_UI_SkillSection_BG(_float fTimeDelta);
	void					Update_UI_SkillFeedback_Trigger(_float fTimeDelta);
	void					Update_UI_PlayerHPBar(_float fTimeDelta);
	void					Update_UI_BossHPBar(_float fTimeDelta);
	void					Update_UI_KeyGuide(_float fTimeDelta);

	void					Update_UI_PlayerEnergyFrame(_float fTimeDelta);				// [Energy] Only Frame
	void					Update_UI_PlayerEnergyBar(_float fTimeDelta);				// [Energy] Normal Energy. shared.
	void					Update_UI_PlayerEnergyBar_Augusta(_float fTimeDelta);		// [Energy] about Augusta's unique resourcesl
	void					Update_UI_PlayerEnergyBar_Galbrena(_float fTimeDelta);		// [Energy] about Galbrena's unique resources

private:
	void					Update_UI_SkillSection_OnFeedback(_float fTimeDelta);		/* manunally calls on update.. onfeedback*/

	void					Update_Icon_Rover(const vector<UISKILL_SLOT>& skillSlots);
	void					Update_Icon_Augusta(const vector<UISKILL_SLOT>& skillSlots);
	void					Update_Icon_Galbrena(const vector<UISKILL_SLOT>& skillSlots);
	
	void					Add_UI_SkillSection_OnFeedback(_uint iSectionIndex);

private:
	void					Update_Text_PlayerHP();

private:
	array<_float2, 2>		Calc_SpriteSpace(_uint iIndexX, _uint iIndexY, array<_uint, 2> iNumMax, _float2 vSpriteSize = {1.f, 1.f});

private:
	CCustom_UI* m_pUI_SectorT_BossStatus					= nullptr;
	CCustom_UI* m_pUI_SectorB_Status						= nullptr;
	CCustom_UI* m_pUI_Skill[3]								= { };
	CCustom_UI* m_pUI_Change[3]								= { };

	CCustom_UI* m_pUI_Skill_ReadyFrame						= nullptr;
	CCustom_UI* m_pUI_Skill_ReadyWave						= nullptr;
	CCustom_UI* m_pUI_Skill_BG								= nullptr;
	CCustom_UI* m_pUI_SectorRB_SkillIcons					= nullptr;
	CCustom_UI* m_pUI_Skill_Utility							= nullptr;
	CCustom_UI* m_pUI_Feedback								= nullptr;
	CCustom_UI* m_pUI_HPBar									= nullptr;
	CCustom_UI* m_pUI_BossHPBar								= nullptr;
	CCustom_UI* m_pUI_BossSABar								= nullptr;
	CCustom_UI* m_pUI_KeyButton								= nullptr;

	CCustom_UI* m_pUI_Group_Rover							= nullptr;
	CCustom_UI* m_pUI_Group_Augusta							= nullptr;
	CCustom_UI* m_pUI_Group_Galbrena						= nullptr;

	CCustom_UI* m_pUI_Frame_Rover_Dark						= nullptr;
	CCustom_UI* m_pUI_Frame_Augusta							= nullptr;
	CCustom_UI* m_pUI_FrameGroup_Augusta_OtherEnergy		= nullptr;
	CCustom_UI* m_pUI_FrameGroup_Augusta_UltMode			= nullptr;
	CCustom_UI* m_pUI_Frame_Galbrena						= nullptr;
	CCustom_UI* m_pUI_Frame_Galbrena_Icon					= nullptr;
	CCustom_UI* m_pUI_FrameGroup_Galbrena_RageMode			= nullptr;

	CCustom_UI* m_pUI_Icon_ElementDark						= nullptr;
	CCustom_UI* m_pUI_Icon_ElementThunder					= nullptr;
	CCustom_UI* m_pUI_Icon_ElementFire						= nullptr;
	CCustom_UI* m_pUI_Icon_ElementGuage						= nullptr;

	CCustom_UI* m_pUI_EnergyInstItems						= nullptr;

	CCustom_UI* m_pUI_Frame_Augusta_Inst_SwordEnergy		= nullptr;
	CCustom_UI* m_pUI_Frame_Augusta_Inst_CenterPointEnergy	= nullptr;
	CCustom_UI* m_pUI_Frame_Augusta_Inst_UltModeEnergy		= nullptr;

	CCustom_UI* m_pTextUI_PlayerHP							= nullptr;
	CCustom_UI* m_pTextUI_BossName							= nullptr;


private:
	class CGameSystem*		m_pGameSystem			= { nullptr };

	class CPlayerStatus*	m_pPlayerStatus			= { nullptr };
	class CAbility*			m_pAbility				= { nullptr };
	
	// Update_UI_SkillSection
	unordered_map<_wstring, array<_float2, 2>>		m_mapSkillTexIndices = {};
	array<array<_float2, 2>, 5>						m_arrUtilCoordPresets = {};
	_uint m_iSelectedCHIndex = 0;
	_float m_fElapsedTime = 0.f;

private:
	_uint					m_iUtilityIndex_Tmp = ENUM_CLASS(UI_TAB_UTILITY::NOTHING);


private:

	// ========== for Boss ==========
	_bool					m_isOn_BossStatus = false;

	_float*					m_pCurBossHP = { nullptr };
	_float					m_fBackBossHP = 0.f;

	_float*					m_pCurBossSA = { nullptr };
	_float					m_fBackBossSA = 0.f;
	;
	_float*					m_pGroggyLeftRatio = { nullptr };
	_bool*					m_pIsGroggy = { nullptr };

	_string					m_strMonsterKey = {};
	// ==============================


public:
	static CUI_HUD* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void			Free() override;
};

NS_END