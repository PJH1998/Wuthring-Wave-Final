#include "EnginePch.h"
#include "Sound_Manager.h"

CSound_Manager::CSound_Manager()
{
}

HRESULT CSound_Manager::Load_Sound(const _wstring& strSoundTag, const char* pSoundFilePath)
{
    FMOD_SOUND* pSound = Find_Sound(strSoundTag);

    if (nullptr != pSound)
        return S_OK;

    FMOD_MODE mode = FMOD_DEFAULT | FMOD_CREATESAMPLE;
    FMOD_RESULT eResult = FMOD_System_CreateSound(m_pSystem, pSoundFilePath, mode, 0, &pSound);

    if (FMOD_OK == eResult)
        m_Sounds.emplace(strSoundTag, pSound);
    else
    {
        MSG_BOX("Failed to Load : Sound File");
        return E_FAIL;
    }

    FMOD_System_Update(m_pSystem);

    return S_OK;
}

void CSound_Manager::Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _bool isStop)
{
    FMOD_SOUND* pSound = Find_Sound(strSoundTag);
    if (nullptr == pSound)
        return;

    FMOD_BOOL isPlay = false;

	if (FMOD_Channel_IsPlaying(m_pChannels[iChannelID], &isPlay))
	{
		FMOD_System_PlaySound(m_pSystem, pSound, nullptr, false, &m_pChannels[iChannelID]);
	}
	else if (true == isStop)
	{
		Stop_Sound(iChannelID);
		FMOD_System_PlaySound(m_pSystem, pSound, nullptr, false, &m_pChannels[iChannelID]);
	}

    FMOD_Channel_SetVolume(m_pChannels[iChannelID], fVolume);
    FMOD_System_Update(m_pSystem);
}

void CSound_Manager::Play_BGM(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _bool isStop)
{
    FMOD_SOUND* pSound = Find_Sound(strSoundTag);
    if (nullptr == pSound)
        return;

	FMOD_BOOL isPlay = false;

	if (true == isStop)
		Stop_Sound(iChannelID);
	else
	{
		FMOD_Channel_IsPlaying(m_pChannels[iChannelID], &isPlay);
		if (true == isPlay)
			return;
	}

    FMOD_System_PlaySound(m_pSystem, pSound, nullptr, false, &m_pChannels[iChannelID]);
    FMOD_Channel_SetMode(m_pChannels[iChannelID], FMOD_LOOP_NORMAL);
    FMOD_Channel_SetVolume(m_pChannels[iChannelID], fVolume);
    FMOD_System_Update(m_pSystem);
}

void CSound_Manager::Play_Other(const _wstring& strSoundTag, _float fVolume)
{
	FMOD_SOUND* pSound = Find_Sound(strSoundTag);
	if (nullptr == pSound)
		return;

	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		FMOD_BOOL isPlay = false;
		if (FMOD_Channel_IsPlaying(m_pChannels[i], &isPlay))
		{
			cout << i << endl;
			FMOD_System_PlaySound(m_pSystem, pSound, nullptr, false, &m_pChannels[i]);
			FMOD_Channel_SetVolume(m_pChannels[i], fVolume);
			break;
		}
	}
}

void CSound_Manager::Stop_Sound(_uint iChannelID)
{
    FMOD_Channel_Stop(m_pChannels[iChannelID]);
    FMOD_System_Update(m_pSystem);
}

void CSound_Manager::Stop_All()
{
    for (size_t i = 0; i < m_iNumChannels; ++i)
    {
        if (nullptr != m_pChannels[i])
            FMOD_Channel_Stop(m_pChannels[i]);
    }
    FMOD_System_Update(m_pSystem);
}

void CSound_Manager::Set_ChannelVolume(_uint iChannelID, _float fVolume)
{
    FMOD_Channel_SetVolume(m_pChannels[iChannelID], fVolume);

    FMOD_System_Update(m_pSystem);
}

HRESULT CSound_Manager::Initialize(_uint iNumChannels)
{
    m_iNumChannels = iNumChannels;
    m_pChannels = new FMOD_CHANNEL * [m_iNumChannels] {nullptr};

    // ?ъ슫???대떦 ???媛앹껜 ?앹꽦
    FMOD_System_Create(&m_pSystem, FMOD_VERSION);

    // 1. ?쒖뒪???ъ씤??/ 2. ?ъ슜??媛?곸콈????/ 3. 珥덇린??諛⑹떇
    FMOD_System_Init(m_pSystem, 32, FMOD_INIT_NORMAL, NULL);

    return S_OK;
}

HRESULT CSound_Manager::Clear_Resource()
{
	Stop_All();
	for (auto& Pair : m_Sounds)
		FMOD_Sound_Release(Pair.second);
	m_Sounds.clear();

	return S_OK;
}

FMOD_SOUND* CSound_Manager::Find_Sound(const _wstring& strSoundTag)
{
    auto iter = m_Sounds.find(strSoundTag);

    if (iter == m_Sounds.end())
        return nullptr;

    return iter->second;
}

CSound_Manager* CSound_Manager::Create(_uint iNumChannels)
{
    CSound_Manager* pInstance = new CSound_Manager();

    if (FAILED(pInstance->Initialize(iNumChannels)))
    {
        MSG_BOX("Failed To Create : Sound_Manager");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSound_Manager::Free()
{
    __super::Free();
	
	Stop_All();
    for (auto& Pair : m_Sounds)
        FMOD_Sound_Release(Pair.second);
    m_Sounds.clear();

    Safe_Delete_Array(m_pChannels);

    FMOD_System_Close(m_pSystem);
    FMOD_System_Release(m_pSystem);
}
