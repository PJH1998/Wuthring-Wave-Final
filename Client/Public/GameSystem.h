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
	void		Clear_Resource();
#pragma endregion

#pragma region PARSER
	const vector<vector<_string>>& Load_CSV(const _char* pFilePath);
	void							Load_Sequence(const _char* pFolderPath);
	
	//============================Effect
	void							Create_Map_Model(const _char* pFilePath, LEVEL eLevel);
	void							Create_Effect(const string& strFolderPath, LEVEL eLevel); 
	void							Create_Prefab(const string& strFolderPath, LEVEL eLevel, _int PoolingNum);
	void							Load_EffectTexture_FromFolder(const string& strFolderPath, LEVEL eLevel);
	void							Load_EffectMeshDat_FromFolder(const string& strFolderPath, LEVEL eLevel);
	void							Load_EffectDecalData_FromFolder(const string& strFolderPath);
	//============================Effect

	void							Ready_Prototype_Map(const _char* pDataFilePath, LEVEL eLevel, const _char* pModelFilePath);
	void							Clone_MapObjects(LEVEL eLevel);
	void							Clone_Spawners(LEVEL eLevel);
	void							Create_MapEffects();
#pragma endregion

#pragma region FACTORY
	void							Create_MonsterDummy(LEVEL eLayerLevel, _float3 vPos, const _fmatrix& PreTransformationMatrix);
#pragma endregion

#pragma region DIRECTOR
	void							Add_Action(const _char* pFolderPath);
	void							Play_Action(const _wstring& strActionTag, const _fmatrix& WorldMatrix, _bool isMaintain, _bool isEscape = false);
	void							Stop_Action();
#pragma endregion


#pragma region CHARACTER INFO
#pragma endregion

#pragma region [UI] FONT_PRESET
	// 데미지를 생성합니다. (타겟의 위치벡터, 데미지 수치, 색상용 데미지 타입, 생성 랜덤 범위)
	void			Render_Damage(_float4 vTargetPos, _int iDamage, TEXT_COLOR_TYPE eColorType = TEXT_COLOR_TYPE::NONE, _float fSpawnRange = 10.f);
	// 데미지를 생성합니다. (타겟의 위치벡터, 출력할 텍스트, 색상용 데미지 타입, 생성 랜덤 범위)
	void			Render_Damage(_float4 vTargetPos, _wstring strText, TEXT_COLOR_TYPE eColorType = TEXT_COLOR_TYPE::NONE, _float fSpawnRange = 10.f);
	// 텍스트를 생성합니다. (화면상에 스크린좌표로 글자를 띄우는 UI를 생성합니다. 알파 미적용.)
	class CUI_Text*	Create_FontToScreen(_float2 vScreenPos, _wstring strText, TEXT_COLOR_TYPE eColorType, _float fFontScale, _wstring strUIName,_wstring strFontTag = L"WW_Bold");
	// 텍스트를 생성합니다. (화면상에 스크린좌표로 글자를 띄우는 UI를 생성합니다. Instance Desc 수정을 통한 알파 적용.)
	class CUI_Text*	Create_FontToScreen_Alpha(_float2 vScreenPos, _wstring strText, TEXT_COLOR_TYPE eColorType, _float fFontScale, _wstring strUIName,_wstring strFontTag = L"WW_Bold");
#pragma endregion

