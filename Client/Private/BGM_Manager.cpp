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
	m_fBGMRate = 1.f;
	return S_OK;
}

void CBGM_Manager::Update(_float fTimeDelta)
{
	if (m_IsBattle)
	{
		if (m_fBGMRate > 0.f)
			m_fBGMRate -= fTimeDelta * 0.7f;
	}
	else
	{
		if (!m_IsCurBGMChange)
		{
			if (m_fBGMRate < 1.f)
				m_fBGMRate += fTimeDelta * 0.7f;
			else
				m_pGameInstance->Stop_Sound(ENUM_CLASS(CHANNEL::BATTLE_BGM));
		}
	}
	if (m_fBGMRate > 1.f)
		m_fBGMRate = 1.f;
	else if (m_fBGMRate < 0.f)
		m_fBGMRate = 0.f;

	m_pGameInstance->Set_ChannelVolume(ENUM_CLASS(CHANNEL::BGM), m_fBGMRate);

	if (m_fBGMRate < 1.f)
		m_pGameInstance->Set_ChannelVolume(ENUM_CLASS(CHANNEL::BATTLE_BGM), 1.f - m_fBGMRate);


	if (m_IsCurBGMChange)
	{
		if (m_fBGMRate > 0.f)
			m_fBGMRate -= fTimeDelta * 0.7f;
		else if (m_fBGMRate <= 0.f)
		{
			m_pGameInstance->Play_BGM(m_szChangeBGMName, ENUM_CLASS(CHANNEL::BGM), m_fBGMRate);
			m_fBGMRate = 0.f;
			m_IsCurBGMChange = !m_IsCurBGMChange;
		}
	}
}

void CBGM_Manager::Change_Level(_uint iLevel)
{
	switch(iLevel)
	{
	case ENUM_CLASS(LEVEL::GAMEPLAY):
		m_pGameInstance->Play_BGM(TEXT("music_scene_qiqiu_pingyuan_night (SFX)"), ENUM_CLASS(CHANNEL::BGM), m_fBGMRate);
		break;
	case ENUM_CLASS(LEVEL::HEAVEN):
		//m_pGameInstance->Play_BGM(m_CurBGM, ENUM_CLASS(CHANNEL::BGM), m_fBGMRate);
		break;
	}
}

void CBGM_Manager::Ready_BGM()
{
	//BGM 폴더 뒤지면서 이름 수집. 걍 하드로 할까요
#pragma region LEVIATHAN
	m_BGMs[BOSSBGM::HEAVEN_ONE] = TEXT("battle_music_boss_wuguidemiuwu_stage1 (SFX)");
	m_BGMs[BOSSBGM::HEAVEN_CHNAGE] = TEXT("battle_music_boss_wuguidemiuwu_transition (SFX)");
	m_BGMs[BOSSBGM::HEAVEN_TWO] = TEXT("battle_music_boss_wuguidemiuwu_stage2 (SFX)");
#pragma endregion

#pragma region ASPHODEL_BARRENS
	m_BGMs[BOSSBGM::ASPHODEL] = TEXT("battle_music_boss_wuguidemiuwu_stage2 (SFX)");
#pragma endregion

#pragma region SOERVERIGN
	m_BGMs[BOSSBGM::SOERVERIGN] = TEXT("battle_music_boss_wuguidemiuwu_stage2 (SFX)");
#pragma endregion

#pragma region DEFAULT
	m_BattleBGM = TEXT("battle_music_boss_wuguidemiuwu_stage2 (SFX)");
#pragma endregion
}

void CBGM_Manager::Stop_BGM()
{
	m_pGameInstance->Stop_Sound(ENUM_CLASS(CHANNEL::BGM));
	m_pGameInstance->Stop_Sound(ENUM_CLASS(CHANNEL::BATTLE_BGM));
}

void CBGM_Manager::Engage_Battle(_bool IsBattle, BOSSBGM eBossLevel)
{
	//잡몹 전투가 아니면 플레이어가 끌 수 있게. -> 최근 전투 판정 나온 게 잡몹이냐 판정. 소리 꺼달라는 채널이 최근 전투와 같냐 판정
	if (m_eLastBattle != BOSSBGM::END)
	{
		if (m_eLastBattle != eBossLevel)
			return;
	}
	m_IsBattle = IsBattle;
	m_eLastBattle = eBossLevel;
	if (eBossLevel == BOSSBGM::END)
		m_pGameInstance->Play_BGM(m_BattleBGM, ENUM_CLASS(CHANNEL::BATTLE_BGM), 0.f);
	else
		m_pGameInstance->Play_BGM(m_BossBGM[eBossLevel], ENUM_CLASS(CHANNEL::BATTLE_BGM), 0.f);
}

void CBGM_Manager::Change_BGM(const _wstring& BGMText)
{
	m_szChangeBGMName = BGMText;
	m_IsCurBGMChange = true;
}

CBGM_Manager* CBGM_Manager::Create()
{
	CBGM_Manager* m_pInstance = new CBGM_Manager();
	m_pInstance->Initialize();

	return m_pInstance;
}

void CBGM_Manager::Free()
{
	__super::Free();

	for (auto& Sound : m_BGMs)
		Sound.second.clear();
	m_BGMs.clear();

	Safe_Release(m_pGameInstance);

}
