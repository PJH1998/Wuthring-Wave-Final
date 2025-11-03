#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CGameSystem final : public CBase
{
	DECLARE_SINGLETON(CGameSystem)
private:
	explicit CGameSystem();
	virtual ~CGameSystem() = default;

public:
#pragma region GameSystem
	void		Ready_GameSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
#pragma endregion

#pragma region Parser
	const vector<vector<_string>>& Load_CSV(const _char* pFilePath);
	void							Ready_Prototype_Map(const _char* pFilePath, LEVEL eLevel);
	void							Clone_MapObjects(LEVEL eLevel, _uint iIndex);
#pragma endregion

#pragma region Factory
	void							Create_MonsterDummy(LEVEL eLayerLevel, _float3 vPos, const _fmatrix& PreTransformationMatrix);
#pragma endregion

#pragma region CHARACTER INFO
	void Sync_CharacterInfo(const CHARACTER_STAT& eCharacterStat);
#pragma endregion

#pragma region [UI] FONT_PRESET
	void		Render_Damage(_float4 vTargetPos, _int iDamage, _uint iDmgElemType = 0, _uint iDmgAnimType = 0);
#pragma endregion

#pragma region [UI] CONTROL_HELPER
	class CCustom_UI*	Find_RootUI(_wstring strName);
	class CCustom_UI*	Find_ChildUI(_wstring strRootUIName, _wstring strChildUIName);

	HRESULT		HUD_FadeOut();
	HRESULT		HUD_FadeIn();
#pragma endregion


#pragma region [UI] STATUS_SYNCER
	HRESULT		Sync_Status_toHUD(CHARACTER_STAT& eStat);


#pragma endregion



private:
	class	CParser*			m_pParser						= { nullptr };
	class	CFactory*			m_pFactory						= { nullptr };

	class	CUI_FontPreset*		m_pUI_FontPreset				= { nullptr };
	class	CUI_ControlHelper*	m_pUI_ControlHelper				= { nullptr };
	class	CUI_StatusSyncer*	m_pUI_StatusSyncer				= { nullptr };


	CHARACTER_STAT m_Stats = {};
public:
	virtual		void	Free() override;

};

NS_END