#pragma region [UI] CONTROL_HELPER
	// UI 꺼내쓰기용. 혹 수정 필요시 말해주세요.

	void		PreAssign_TargetUIs();	// Initialize for UI caching. call after ui load.

	// 기존 GameInstance 에서는 번거롭게 캐스팅 필요하던 걸, 편하게 가져오도록 캐스팅 내장시켜서 재정의.
	class CCustom_UI*	Find_RootUI(_wstring strName);
	class CCustom_UI*	Find_ChildUI(_wstring strRootUIName, _wstring strChildUIName);
	
	HRESULT		HUD_FadeOut(_bool isForceChange = false);	// 보스 체력바를 제외한 HUD를 FadeOut 합니다.
	HRESULT		HUD_FadeIn(_bool isForceChange = false);	// 보스 체력바를 제외한 HUD를 FadeIn  합니다.

	//HRESULT		HUD_FadeOut_BossHPBar();	// 보스 체력바 UI를 FadeOut 합니다.	// HUD_Toggle_BossStatusUI 에 통합.
	//HRESULT		HUD_FadeIn_BossHPBar();		// 보스 체력바 UI를 FadeIn  합니다.	// HUD_Toggle_BossStatusUI 에 통합.

	// 보스 체력바에 필요한 정보를 할당합니다.
	void		HUD_Bind_BossStatus(_wstring strUIBosssName, const _char* pMonsterKey, _float* pCurBossHP, _float* pCurBossSA, _bool* pIsGroggy, _float* pGroggyLeftRatio);
	// 보스 체력바를 토글합니다. 정보 할당 없이 On 시도 시 Crash.
	void		HUD_Toggle_BossStatusUI(_bool isOn, _bool isForceChange = false);

	// 상호작용 UI를 켭니다. / strText : 출력될 글자.
	void		Show_InteractUI(_wstring strText);
	// 상호작용 UI를 끕니다. / isPressedAs : 클릭으로 눌렸을 때처럼, 피드백 애니메이션 재생 후 사라질 것인지 여부
	// (해당 함수 호출 없이 마우스 클릭으로도 끌 수 있습니다,)
	void		Hide_InteractUI(_bool isPressedAs = false);

	// 상호작용 UI를 매 프레임 렌더 요청을 보냅니다.
	// 도중 isPressedAs가 true가 되거나, 렌더 호출이 1프레임 끊길 시 자동으로 Hide됩니다.
	void		Req_Render_InteractUI(_wstring strText, _bool isPressedAs = false);
	// 상호작용 UI가 "마우스"를 통해 상호작용되었는지를 반환합니다. 비활성 시 기본 false.
	_bool		Get_InteractUI_Feedback(UI_EVENT_TYPE eEventInteractType);


	// 락온 UI를 생성합니다. / *pTargetPos : 락온 대상의 위치 포인터.
	void		Attach_LockOnUI(_float3* pTargetPos);
	// 락온 UI를 해제합니다.
	void		Detach_LockOnUI();

	// 패리 UI를 생성합니다. / *pTargetPos : 락온 대상의 위치 포인터.
	// (일단은 생성 후 약 0.35초 = 21프레임 를 원이 겹치는 시점으로 잡았습니다.)
	void		Attach_Parry(_float3* pTargetPos);
	// 패리 UI가 살아있는 도중, 패리에 성공했음을 보냅니다. (원 즉시제거, 이펙트 이미지 출력)
	void		Enable_Parried();

	// 몬스터 HP바 표시를 위한 정보를 할당합니다. / &tDesc : 필요 정보 구조체
	// 살아 있는 동안 매 프레임 호출이 필요하며, 요구 구조체 내의 몬스터 키는 몹 주소를 할당해주시면 됩니다.
	void		Update_MobStatus(const UI_MOBINFO_DESC& tDesc);


	// 탭 유틸리티 UI를 켭니다. /  iCurSelectedUtilityIndex : 현재 선택중인 유틸리티 인덱스 (UI_TAB_UTILITY Enum을 따름)
	// 마우스 커서 락 해제 필요.(wip)
	void		Show_TabUtilityUI(_uint iCurSelectedUtilityIndex = ENUM_CLASS(UI_TAB_UTILITY::NOTHING));
	// 탭 유틸리티 UI를 끄라는 요청을 보내며 (애니메이션 재생을 위함), 선택한 유틸리티를 반환합니다.
	// 반환값은 Client_Enum 의 UI_TAB_UTILITY 를 따릅니다.
	_uint		HideNGet_TabUtilityUI();

	// 다채화를 켭니다. / iTargetLevel : 열 레벨. 오픈 대상 파일들은 CUI_Ovfl_Palette::Load_LevelData 에 순서대로 정의됨.
	void		Open_Game_OverflowPalette(_uint iTargetLevel = 0);
	// 다채화를 끕니다.
	void		Close_Game_OverflowPalette();

	// 그래플링 UI가 생길 지점의 점 위치를 할당합니다. (pooling 이용, 최대 50) 
	// 카메라 거리에 따른 크기 변화 기준 등 내부에서 상수로 변경 가능. 너무 멀면 렌더콜X
	void		Attach_GrapplePoint(_float3* pTargetPos, UI_GRAPPLE_TYPE eType);

	// [WIP] QTE 켜기. / _float2 : 스크린 상 스폰 좌표. (중점 0, 0, 우상단이 + 방향)
	// eQTEType : QTE 종류 (연타로 게이지채우기, 단발성 중 선택), eIconIndex : 사용 버튼 종류.
	void		Play_QTE(
		_float2 vSpawnPos		= _float2{0.f, 0.f}, 
		UI_QTE_TYPE eQTEType	= UI_QTE_TYPE::FILLGUAGE,
		UI_QTE_BTN eIconIndex	= UI_QTE_BTN::F,
		_float2 vScale			= _float2{1.f, 1.f}
	);		// 여기에 정보 받기용으로 out 포인터 인자라도 만들거나, status 같은 곳에 호출? 


	// [WIP] 미니맵에 표시할 정보를 추가/삭제합니다. PerFrame 함수는 매 프레임 호출이 필요합니다.
	//      임의로 색상/타입 추가 시, [UI_MINIMAP_OBJTYPE] 및 [UI_HUD_Sector_Minimap::PreAssign_Presets] 에서 추가 후 사용하시면 됩니다.
	void		Bind_ObjectPos_PerFrame_ToMinimap(const _float3& vPosition, UI_MINIMAP_OBJTYPE eType);			// 몬스터 등과 같이 실시간 갱신이 필요한 경우. Update_MobStatus 에 내장됨.
	void		Attach_ObjectPos_ToMinimap(const _float3& vPosition, UI_MINIMAP_OBJTYPE eType, void* pOwner);	// 상자 등과 같이 고정형 위치이며, 한번만 등록하는게 나은 경우. 실시간 갱신 X
	void		Detach_ObjectPos_ToMinimap(void* pOwner);														// 제거.


	// [WIP] 날아갈 궤적 및 충돌 예상 지점에의 구체를 표시합니다. 계산에 필요한 정보들의 매 프레임 갱신 필요.
	// - vStartPos : 시작 위치. 즉 오브젝트의 위치 + 오프셋 등
	// - vStartVelocity : 시작 속도. 즉, 던지려는 방향과 그 세기(power)
	// - vAcceleration : 가속도. (별일 없으면 중력가속도 _float3{0.f, -9.8, 0.f} 넣으면 될 듯)
	// ===== 이하는 필요 시 수정 ===== 
	// - fMaxTime : 해당 값 기준 몇초까지 날아갈 거리만큼 리본메쉬를 그릴 것인지
	// - iSegmentCount : 리본메쉬 정밀도 (낮으면 버텍스의 굴곡짐이 잘 보임)
	// - fRibbonWidth : 리본메쉬 가로두께
	// - isUseCustomColor : 색 커스텀 여부 (false로 둘 시 캐릭터 속성 색 사용. 기본값은 빨간색)
	// - vBaseColor : 기본색, vHeadColor : 시작방향 색, vTailColor : 끝 방향 색
	void		Req_Render_CurveTrace(
		_float3& vStartPos,
		_float3& vStartVelocity,
		_float3& vAcceleration, 
		_float fMaxTime = 4.f, 
		_uint iSegmentCount = 50, 
		_float fRibbonWidth = 0.25f, 
		_bool isUseCustomColor = true,
		_float4 vBaseColor = _float4(1.f, 1.f, 1.f, 1.f),
		_float4 vHeadColor = _float4(1.f, 0.f, 0.f, 1.f),
		_float4 vTailColor = _float4(.8f, 0.f, 0.f, 1.f)
	);


