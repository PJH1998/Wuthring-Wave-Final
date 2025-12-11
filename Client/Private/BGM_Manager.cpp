#include"ClientPch.h"
#include "BGM_Manager.h"
#include"GameInstance.h"
CBGM_Manager::CBGM_Manager()
	:m_pGameInstance(CGameInstance::GetInstance())
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CBGM_Manager::Initialize()
{
	Ready_BGM();


	//레벨별로 브금 이름 알아서 설정.
	m_pGameInstance->Play_BGM(m_CurBGM, ENUM_CLASS(CHANNEL::BGM), m_fBGMRate);
	return S_OK;
}

void CBGM_Manager::Update(_float fTimeDelta)
{
	m_fLerpTime += fTimeDelta;
	if (m_IsBattle)
	{
	}
	else
	{

	}
	if (m_IsCurBGMChange)
	{

	}
	m_pGameInstance->Set_ChannelVolume(ENUM_CLASS(CHANNEL::BGM), m_fBGMRate);
	if (m_IsBattle)
		m_pGameInstance->Play_BGM(m_BattleBGM, ENUM_CLASS(CHANNEL::BATTLE_BGM), 1.f - m_fBGMRate);
}

void CBGM_Manager::Ready_BGM()
{

}

void CBGM_Manager::Change_BGM(_bool IsBattle)
{
	m_IsBattle = IsBattle;
	m_fLerpTime = 0.f;
}

void CBGM_Manager::Free()
{
	__super::Free();

	for (auto& Sound : m_BGMs)
		Sound.second.clear();
	m_BGMs.clear();

	Safe_Release(m_pGameInstance);

}
