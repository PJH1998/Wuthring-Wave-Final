#include "ClientPch.h"
#include "Ability.h"
#include "GameSystem.h"
#include "GameInstance.h"

CAbility::CAbility(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
	, m_pGameSystem { CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

CAbility::CAbility(const CAbility& Prototype)
	: CComponent ( Prototype)
	, m_pGameSystem { Prototype.m_pGameSystem }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CAbility::Initialize_Prototype()
{
	if (FAILED(CComponent::Initialize_Prototype()))
		return E_FAIL;
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
			iter = m_mapSkillCooldowns.erase(iter);
		else
			++iter;
	}

	// 매프레임 Stamina 자동 회복. // 초당 10
	Add_Cost(COST_TYPE::STAMINA, fTimeDelta * 10.f);
}

void CAbility::Register_AllAbilityFiles(const _string& strFolderPath)
{
	_string skillPath = strFolderPath + "Skill.csv";
	_string statPath = strFolderPath + "Stat.csv";

	Read_Skill(skillPath.c_str());
	Read_Stat(statPath.c_str());
}



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
		m_Costs[ENUM_CLASS(pSkillInfo->eCostType)] += pSkillInfo->fCost;
		break;
	}

	// 5. 쿨타임 적용 (쿨타임이 0초보다 클 경우)
	if (pSkillInfo->fCoolDown > 0.f)
		m_mapSkillCooldowns.emplace(strSkillName , pSkillInfo->fCoolDown); // 쿨타임이 0초보다 큰 경우에만 쿨타임 적용.

	m_strPrevSkillName = strSkillName;

	// 6. 스킬 사용 성공 (READY 상태였음)
	return SKILL_STATE::READY;
}

void CAbility::Set_Cost(COST_TYPE eType, _float fValue)
{
	_uint iType = ENUM_CLASS(eType);

	if (iType >= m_Costs.size())
		return;

	m_Costs[iType] = min(fValue, m_fCostMax);
}

void CAbility::Add_Cost(COST_TYPE eType, _float fValue)
{
	_uint iType = ENUM_CLASS(eType);

	if (iType >= m_Costs.size())
		return;

	m_Costs[iType] = max(m_Costs[iType] + fValue, m_fCostMax);
}

#ifdef _DEBUG
void CAbility::Debug_FullCost()
{
	_uint iStart = ENUM_CLASS(COST_TYPE::COST1);
	_uint iEnd = ENUM_CLASS(COST_TYPE::COST_TYPE_END);
	for (_uint i = 1; i < iEnd; ++i)
		m_Costs[i] = m_fCostMax;
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
	vector<vector<_string>> data = m_pGameSystem->Load_CSV(pFilePath);

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
		eSkill.eSkillType = ConvertSkillType(data[i][2]);
		eSkill.eCostType = ConvertCostType(data[i][3]);
		eSkill.fCost = stof(data[i][4]);
		
		eSkill.fCoolDown = stof(data[i][5]);
		eSkill.strKeyInput = data[i][6];
		eSkill.fDamage = stof(data[i][7]);

		// 3. 메인 스킬 맵에 저장
		m_mapSkills.emplace(eSkill.strSkillName, eSkill);

		// 4. 스킬 체인 맵에 저장 (PrevName이 "None"이 아닐 경우)
		if (eSkill.strPrevName != "None" && !eSkill.strPrevName.empty())
			m_mapSkillChain.emplace(eSkill.strPrevName, eSkill.strSkillName);

	}
}

void CAbility::Read_Stat(const _char* pFilePath)
{
	vector<vector<_string>> data = m_pGameSystem->Load_CSV(pFilePath);

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

	m_CharacterInfo.fAttack = stof(data[1][9]);
	m_CharacterInfo.fAttackAddMin = stof(data[1][10]);
	m_CharacterInfo.fAttackAddMax = stof(data[1][11]);
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
	Safe_Release(m_pGameSystem);

	m_mapSkills.clear();
	m_mapSkillChain.clear();
	m_mapSkillCooldowns.clear();

	m_CharacterInfo = {};
}