#pragma endregion

#pragma region PLAYER STATUS
	class CPlayerStatus* Get_PlayerStatus() const { return m_pPlayerStatus; }
#pragma endregion


#pragma region TRIGGER
	void TriggerRegister(_uint iNumTriggerMapIndex, TriggerCallback pFunc);
	void OnTriggerActivate(_uint iNumTriggerMapIndex, void* pArg = nullptr);
	//레벨 전환시 초기화 고려.
	void Clear_TriggerCallBack();
	const _tchar* Get_SonoroText();
#pragma endregion

#pragma region SONORO_MANAGER
	_bool* Add_To_Management(OBJECTTYPE eType, class CMapObject_Sonoro* pObjects, _bool** SonoroMode);
	_bool* Add_To_Management(OBJECTTYPE eType, class CMapObject_NonSonoro* pObjects, _bool** SonoroMode);
	_bool* Add_To_Management(INSTANCETYPE eType, class CMapObject_Instance* pObjects, _bool** SonoroMode);
	void Update(_float fTimeDelta);
	_bool  Change_Sonoro(_bool IsSonoro);
	_bool IsSonoro();

#pragma endregion

#pragma region MONSTER_TABLE
	HRESULT LoadMonsterTable(const _char* pFilePath);
	MONSTER_INFO* Get_MonsterInfo(const _char* pMonsterKey) const;
	HRESULT LoadNPCDataTable(const _char* pFilePath, _uint iType);
	_uint Get_NumNPCInstance(_uint iType) const;
	const vector<NPCINFO>& Get_NpcData(_uint iType) const;
