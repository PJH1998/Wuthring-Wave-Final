#include "ClientPch.h"
#include "PlayerStatus.h"
#include "Ability.h"
#include "Player.h"
#include "GameInstance.h"
#include "GameSystem.h"

CPlayerStatus::CPlayerStatus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pGameInstance { CGameInstance::GetInstance() }
	, m_pDevice { pDevice }
	, m_pContext { pContext }
{
	m_Abilities.resize(CPlayer::CHARACTERTYPE::TYPE_END);

	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}


HRESULT CPlayerStatus::Initialize(const vector<_string>& AbilityFolders)
{
	// 1. Ability Component Clone.
	
	//for (_uint i = CPlayer::CHARACTERTYPE::ROVER; i < CPlayer::CHARACTERTYPE::TYPE_END; ++i)
	for (_uint i = CPlayer::CHARACTERTYPE::ROVER; i < CPlayer::CHARACTERTYPE::GALBRENA; ++i) // 갈브가 없음 아직.
	{
		// 2. Create 하기
		m_Abilities[i] = CAbility::Create(m_pDevice, m_pContext); 
		
		// 3. 유효하지 않으면 Crash
		ASSERT_CRASH(m_Abilities[i]);
		
		// 4. File .csv 등록
		m_Abilities[i]->Register_AllAbilityFiles(AbilityFolders[i]);
	}


    return S_OK;
}

void CPlayerStatus::Update(_float fTimeDelta)
{
	for (_uint i = CPlayer::CHARACTERTYPE::AUGUSTA; i < CPlayer::CHARACTERTYPE::TYPE_END; ++i)
	{
		// 비활성화된 캐릭터들의 쿨타임도 초기화.
		if (nullptr != m_Abilities[i])
			m_Abilities[i]->Update(fTimeDelta);
	}

}

_float CPlayerStatus::Get_HpRatio(_uint iCharIndex) const
{
	if (nullptr == m_Abilities[iCharIndex])
		return 0.f;

    return m_Abilities[iCharIndex]->Get_HpRatio();
}

_float CPlayerStatus::Get_Cost(_uint iCharIndex, COST_TYPE eType) const
{
	if (nullptr == m_Abilities[iCharIndex])
		return 0.f;

	return m_Abilities[iCharIndex]->Get_Cost(eType);
}

_float CPlayerStatus::Get_RemainingCooldown(_uint iCharIndex, const _string& strSkillName) const
{
	if (nullptr == m_Abilities[iCharIndex])
		return 0.f;

	return m_Abilities[iCharIndex]->Get_RemainingCooldown(strSkillName);
}

_float CPlayerStatus::Get_MaxCooldown(_uint iCharIndex, const _string& strSkillName) const
{
	if (nullptr == m_Abilities[iCharIndex])
		return 0.f;

	return m_Abilities[iCharIndex]->Get_MaxCooldown(strSkillName);
}

CAbility* CPlayerStatus::Get_Ability(_uint iCharIndex) const
{
	return m_Abilities[iCharIndex];
}




CPlayerStatus* CPlayerStatus::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const vector<_string>& AbilityFolders)
{
	CPlayerStatus* pInstance = new CPlayerStatus(pDevice, pContext);
	if (FAILED(pInstance->Initialize(AbilityFolders)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Create Failed Player Status");
	}

	return pInstance;
}

void CPlayerStatus::Free()
{
	CBase::Free();
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	for (auto& pAbility : m_Abilities)
		Safe_Release(pAbility);
	m_Abilities.clear();

}
