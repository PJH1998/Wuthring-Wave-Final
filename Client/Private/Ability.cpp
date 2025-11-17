#include "ClientPch.h"
#include "Ability.h"
#include "GameInstance.h"
#include "GameSystem.h"

CAbility::CAbility(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{
}

CAbility::CAbility(const CAbility& Prototype)
	: CComponent ( Prototype)
{
}

HRESULT CAbility::Initialize_Prototype()
{
	if (FAILED(CComponent::Initialize_Prototype()))
		return E_FAIL;

	m_UISlots.resize(KEY_END);
	m_Keys.resize(KEY_END);
	m_Keys[KEY_LB] = "LB";
	m_Keys[KEY_T] = "T";
	m_Keys[KEY_E] = "E";
	m_Keys[KEY_Q] = "Q";
	m_Keys[KEY_R] = "R";

	return S_OK;
}

HRESULT CAbility::Initialize_Clone(void* pArg)
{
	if (FAILED(CComponent::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

// Ability에서 매프레임 업데이트 해야 하는 정보
void CAbility::Update(_float fTimeDelta)
{
	// 쿨타임이 돌고 있는 스킬들을 순회
	for (auto iter = m_mapSkillCooldowns.begin(); iter != m_mapSkillCooldowns.end(); )
	{
		// 남은 쿨타임 감소
		iter->second -= fTimeDelta;

		// 쿨타임이 다 끝났다면 맵에서 제거
		if (iter->second <= 0.f)
		{
			iter = m_mapSkillCooldowns.erase(iter);
		}
			
		else
			++iter;
	}

	Update_CostCondition(fTimeDelta);

	// 매프레임 Stamina 자동 회복. // 초당 10
	Add_Cost(COST_TYPE::STAMINA, fTimeDelta * 10.f);

	// UI 슬롯 업데이트
	UISlotUpdate(fTimeDelta);
}

void CAbility::Register_AllAbilityFiles(const _string& strFolderPath)
{
	_string skillPath = strFolderPath + "Skill.csv";
	_string statPath = strFolderPath + "Stat.csv";

	Read_Skill(skillPath.c_str());
	Read_Stat(statPath.c_str());
}

void CAbility::Update_CostCondition(_float fTimeDelta)
{

	// 감소시켜야할 Cost가 있다면?
	for (auto iter = m_mapCostConditions.begin(); iter != m_mapCostConditions.end();)
	{
		COST_TYPE eCostType = static_cast<COST_TYPE>(iter->first);

		// 초당 10.f 감소.
		Add_Cost(eCostType, -fTimeDelta * 10.f);

		// Cost가 0.f 라면? 제거.
		if (m_Costs[iter->first] <= 0.f)
		{
			Remove_Condition(iter->second);
			Set_Cost(eCostType, 0.f); // 0으로 초기화

			iter = m_mapCostConditions.erase(iter);
		}
		else
			++iter;
	}
}

void CAbility::UISlotUpdate(_float fTimeDelta)
{
	// 1. 기존 내용 지우기 (재사용)
	m_UISlots.clear();
	for (const auto& strKey : m_Keys)
	{
		UISKILL_SLOT eSkillSlot = Determine_State(m_iCharacter, strKey);
		m_UISlots.emplace_back(eSkillSlot);
	}

}



void CAbility::Set_UICharacterType(_uint iCharactertType)
{
	m_iCharacter = iCharactertType;
}

#pragma region UI Interface
// 지금 쿨타임 돌고 있는 스킬들 쿨타임 검색.
_float CAbility::Get_RemainingCooldown(const _string& strSkillName) const
{
	auto iter = m_mapSkillCooldowns.find(strSkillName);
	return (iter != m_mapSkillCooldowns.end()) ? iter->second : 0.f;
}

_float CAbility::Get_MaxCooldown(const _string& strSkillName)
{
	const SKILL_INFO* pInfo = Get_SkillInfo(strSkillName);
	return (pInfo) ? pInfo->fCoolDown : 0.f;
}

// Cost 
_float CAbility::Get_CostRatio(COST_TYPE eType) const
{
	_float fCost = Get_Cost(eType);
	return (m_fCostMax > 0.f) ? fCost / m_fCostMax : 0.f;
}

_float CAbility::Get_HpRatio() const
{
	return m_CharacterInfo.fMaxHp > 0.f ? m_CharacterInfo.fHp / m_CharacterInfo.fMaxHp : 0.f;
}

_bool CAbility::Check_AnyCondition(_uint iConditionFlag)
{
	return (m_iCondition & iConditionFlag) != 0;
}

_bool CAbility::Check_AllCondition(_uint iConditionFlag)
{
	return (m_iCondition & iConditionFlag) == iConditionFlag;
}

#pragma endregion


#ifdef _DEBUG
void CAbility::Print_KeySlotinfo()
{
	for (_uint i = 0; i < KEY_END; ++i)
	{
		stringstream ss;


		_string stateType = {};
		if (m_UISlots[i].iStateType == 0)
		{
			stateType = "LB_STRONG_READY";
		}
		else if (m_UISlots[i].iStateType == 1)
		{
			stateType = "LB_SWORD_READY";
		}
		else if (m_UISlots[i].iStateType == 2)
		{
			stateType = "T_INTERACTION_READY";
		}
		else if (m_UISlots[i].iStateType == 3)
		{
			stateType = "T_INTERACTION_FAILED";
		}
		else if (m_UISlots[i].iStateType == 4)
		{
			stateType = "E_GRIFFON_READY";
		}
		else if (m_UISlots[i].iStateType == 5)
		{
			stateType = "E_RISE_READY";
		}
		else if (m_UISlots[i].iStateType == 6)
		{
			stateType = "E_DEFAULT_READY";
		}
		else if (m_UISlots[i].iStateType == 7)
		{
			stateType = "Q_ECHO_READY";
		}
		else if (m_UISlots[i].iStateType == 7)
		{
			stateType = "Q_ECHO_FAILED";
		}
		else if (m_UISlots[i].iStateType == 8)
		{
			stateType = "Q_ECHO_FAILED";
		}
		else if (m_UISlots[i].iStateType == 9)
		{
			stateType = "R_ULTI_READY";
		}
		else if (m_UISlots[i].iStateType == 10)
		{
			stateType = "R_SWORD_READY";
		}
		else if (m_UISlots[i].iStateType == 11)
		{
			stateType = "R_SWORD_ULTI_READY";
		}



		ss << "====================================" << endl
			<< "Key Input : " << m_UISlots[i].strKeyInput << endl
			<< "CharacterType : " << m_UISlots[i].iCharacterType << endl
			<< "iStateType : " << stateType << endl
			<< "fCurrentCoolTime : " << m_UISlots[i].fCurrentCoolTime << endl
			<< "fMaxCoolTime : " << m_UISlots[i].fMaxCoolTime << endl
			<< "strSkillName : " << m_UISlots[i].strSkillName << endl
			<< "====================================" << endl;

		cout << ss.str();
		//_wstring strDebug = StringToWString(ss.str());

		///OutputDebugString(strDebug.c_str());
	}
}
#else
void CAbility::Print_KeySlotinfo()
{
	for (_uint i = 0; i < KEY_END; ++i)
	{
		stringstream ss;


		_string stateType = {};
		if (m_UISlots[i].iStateType == 0)
		{
			stateType = "LB_STRONG_READY";
		}
		else if (m_UISlots[i].iStateType == 1)
		{
			stateType = "LB_SWORD_READY";
		}
		else if (m_UISlots[i].iStateType == 2)
		{
			stateType = "T_INTERACTION_READY";
		}
		else if (m_UISlots[i].iStateType == 3)
		{
			stateType = "T_INTERACTION_FAILED";
		}
		else if (m_UISlots[i].iStateType == 4)
		{
			stateType = "E_GRIFFON_READY";
		}
		else if (m_UISlots[i].iStateType == 5)
		{
			stateType = "E_RISE_READY";
		}
		else if (m_UISlots[i].iStateType == 6)
		{
			stateType = "E_DEFAULT_READY";
		}
		else if (m_UISlots[i].iStateType == 7)
		{
			stateType = "Q_ECHO_READY";
		}
		else if (m_UISlots[i].iStateType == 7)
		{
			stateType = "Q_ECHO_FAILED";
		}
		else if (m_UISlots[i].iStateType == 8)
		{
			stateType = "Q_ECHO_FAILED";
		}
		else if (m_UISlots[i].iStateType == 9)
		{
			stateType = "R_ULTI_READY";
		}
		else if (m_UISlots[i].iStateType == 10)
		{
			stateType = "R_SWORD_READY";
		}
		else if (m_UISlots[i].iStateType == 11)
		{
			stateType = "R_SWORD_ULTI_READY";
		}



		ss << "====================================" << endl
			<< "Key Input : " << m_UISlots[i].strKeyInput << endl
			<< "CharacterType : " << m_UISlots[i].iCharacterType << endl
			<< "iStateType : " << stateType << endl
			<< "fCurrentCoolTime : " << m_UISlots[i].fCurrentCoolTime << endl
			<< "fMaxCoolTime : " << m_UISlots[i].fMaxCoolTime << endl
			<< "strSkillName : " << m_UISlots[i].strSkillName << endl
			<< "====================================" << endl;

		cout << ss.str();
		//_wstring strDebug = StringToWString(ss.str());

		///OutputDebugString(strDebug.c_str());
	}
}

#endif // _DEBUG




const SKILL_INFO* CAbility::Get_SkillInfo(const _string& strSkillName)
{
	auto iter = m_mapSkills.find(strSkillName);

	if (iter != m_mapSkills.end())
	{
		// 찾았으면 해당 SKILL_INFO 객체의 주소(포인터)를 반환
		return &(iter->second);
	}

	// 못 찾았으면 nullptr 반환
	return nullptr;
}

_float CAbility::Get_Cost(COST_TYPE eType) const
{;
	_uint iType = ENUM_CLASS(eType);
	if (iType > m_Costs.size())
		return -1.f; // 잘못된 값

	// Cost Float 값 반환.
	return m_Costs[iType];
}

// 사용 전에 무조건 Check_SkillState호출?
SKILL_STATE CAbility::TryUseSkill(const _string& strSkillName)
{
	
	// 1. 먼저 스킬 사용이 가능한지 상태를 체크
	const SKILL_STATE eState = Check_SkillState(strSkillName);

	// 2. 사용 가능 상태가 아니면, 해당 상태를 반환하고 종료
	if (SKILL_STATE::READY != eState)
		return eState;

	// 3. 사용 가능하므로, 자원 소모 및 쿨타임 적용
	const SKILL_INFO* pSkillInfo = Get_SkillInfo(strSkillName);
	if (nullptr == pSkillInfo)
		return SKILL_STATE::NOT_EXIST;

	// 4. 자원 소모
	switch (pSkillInfo->eCostType)
	{
	case COST_TYPE::NONE:
		break; // 소모 없음
	default:
		Add_Cost(pSkillInfo->eCostType, pSkillInfo->fCost);
		break;
	}

	// 5. 쿨타임 적용 (쿨타임이 0초보다 클 경우)
	if (pSkillInfo->fCoolDown > 0.f)
		m_mapSkillCooldowns.emplace(strSkillName , pSkillInfo->fCoolDown); // 쿨타임이 0초보다 큰 경우에만 쿨타임 적용.

	// 6. KeyInput과 Phase를 전달하기.
	m_strPrevSkillName = strSkillName;

	// 7. 스킬 사용 성공 (READY 상태였음)
	return SKILL_STATE::READY;
}

void CAbility::Set_Cost(COST_TYPE eType, _float fCost)
{
	_uint iType = ENUM_CLASS(eType);

	if (iType >= m_Costs.size())
		return;

	m_Costs[iType] = min(fCost, m_fCostMax);
}

void CAbility::Add_Cost(COST_TYPE eType, _float fCost)
{
	_uint iType = ENUM_CLASS(eType);

	if (iType >= m_Costs.size())
		return;

	m_Costs[iType] = min(m_Costs[iType] + fCost, m_fCostMax);
}

void CAbility::Set_Hp(_float fHp)
{
	m_CharacterInfo.fHp = fHp;
}

void CAbility::Add_Hp(_float fHp)
{
	m_CharacterInfo.fHp += fHp;

	// 0보다 아래로 안가도록.
	m_CharacterInfo.fHp = max(0.f, m_CharacterInfo.fHp);

	// MaxHp보다 안커지도록.
	m_CharacterInfo.fHp = min(m_CharacterInfo.fMaxHp, m_CharacterInfo.fHp);
}

void CAbility::Set_HarmonyGauge(_float fResonance)
{
	m_CharacterInfo.fHarmonyGauge = fResonance;
}

void CAbility::Add_HarmonyGauge(_float fResonance)
{
	m_CharacterInfo.fHarmonyGauge += fResonance;

	// 0보다 아래로 안가도록.
	m_CharacterInfo.fHarmonyGauge = max(0.f, m_CharacterInfo.fHarmonyGauge);

	// MaxHp보다 안커지도록.
	m_CharacterInfo.fHarmonyGauge = min(100.f, m_CharacterInfo.fHarmonyGauge);
}

void CAbility::Bind_Condition(_uint iCondition)
{
	m_iCondition |= iCondition;
}

void CAbility::Remove_Condition(_uint iCondition)
{
	m_iCondition &= ~iCondition; // 반전 마스크 적용.
}

void CAbility::Bind_CostCondition(_uint iCostType, _uint iConditionFlag)
{
	// 시간에 따라 Cost 감소 시작.
	m_mapCostConditions.emplace(iCostType, iConditionFlag);
}





UISKILL_SLOT CAbility::Determine_State(_uint iCharacterIdx, const _string& strKey)
{
	UI_CHARACTERTYPE eType = static_cast<UI_CHARACTERTYPE>(iCharacterIdx);
	switch (eType)
	{
	case UI_CHARACTERTYPE::ROVER:
		return Determine_StateRover(iCharacterIdx, strKey);
		break;
	case UI_CHARACTERTYPE::AUGUSTA:
		return Determine_StateAugusta(iCharacterIdx, strKey);
		break;
	case UI_CHARACTERTYPE::GALBRENA:
		return Determine_StateGalbrena(iCharacterIdx, strKey);
		break;
	}

	return UISKILL_SLOT();
}

UISKILL_SLOT CAbility::Determine_StateRover(_uint iCharacterIdx, const _string& strKey)
{
	UISKILL_SLOT skillSlot{};

	skillSlot.strKeyInput = strKey;
	skillSlot.iCharacterType = iCharacterIdx;
	skillSlot.iStateType = ENUM_CLASS(UI_ROVER_STATE::DEFAULT);
	skillSlot.fMaxCoolTime = 0.f;

	// E 공격. => 말고 슬롯 안바뀜.
	if (strKey == "E")
	{
		if (m_iCondition & ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE))
		{
			const SKILL_INFO* pSkillInfo = Get_SkillInfo("Ex_Skill02");
			if (nullptr != pSkillInfo)
			{
				skillSlot.fCurrentCoolTime = Get_RemainingCooldown("Ex_Skill02");
				skillSlot.fMaxCoolTime = pSkillInfo->fCoolDown;
				skillSlot.iStateType = ENUM_CLASS(UI_ROVER_STATE::E_BURST_READY);
			}
		}
		else
		{
			const SKILL_INFO* pSkillInfo = Get_SkillInfo("Skill02");
			if (nullptr != pSkillInfo)
			{
				skillSlot.fCurrentCoolTime = Get_RemainingCooldown("Skill02");
				skillSlot.fMaxCoolTime = pSkillInfo->fCoolDown;
				skillSlot.iStateType = ENUM_CLASS(UI_ROVER_STATE::E_DEFAULT_READY);
			}
		}
	}
	else if (strKey == "R")
	{
		skillSlot.fCurrentCoolTime = Get_RemainingCooldown("Burst01_Ulti");
		const SKILL_INFO* pSkillInfo = Get_SkillInfo("Burst01_Ulti");
		if (nullptr != pSkillInfo)
			skillSlot.fMaxCoolTime = pSkillInfo->fCoolDown;
		skillSlot.iStateType = ENUM_CLASS(UI_ROVER_STATE::R_READY);
	}




	return skillSlot;
}

UISKILL_SLOT CAbility::Determine_StateAugusta(_uint iCharacterIdx, const _string& strKey)
{
	UISKILL_SLOT skillSlot{};
	
	// 우선 MOUSE LB
	skillSlot.strKeyInput = strKey;
	skillSlot.iCharacterType = iCharacterIdx;
	skillSlot.fMaxCoolTime = 0.f;
	skillSlot.iStateType = ENUM_CLASS(UI_AUGUSTA_STATE::DEFAULT);
	if (strKey == "LB")
	{
		// Bit And 연산해서 걸리면?
		// 우선 순위별
		if (m_iCondition & ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK))
		{
			skillSlot.fCurrentCoolTime = 0.f;
			skillSlot.fMaxCoolTime = 0.f;
			skillSlot.iStateType = ENUM_CLASS(UI_AUGUSTA_STATE::LB_SWORD_READY);
		}
		else if (m_Costs[ENUM_CLASS(COST_TYPE::COST1)] >= m_fCostMax)
		{
			skillSlot.fCurrentCoolTime = 0.f;
			skillSlot.fMaxCoolTime = 0.f;
			skillSlot.iStateType = ENUM_CLASS(UI_AUGUSTA_STATE::LB_STRONG_READY); // 강공 실행 가능.
		}
	}
	else if (strKey == "T")
	{

	}
	else if (strKey == "E")
	{


		// 1. Skill Strike에 진입하자마자 컨디션을 E_RISE_READY로 변경.
		if (m_iCondition & ENUM_CLASS(UI_AUGUSTA_CONDITION::E_RISE))
		{
			skillSlot.fCurrentCoolTime = 0.f;
			skillSlot.fMaxCoolTime = 0.f;
			skillSlot.iStateType = ENUM_CLASS(UI_AUGUSTA_STATE::E_RISE_READY);
		}
		// 2. Griffon의 컨디션 상태면 Griffon Ready가 가능하게?
		else if ((m_Costs[ENUM_CLASS(COST_TYPE::COST2)] >= m_fCostMax) || Check_AnyCondition(ENUM_CLASS(UI_AUGUSTA_CONDITION::E_GRIFFON)))
		{
			skillSlot.fCurrentCoolTime = 0.f;
			skillSlot.fMaxCoolTime = 0.f;
			skillSlot.iStateType = ENUM_CLASS(UI_AUGUSTA_STATE::E_GRIFFON_READY);
		}
		else
		{
			// 3. 아무런 상태가 아닌 경우 기본 E가 나오게?
			skillSlot.fCurrentCoolTime = Get_RemainingCooldown("Skill_Hack");
			const SKILL_INFO* pSkillInfo = Get_SkillInfo("Skill_Hack");
			if (nullptr != pSkillInfo)
				skillSlot.fMaxCoolTime = pSkillInfo->fCoolDown;
			skillSlot.iStateType = ENUM_CLASS(UI_AUGUSTA_STATE::E_DEFAULT_READY);
		}

		//else if (m_Costs[ENUM_CLASS(COST_TYPE::COST2)] >= m_fCostMax)
		//{
		//	skillSlot.fCurrentCoolTime = 0.f;
		//	skillSlot.fMaxCoolTime = 0.f;
		//	skillSlot.iStateType = ENUM_CLASS(UI_AUGUSTA_STATE::E_GRIFFON_READY);
		//}
		//// Rise 다음 단계에서 그리폰이 나와야하는데 바로 기본 스킬이 나옴.

		
	}
	else if (strKey == "Q")
	{
		//

	}
	else if (strKey == "R")
	{
		if (m_iCondition & ENUM_CLASS(UI_AUGUSTA_CONDITION::R_SP_ATTACKOMNI))
		{
			skillSlot.fCurrentCoolTime = 0.f;
			skillSlot.fMaxCoolTime = 0.f;
			skillSlot.iStateType = ENUM_CLASS(UI_AUGUSTA_STATE::R_SWORD_ULTI_READY);
		}
		else if (m_Costs[ENUM_CLASS(COST_TYPE::COST3)] >= m_fCostMax)
		{
			skillSlot.fCurrentCoolTime = 0.f;
			skillSlot.fMaxCoolTime = 0.f;
			skillSlot.iStateType = ENUM_CLASS(UI_AUGUSTA_STATE::R_SWORD_READY);
		}
		else
		{
			// 궁극기..
			skillSlot.fCurrentCoolTime = Get_RemainingCooldown("Attack_SpeedDrive");
			const SKILL_INFO* pSkillInfo = Get_SkillInfo("Attack_SpeedDrive");
			if (nullptr != pSkillInfo)
				skillSlot.fMaxCoolTime = pSkillInfo->fCoolDown;
			skillSlot.iStateType = ENUM_CLASS(UI_AUGUSTA_STATE::R_ULTI_READY);
		}
	}




	return skillSlot;
}

UISKILL_SLOT CAbility::Determine_StateGalbrena(_uint iCharacterIdx, const _string& strKey)
{
	UISKILL_SLOT skillSlot{};

	skillSlot.strKeyInput = strKey;
	skillSlot.iCharacterType = iCharacterIdx;
	skillSlot.iStateType = ENUM_CLASS(UI_GALBRENA_STATE::DEFAULT);
	skillSlot.fMaxCoolTime = 0.f;

	// E 공격. => 말고 슬롯 안바뀜.
	if (strKey == "E")
	{
		if (m_iCondition & ENUM_CLASS(UI_GALBRENA_CONDITION::BURST_ACTIVE))
		{
			const SKILL_INFO* pSkillInfo = Get_SkillInfo("Ex_Skill02");
			if (nullptr != pSkillInfo)
			{
				skillSlot.fCurrentCoolTime = Get_RemainingCooldown("Ex_Skill02");
				skillSlot.fMaxCoolTime = pSkillInfo->fCoolDown;
				skillSlot.iStateType = ENUM_CLASS(UI_GALBRENA_STATE::E_BURST_READY);
			}
		}
		else
		{
			const SKILL_INFO* pSkillInfo = Get_SkillInfo("Attack_Jump_Start");
			if (nullptr != pSkillInfo)
			{
				skillSlot.fCurrentCoolTime = Get_RemainingCooldown("Attack_Jump_Start");
				skillSlot.fMaxCoolTime = pSkillInfo->fCoolDown;
				skillSlot.iStateType = ENUM_CLASS(UI_ROVER_STATE::E_DEFAULT_READY);
			}
		}
	}
	else if (strKey == "R")
	{
	
	}

	return skillSlot;
}

#ifdef _DEBUG
void CAbility::Debug_FullCost(_bool IsAll)
{
	_uint iStart = ENUM_CLASS(COST_TYPE::COST1);
	_uint iEnd = ENUM_CLASS(COST_TYPE::COST_TYPE_END);

	if (!IsAll)
	{
		for (_uint i = 1; i < iEnd; ++i)
		{
			if (i == 3 || i == 4)
				continue;

			m_Costs[i] = m_fCostMax;
		}
	}
	else
	{
		for (_uint i = 1; i < iEnd; ++i)
		{
			m_Costs[i] = m_fCostMax;
		}

		// 풀로 채우기.
		m_CharacterInfo.fHarmonyGauge = m_CharacterInfo.fMaxHarmonyGauge;
	}
}

void CAbility::Print_Cost()
{
	cout << "Cost 1 : " << m_Costs[ENUM_CLASS(COST_TYPE::COST1)] << endl;
	cout << "Cost 2 : " << m_Costs[ENUM_CLASS(COST_TYPE::COST2)] << endl;
	cout << "Cost 3 : " << m_Costs[ENUM_CLASS(COST_TYPE::COST3)] << endl;
	cout << "Cost 4 : " << m_Costs[ENUM_CLASS(COST_TYPE::COST4)] << endl;
	cout << "Cost 5 : " << m_Costs[ENUM_CLASS(COST_TYPE::COST5)] << endl;
	cout << "Cost Stamina : " << m_Costs[ENUM_CLASS(COST_TYPE::STAMINA)] << endl;
}

void CAbility::Print_CoolTime()
{
	cout << "Cool Down " << endl;
	for (auto& pair : m_mapSkillCooldowns)
	{
		cout << pair.first << " : " << pair.second << endl;
	}
	cout << "Cool Down End" << endl;
}
#else
void CAbility::Debug_FullCost(_bool IsAll)
{
	_uint iStart = ENUM_CLASS(COST_TYPE::COST1);
	_uint iEnd = ENUM_CLASS(COST_TYPE::COST_TYPE_END);

	if (!IsAll)
	{
		for (_uint i = 1; i < iEnd; ++i)
		{

			if (i == 3 || i == 4)
				continue;

			m_Costs[i] = m_fCostMax;

		}
	}
	else
	{
		for (_uint i = 1; i < iEnd; ++i)
		{
			m_Costs[i] = m_fCostMax;
		}
	}
}

void CAbility::Print_Cost()
{
	cout << "Cost 1 : " << m_Costs[ENUM_CLASS(COST_TYPE::COST1)] << endl;
	cout << "Cost 2 : " << m_Costs[ENUM_CLASS(COST_TYPE::COST2)] << endl;
	cout << "Cost 3 : " << m_Costs[ENUM_CLASS(COST_TYPE::COST3)] << endl;
	cout << "Cost 4 : " << m_Costs[ENUM_CLASS(COST_TYPE::COST4)] << endl;
	cout << "Cost 5 : " << m_Costs[ENUM_CLASS(COST_TYPE::COST5)] << endl;
	cout << "Cost Stamina : " << m_Costs[ENUM_CLASS(COST_TYPE::STAMINA)] << endl;
}

void CAbility::Print_CoolTime()
{
	cout << "Cool Down " << endl;
	for (auto& pair : m_mapSkillCooldowns)
	{
		cout << pair.first << " : " << pair.second << endl;
	}
	cout << "Cool Down End" << endl;
}
#endif // _DEBUG



void CAbility::Read_Skill(const _char* pFilePath)
{
	vector<vector<_string>> data = CGameSystem::GetInstance()->Load_CSV(pFilePath);

	if (data.size() < 2)
	{
		MSG_BOX("Data Not Found");
		return;
	}
		
	for (_uint i = 1; i < data.size(); ++i)
	{
		// 1. 새 스킬 정보 객체 생성
		SKILL_INFO eSkill; 

		// 2. CSV 데이터 파싱 및 저장
		eSkill.strSkillName = data[i][0];
		eSkill.strPrevName = data[i][1];
		eSkill.strKeyInput = data[i][2];
		eSkill.strKeyType = data[i][3];
		eSkill.eSkillType = ConvertSkillType(data[i][4]);
		eSkill.eCostType = ConvertCostType(data[i][5]);
		eSkill.fCost = stof(data[i][6]);
		
		eSkill.fCoolDown = stof(data[i][7]);
		eSkill.fDamage = stof(data[i][8]);
		eSkill.strDescription = data[i][9];

		// 3. 메인 스킬 맵에 저장
		m_mapSkills.emplace(eSkill.strSkillName, eSkill);
	}
}

void CAbility::Read_Stat(const _char* pFilePath)
{
	vector<vector<_string>> data = CGameSystem::GetInstance()->Load_CSV(pFilePath);

	if (data.size() < 2)
	{
		MSG_BOX("Data Not Found");
		return;
	}

	m_CharacterInfo.strName = data[1][0];
	m_CharacterInfo.fHp = stof(data[1][1]);
	m_CharacterInfo.fMaxHp = stof(data[1][2]);
	m_CharacterInfo.fMaxStamina = stof(data[1][3]);
	
	m_Costs.resize(ENUM_CLASS(COST_TYPE::COST_TYPE_END));
	m_Costs[ENUM_CLASS(COST_TYPE::COST1)] = 0.f;
	m_Costs[ENUM_CLASS(COST_TYPE::COST2)] = 0.f;
	m_Costs[ENUM_CLASS(COST_TYPE::COST3)] = 0.f;
	m_Costs[ENUM_CLASS(COST_TYPE::COST4)] = 0.f;
	m_Costs[ENUM_CLASS(COST_TYPE::COST5)] = 0.f;
	m_Costs[ENUM_CLASS(COST_TYPE::STAMINA)] = m_CharacterInfo.fMaxStamina;

	m_CharacterInfo.fAttack = stof(data[1][10]);
	m_CharacterInfo.fAttackAddMin = stof(data[1][11]);
	m_CharacterInfo.fAttackAddMax = stof(data[1][12]);
	m_CharacterInfo.fMaxHarmonyGauge = stof(data[1][13]);
}


SKILL_STATE CAbility::Check_SkillState(const _string& strSkillName, const _string& strPrevName)
{
	const SKILL_INFO* pSkillInfo = Get_SkillInfo(strSkillName);

	// 1. 스킬이 존재하는가?
	if (nullptr == pSkillInfo)
		return SKILL_STATE::NOT_EXIST;

	// 2. 쿨타임 중인가?
	if (m_mapSkillCooldowns.count(strSkillName) > 0)
		return SKILL_STATE::COOLING_DOWN;

	// 3-1. prevName이 empty가 아니라면?
	if (!strPrevName.empty())
	{
		// 3-2. 매개변수로 받은 이전 애님 이름이 등록된 현재 스킬의 이전 이름과 동일하지 않다면?
		if (pSkillInfo->strPrevName != strPrevName)
		{ 
			return SKILL_STATE::NOT_EXIST;
		}
	}

	// 4.  코스트가 SkillInfo에 등록된 fCost가 증가하는 것이라면?
	if (pSkillInfo->fCost > 0.f)
		return SKILL_STATE::READY; // 증가만 시켜주면됨.

	// 5. 현재 코스트와 SkillInfo에 등록된 fCost를 더했을때 0보다 작다면? (부호가 음수가 있음)
	if (m_Costs[ENUM_CLASS(pSkillInfo->eCostType)] + pSkillInfo->fCost < 0.f)
		return SKILL_STATE::NOT_ENOUGH_COST;

	// 모든 조건을 통과
	return SKILL_STATE::READY;
}

SKILL_TYPE CAbility::ConvertSkillType(const _string& strCostType)
{
	SKILL_TYPE eCommonSkillType = SKILL_TYPE::SKILL_TYPE_END;
	if (strCostType == "NONE")
	{
		eCommonSkillType = SKILL_TYPE::NONE;
	}
	else if (strCostType == "RESONANCE")
	{
		eCommonSkillType = SKILL_TYPE::RESONANCE;
	}
	else if (strCostType == "AUGUSTA_POINT")
	{
		eCommonSkillType = SKILL_TYPE::AUGUSTA_POINT;
	}
	else if (strCostType == "AUGUSTA_ULTI")
	{
		eCommonSkillType = SKILL_TYPE::AUGUSTA_ULTI;
	}
	else if (strCostType == "AUGUSTA_SWORD")
	{
		eCommonSkillType = SKILL_TYPE::AUGUSTA_SWORD;
	}

	return eCommonSkillType;
}

COST_TYPE CAbility::ConvertCostType(const _string& strCostType)
{
	COST_TYPE eCostType = static_cast<COST_TYPE>(stoul(strCostType));
	return eCostType;
}

CAbility* CAbility::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAbility* pInstance = new CAbility(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CAbility");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CAbility::Clone(void* pArg)
{
	CAbility* pInstance = new CAbility(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Clone Failed : CAbility");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAbility::Free()
{
	CComponent::Free();

	m_mapSkills.clear();
	m_mapSkillChain.clear();
	m_mapSkillCooldowns.clear();

	m_CharacterInfo = {};
}              