#pragma endregion

#pragma region SFX_PREFAB
	void	Ready_SFX_Prefab(const _char* pFolderPath, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex);
#pragma endregion

#pragma region MOUSECONTROLLER
	void						Register_Mouse(class CMouse* pMouse);
	void						Set_MouseFix(_bool isFix);
	_bool						IsFix();
#pragma endregion

#pragma region PLAYER_INTERACT
	void						Bind_Condition_ToPlayer(const _string& strTransition);
	void						Call_Animation();
	void						Call_PlayerVisible();
	void						Unbind_Grab();
#pragma endregion

#pragma region PLAYER
	void						Register_SequencePlayer(class CSequencePlayer* pSequencePlayer);
	void						Register_Player(class CPlayer* pPlayer);
	_vector						Get_PlayerLookVector();
	_vector						Get_PlayerPosition();
	const _float4x4*			Get_PlayerMatrixPtr();

	void						Summon_SequenceCharacter(class CTransform* pTransform);

	
#pragma endregion


private:
	class	CParser*			m_pParser						= { nullptr };
	class	CFactory*			m_pFactory						= { nullptr };

	class	CUI_FontPreset*		m_pUI_FontPreset				= { nullptr };
	class	CUI_ControlHelper*	m_pUI_ControlHelper				= { nullptr };
	//class	CUI_StatusSyncer*	m_pUI_StatusSyncer				= { nullptr };

	class	CDirector*			m_pDirector 					= { nullptr };
	class	CPlayerStatus* 		m_pPlayerStatus 				= { nullptr };
	class	CPlayer*			m_pPlayer						= { nullptr };
	class   CSequencePlayer*	m_pSequencePlayer				= { nullptr };
	
	class	CSonoro_Manager*	m_pSonoro_Manager				= { nullptr };

	class	CMonsterTable*		m_pMonsterTable					= { nullptr };
	class	CMouseController*	m_pMouseController				= { nullptr };

	unordered_map<_uint, vector<TriggerCallback>> m_TriggerEvents;
	Mutex m_Mutex;
public:
	void				Release_System();

	virtual		void	Free() override;

};

NS_END