#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CGameSystem final : public CBase
{
	DECLARE_SINGLETON(CGameSystem)
	using TriggerCallback = function<void(void*)>;
private:
	explicit CGameSystem();
	virtual ~CGameSystem() = default;

public:
#pragma region GAMESYSTEM
	void		Ready_GameSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
#pragma endregion

#pragma region PARSER
	const vector<vector<_string>>& Load_CSV(const _char* pFilePath);
	
	//============================Effect
	void							Create_Map_Model(const _char* pFilePath, LEVEL eLevel);
	void							Create_Effect(const string& strFolderPath, LEVEL eLevel); 
	void							Create_Prefab(const string& strFolderPath, LEVEL eLevel);
	void							Load_EffectTexture_FromFolder(const string& strFolderPath, LEVEL eLevel);
	void							Load_EffectMeshDat_FromFolder(const string& strFolderPath, LEVEL eLevel);
	//============================Effect

	void							Ready_Prototype_Map(const _char* pFilePath, LEVEL eLevel);
	void							Clone_MapObjects(LEVEL eLevel, _uint iIndex);
#pragma endregion

#pragma region FACTORY
	void							Create_MonsterDummy(LEVEL eLayerLevel, _float3 vPos, const _fmatrix& PreTransformationMatrix);
#pragma endregion

#pragma region DIRECTOR
	void							Add_Action(const _char* pFolderPath);
	void							Play_Action(const _wstring& strActionTag, const _fmatrix& WorldMatrix, _bool isMaintain);
	void							Stop_Action();
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
	//HRESULT		Sync_Status_toHUD(CHARACTER_STAT& eStat);


#pragma endregion

#pragma region PLAYER STATUS
	class CPlayerStatus* Get_PlayerStatus() const { return m_pPlayerStatus; }
#pragma endregion


#pragma region TRIGGER
	void TriggerRegister(_uint iNumTriggerMapIndex, TriggerCallback pFunc);
	void OnTriggerActivate(_uint iNumTriggerMapIndex, void* pArg = nullptr);
	//레벨 전환시 초기화 고려.
	void Clear_TriggerCallBack();
#pragma endregion



private:
	class	CParser*			m_pParser						= { nullptr };
	class	CFactory*			m_pFactory						= { nullptr };

	class	CUI_FontPreset*		m_pUI_FontPreset				= { nullptr };
	class	CUI_ControlHelper*	m_pUI_ControlHelper				= { nullptr };
	class	CUI_StatusSyncer*	m_pUI_StatusSyncer				= { nullptr };

	class	CDirector*			m_pDirector 					= { nullptr };
	class	CPlayerStatus* 		m_pPlayerStatus 				= { nullptr };


	CHARACTER_STAT m_Stats = {};
	unordered_map<_uint, vector<TriggerCallback>> m_TriggerEvents;
public:
	virtual		void	Free() override;

};

NS_END