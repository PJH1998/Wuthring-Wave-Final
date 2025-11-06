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

private:
	HRESULT					Load_ChildObjects(_wstring strFilePath);
	HRESULT					Load_Animations(vector<_wstring> vecAnimFilePath);

private:
	HRESULT					Ready_Components(void* pArg);
	HRESULT					Ready_Presets();

private:					// �ڽ� UI�� ���� ��� ������ �ش� �����̳� UI�� ����.
	void					Update_UI_SkillSection(_float fTimeDelta);
	void					Update_UI_SkillSection_BG(_float fTimeDelta);
	void					Update_UI_SkillFeedback_Trigger(_float fTimeDelta);
	void					Update_UI_PlayerHPBar(_float fTimeDelta);
	void					Update_UI_BossHPBar(_float fTimeDelta);
	void					Update_UI_KeyGuide(_float fTimeDelta);

	void					Update_UI_PlayerEnergyFrame(_float fTimeDelta);				// [Energy] Only Frame
	void					Update_UI_PlayerEnergyBar(_float fTimeDelta);				// [Energy] Normal Energy. shared.
	void					Update_UI_PlayerEnergyBar_Augusta(_float fTimeDelta);		// [Energy] about Augusta's unique resources
	void					Update_UI_PlayerEnergyBar_Galbrena(_float fTimeDelta);		// [Energy] about Galbrena's unique resources

private:
	void					Update_UI_SkillSection_OnFeedback(_float fTimeDelta);		/* manunally calls on update.. onfeedback*/
	void					Update_AugustaIcon(const vector<UISKILL_SLOT>& skillSlots);
	
	void					Add_UI_SkillSection_OnFeedback(_uint iSectionIndex);

private:
	array<_float2, 2>		Calc_SpriteSpace(_uint iIndexX, _uint iIndexY, array<_uint, 2> iNumMax, _float2 vSpriteSize = {1.f, 1.f});

private:
	class CGameSystem*		m_pGameSystem = { nullptr };

	class CPlayerStatus*	m_pPlayerStatus = { nullptr };
	class CAbility*			m_pAbility = { nullptr };
	

	// * Temp assumed value.
	//		| ROVER		|  AUGUSTA			| GARBENA
	// ------------------------------------------------------------
	//	0	| Normal	| Normal			| Normal
	//	1	| DarkSerge	| Normal - Combo 1	| Normal - Burst Ready
	//	2	| 			| Normal - Combo 2	| Burst
	//	3	| 			| Normal - Combo 3	|
	//	4	| 			| Ult				|
	_uint					m_iPlayerEnhancedMode = 0;

	// Update_UI_SkillSection
	unordered_map<_wstring, array<_float2, 2>>		m_mapSkillTexIndices = {};


public:
	static CUI_HUD* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void			Free() override;
};

NS_END