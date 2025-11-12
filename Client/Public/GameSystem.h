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
	void							Load_EffectDecalData_FromFolder(const string& strFolderPath);
	//============================Effect

	void							Ready_Prototype_Map(const _char* pFilePath, LEVEL eLevel);
	void							Clone_MapObjects(LEVEL eLevel);
	void							Clone_Spawners(LEVEL eLevel);
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

#pragma region ㅣ[UI] FONT_PRESET
	// 데미지를 생성합니다. (타겟의 위치벡터, 데미지 수치, 색상용 데미지 타입, 생성 랜덤 범위)
	void			Render_Damage(_float4 vTargetPos, _int iDamage, TEXT_COLOR_TYPE eColorType = TEXT_COLOR_TYPE::NONE, _float fSpawnRange = 10.f);
	// 데미지를 생성합니다. (타겟의 위치벡터, 출력할 텍스트, 색상용 데미지 타입, 생성 랜덤 범위)
	void			Render_Damage(_float4 vTargetPos, _wstring strText, TEXT_COLOR_TYPE eColorType = TEXT_COLOR_TYPE::NONE, _float fSpawnRange = 10.f);
	// 텍스트를 생성합니다.
	class CUI_Text*	Create_FontToScreen(_float2 vScreenPos, _wstring strText, TEXT_COLOR_TYPE eColorType, _float fFontScale, _wstring strUIName,_wstring strFontTag = L"WW_Bold");
#pragma endregion

#pragma region [UI] CONTROL_HELPER
	class CCustom_UI*	Find_RootUI(_wstring strName);
	class CCustom_UI*	Find_ChildUI(_wstring strRootUIName, _wstring strChildUIName);

	HRESULT		HUD_FadeOut();
	HRESULT		HUD_FadeIn();

	void		Render_InteractUI(_wstring strText);

	
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

#pragma region SONORO_MANAGER
	_bool* Add_To_Management(OBJECTTYPE eType, class CMapObject_Sonoro* pObjects, _bool** SonoroMode);
	_bool* Add_To_Management(OBJECTTYPE eType, class CMapObject_NonSonoro* pObjects, _bool** SonoroMode);
	void Update(_float fTimeDelta);
	void Change_Sonoro(_bool IsSonoro);
	_bool IsSonoro();

#pragma endregion

#pragma region MONSTER_TABLE
	HRESULT LoadMonsterTable(const _char* pFilePath);
	MONSTER_INFO* Get_MonsterInfo(const _char* pMonsterKey) const;
#pragma endregion

private:
	class	CParser*			m_pParser						= { nullptr };
	class	CFactory*			m_pFactory						= { nullptr };

	class	CUI_FontPreset*		m_pUI_FontPreset				= { nullptr };
	class	CUI_ControlHelper*	m_pUI_ControlHelper				= { nullptr };
	class	CUI_StatusSyncer*	m_pUI_StatusSyncer				= { nullptr };

	class	CDirector*			m_pDirector 					= { nullptr };
	class	CPlayerStatus* 		m_pPlayerStatus 				= { nullptr };
	
	class	CSonoro_Manager*	m_pSonoro_Manager				= { nullptr };

	class	CMonsterTable*		m_pMonsterTable					= { nullptr };

	CHARACTER_STAT m_Stats = {};
	unordered_map<_uint, vector<TriggerCallback>> m_TriggerEvents;
public:
	virtual		void	Free() override;

};

NS_END