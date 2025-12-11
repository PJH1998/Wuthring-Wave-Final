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
		if(!m_IsCurBattleBGMChange)
		{
			if (m_fBGMRate > 0.f)
				m_fBGMRate -= fTimeDelta * 0.7f;
		}
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
			m_pGameInstance->Stop_Sound(ENUM_CLASS(CHANNEL::BGM));
			m_pGameInstance->Play_BGM(m_szChangeBGMName, ENUM_CLASS(CHANNEL::BGM), m_fBGMRate);
			m_fBGMRate = 0.f;
			m_IsCurBGMChange = !m_IsCurBGMChange;
		}
	}
	else if (m_IsCurBattleBGMChange)
	{
		if (m_fBGMRate < 1.f)
			m_fBGMRate += fTimeDelta * 0.7f;
		else if (m_fBGMRate >= 1.f)
		{
			m_pGameInstance->Stop_Sound(ENUM_CLASS(CHANNEL::BATTLE_BGM));
			m_pGameInstance->Play_BGM(m_szChangeBattleBGMName, ENUM_CLASS(CHANNEL::BATTLE_BGM), m_fBGMRate);
			m_fBGMRate = 1.f;
			m_IsCurBattleBGMChange = !m_IsCurBattleBGMChange;
		}
	}
	
}

void CBGM_Manager::Change_Level(_uint iLevel)
{
	switch(iLevel)
	{
	case ENUM_CLASS(LEVEL::GAMEPLAY):
		m_pGameInstance->Play_BGM(TEXT("music_scene_septimont_aitongyuan_poi-after_cm_75bpm_4_4 (SFX)"), ENUM_CLASS(CHANNEL::BGM), m_fBGMRate);
		//마을 들어가면 music_scene_qiqiu_pingyuan_night (SFX) 로
		break;
	case ENUM_CLASS(LEVEL::HEAVEN):
		m_pGameInstance->Play_BGM(TEXT("Journeying Paradise"), ENUM_CLASS(CHANNEL::BGM), m_fBGMRate);
		break;
	}
}

void CBGM_Manager::Ready_BGM()
{
	//BGM 폴더 뒤지면서 이름 수집. 걍 하드로 할까요
#pragma region LEVIATHAN
	m_BGMs[BOSSBGM::HEAVEN_INTRO] = TEXT("battle_music_boss_wuguanzhe_intro (SFX)");
	m_BGMs[BOSSBGM::HEAVEN_ONE] = TEXT("battle_music_boss_wuguidemiuwu_stage1 (SFX)");
	m_BGMs[BOSSBGM::HEAVEN_CHNAGE] = TEXT("battle_music_boss_wuguidemiuwu_transition (SFX)");
	m_BGMs[BOSSBGM::HEAVEN_TWO] = TEXT("battle_music_boss_wuguidemiuwu_stage2 (SFX)");
#pragma endregion

#pragma region ASPHODEL_BARRENS	
	m_BGMs[BOSSBGM::ASPHODEL] = TEXT("battle_outside_monster_small_loop_strong_v2 (SFX)");
#pragma endregion

#pragma region SOERVERIGN
	m_BGMs[BOSSBGM::SOERVERIGN] = TEXT("battle_music_boss_shenwang2_bpm170_4-4_ingame (SFX)");
#pragma endregion

#pragma region DEFAULT
	m_BattleBGM = TEXT("battle_music_normal_qiqiu_bpm130_4-4 (SFX)");
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

	//현재 같은 채널에 노래 2개 재생하려고 하니까 소리가 겹침.ㅅ발 그러고 안꺼짐.
	//그냥 브금을 저거로 바꿔버려?
	//잡몹전투일 때 
	if (m_eLastBattle != BOSSBGM::END)
	{
		if (m_eLastBattle == BOSSBGM::MODINARY && eBossLevel == BOSSBGM::END)
		{
			m_IsBattle = IsBattle;
			m_eLastBattle = eBossLevel;
			return;
		}
		if (m_eLastBattle != eBossLevel)
			return;
	}
	m_IsBattle = IsBattle;
	m_eLastBattle = eBossLevel;
	if (eBossLevel == BOSSBGM::MODINARY)
		m_pGameInstance->Play_BGM(m_BattleBGM, ENUM_CLASS(CHANNEL::BATTLE_BGM), 0.f);
	else
		m_pGameInstance->Play_BGM(m_BGMs[eBossLevel], ENUM_CLASS(CHANNEL::BATTLE_BGM), 0.f);

	if (!IsBattle)
		m_eLastBattle = BOSSBGM::END;
}

void CBGM_Manager::Change_BGM(const _wstring& BGMText)
{
	m_szChangeBGMName = BGMText;
	m_IsCurBGMChange = true;
}

void CBGM_Manager::Change_BattleBGM(BOSSBGM eBoss)
{
	m_szChangeBattleBGMName = m_BGMs[eBoss];
	m_IsCurBattleBGMChange = true;
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
