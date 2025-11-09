#pragma once
#include "Component.h"

NS_BEGIN(Client)
class CAbility final : public CComponent
{
public:
	enum KEY
	{
		KEY_LB = 0,
		KEY_T,
		KEY_E,
		KEY_Q,
		KEY_R,
		KEY_END
	};

private:
	explicit CAbility(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAbility(const CAbility& Prototype);
	virtual ~CAbility() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize_Clone(void* pArg);
	virtual void Update(_float fTimeDelta);
	void Register_AllAbilityFiles(const _string& strFolderPath);
	void Update_CostCondition(_float fTimeDelta);
	void UISlotUpdate(_float fTimeDelta);
	
public:
	void Set_UICharacterType(_uint eCharactertType);


#pragma region UI Interface
public:
	// 쿨타임 중인 얘들
	// 최소 쿨타임 / 최대 쿨타임.
	_float Get_RemainingCooldown(const _string& strSkillName) const;  // 기존
	_float Get_MaxCooldown(const _string& strSkillName);        // 기존

	_float Get_HpRatio() const;

	_bool Check_AnyCondition(_uint iConditionFlag); // 현재 컨디션 제어
	_bool Check_AllCondition(_uint iConditionFlag); // 모든 컨디션 확인

	// 수치 값들 (공명 , HP 게이지 등등)
	_float Get_CostRatio(COST_TYPE eType) const;    // 그냥 캐릭터들 특수 Cost로 사용할 거같고.
	_float Get_Hp() { return m_CharacterInfo.fHp; } // 현재 Hp
	_float Get_MaxHp() { return m_CharacterInfo.fMaxHp; } // Max Hp
	_float Get_Resonance() { return m_CharacterInfo.fResonance; }
	

	const vector<UISKILL_SLOT>& Get_UISkillSlots() const { return m_UISlots; }
#pragma endregion


#ifdef _DEBUG
public:
	void Print_KeySlotinfo();
#else
	void Print_KeySlotinfo();
#endif // _DEBUG



#pragma region Character State Machine
public:
	const SKILL_INFO* Get_SkillInfo(const _string& strSkillName);
	const CHARACTER_INFO& Get_CharacterInfo() const { return m_CharacterInfo; }
	_float Get_Cost(COST_TYPE eType) const;
	SKILL_STATE Check_SkillState(const _string& strSkillName, const _string& strPrevName = "");
	SKILL_STATE TryUseSkill(const _string& strSkillName); // 스킬 사용 시도
	void Set_Cost(COST_TYPE eType, _float fCost);
	void Add_Cost(COST_TYPE eType, _float fCost);
	void Set_Hp(_float fHp);
	void Add_Hp(_float fHp);
	void Set_Resonance(_float fResonance);
	void Add_Resonance(_float fResonance);

	void Bind_Condition(_uint iCondition);
	void Remove_Condition(_uint iCondition);

	void Bind_CostCondition(_uint iCostType, _uint iConditionFlag);
	

	UISKILL_SLOT Determine_State(_uint iCharacterIdx, const _string& strKey); // CharacterIdx와 누른 키.
	UISKILL_SLOT Determine_StateRover(_uint iCharacterIdx, const _string& strKey); // CharacterIdx와 누른 키.
	UISKILL_SLOT Determine_StateAugusta(_uint iCharacterIdx, const _string& strKey); // CharacterIdx와 누른 키.
	UISKILL_SLOT Determine_StateGalbrena(_uint iCharacterIdx, const _string& strKey); // CharacterIdx와 누른 키.

#pragma endregion

#ifdef _DEBUG
public:
	void Debug_FullCost(_bool IsAll = false);

	void Print_Cost();
	void Print_CoolTime();
#else
	void Print_Cost();
	void Print_CoolTime();
public:
	void Debug_FullCost(_bool IsAll = false);
#endif // _DEBUG


private:
	// 1. 스킬 원본 데이터를 저장 (Key: 스킬 이름, Value: 스킬 정보)
	unordered_map<_string, SKILL_INFO> m_mapSkills;

	// 2. 스킬 체인 데이터 (Key : 이전 스킬 이름, Value : 다음 스킬 정보)
	unordered_map<_string, _string> m_mapSkillChain;

	// 3. 현재 쿨타임이 돌고 있는 스킬 목록 (Key : 스킬 이름, Value: 남은 쿨타임)
	unordered_map<_string, _float> m_mapSkillCooldowns;

	// 4. UI에 전달할 SkillSLot
	vector<UISKILL_SLOT> m_UISlots; // 매프레임 업데이트
	
	// 5. Keys.
	vector<_string> m_Keys = {};

	// 6. 특정 상황에 특정 Cost가 자연적으로 감소되는 경우 ex) 인멸자 공명 게이지 강공 실행시
	unordered_map<_uint, _uint> m_mapCostConditions;

	CHARACTER_INFO m_CharacterInfo = {};
	_string m_strPrevSkillName = {};

	

	// 7. 자기가 어떤 캐릭터인지 알 수 있게. => 어디서 초기화하지?
	_uint m_iCharacter = {}; 
	_uint m_iCondition = {}; // 캐릭터에 따라 컨디션 변경.


	/*
	* 소모값 기본 0.f으로 소유
	* 사용 예시.
		SKILL_INFO eSkillInfo
		Costs[ENUM_CLASS(eSkillInfo.eStatType)] -= eSkillInfo.fCost;
	*/
	vector<_float> m_Costs;
	const _float m_fCostMax = { 100.f };

private:
	void Read_Skill(const _char* pFilePath);
	void Read_Stat(const _char* pFilePath);
	const vector<vector<_string>>& Load_CSV(const _char* pFolderPath);

private:
	
	SKILL_TYPE ConvertSkillType(const _string& strCostType);
	COST_TYPE ConvertCostType(const _string& strCostType);


public:
	static CAbility* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END

