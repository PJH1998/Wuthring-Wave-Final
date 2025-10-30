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

private:					// 자식 UI에 관한 모든 동작은 해당 컨테이너 UI가 전담.
	void					Update_UI_SkillSection(_float fTimeDelta);
	void					Update_UI_PlayerHPBar(_float fTimeDelta);
	void					Update_UI_BossHPBar(_float fTimeDelta);
	void					Update_UI_KeyGuide(_float fTimeDelta);

	void					Update_UI_PlayerEnergyFrame(_float fTimeDelta);				// [Energy] Only Frame
	void					Update_UI_PlayerEnergyBar(_float fTimeDelta);				// [Energy] Normal Energy. shared.
	void					Update_UI_PlayerEnergyBar_Augusta(_float fTimeDelta);		// [Energy] about Augusta's unique resources
	void					Update_UI_PlayerEnergyBar_Galbrena(_float fTimeDelta);		// [Energy] about Galbrena's unique resources
	

private:
	_uint					m_iSelectedCHIndex = 0;
	_float					m_fPlayerEnergy[CH_END] = { 0.f, 0.f, 0.f };

	const   _float			m_fPlayerMaxEnergy[CH_END] = { 100.f, 100.f, 100.f };


	_uint					m_iEnergyBarMode = 0;		// 0 : normal, 1 : ult or  mode change

public:
	static CUI_HUD* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void			Free() override;
};

NS_END