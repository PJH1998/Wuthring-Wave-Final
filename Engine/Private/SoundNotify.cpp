#include "EnginePch.h"
#include "SoundNotify.h"
#include "GameInstance.h"

CSoundNotify::CSoundNotify(_float fTrackPosition, const _string& strTag, _float fVolume)
	: CAnimNotify{ fTrackPosition }
	, m_strSoundTag { strTag }
	, m_fVolume { fVolume }
{

}

void CSoundNotify::Execute()
{
	if ("BGM" == m_strSoundTag)
	{
		//m_pGameInstance->Play_BGM(m_strSoundTag, 0, m_fVolume);
	}
}

json CSoundNotify::To_Json() const
{
	json soundJson;
	soundJson["NotifyType"] = "Sound";
	soundJson["SoundType"] = m_strSoundType;
	soundJson["TrackPosition"] = m_fTrackPosition;
	soundJson["SoundTag"] = m_strSoundTag;
	soundJson["Volume"] = m_fVolume;
	return soundJson;
}

CSoundNotify* CSoundNotify::From_Json(_float fTrackPosition, const _string& strTag, _float fVolume)
{
	return new CSoundNotify(fTrackPosition, strTag, fVolume);
}

void CSoundNotify::Free()
{
	CAnimNotify::Free();
}




