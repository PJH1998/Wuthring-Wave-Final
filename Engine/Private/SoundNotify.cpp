#include "EnginePch.h"
#include "GameInstance.h"
#include "SoundNotify.h"

CSoundNotify::CSoundNotify(_float fTrackPosition, const _string& strTag, const _string& strSoundType, _float fVolume)
	: CAnimNotify{ fTrackPosition}
	, m_strSoundTag { strTag }
	, m_strSoundType { strSoundType }
	, m_fVolume { fVolume }
	, m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
	m_wStrSoundTag = StringToWString(m_strSoundTag);
}

void CSoundNotify::Execute()
{
	static _uint iChannel = 0;

	iChannel = iChannel % 31;
	if ("Effect" == m_strSoundType)
		m_pGameInstance->Play_Sound(m_wStrSoundTag, iChannel++, m_fVolume); // 일단 채널 임시.
		//m_pGameInstance->Play_Other(m_wStrSoundTag, m_fVolume); // 일단 채널 임시.

}

json CSoundNotify::To_Json() const
{
	json soundJson;
	soundJson["NotifyType"] = "Sound"; // 읽어올 때 구분을 위해?
	soundJson["TrackPosition"] = m_fTrackPosition;
	soundJson["SoundTag"] = m_strSoundTag;
	soundJson["SoundType"] = m_strSoundType;
	soundJson["Volume"] = m_fVolume;
	return soundJson;
}

const _string& CSoundNotify::Get_NotifyTypeName() const
{
	// 노티파이는 여러개 생성되는데 같은 typeName이므로 공유한다.
	static const _string typeName = "Sound";
	return typeName;
}

#ifdef _DEBUG
// ImGui용도 출력
void CSoundNotify::ImGui_Print()
{
	ImGui::Text("TrackPosition : %.2f", m_fTrackPosition);
	ImGui::Text("Volume : %.2f", m_fVolume);
	ImGui::Text("Sound Tag : %s", m_strSoundTag.c_str());
	ImGui::Text("Sound Type : %s", m_strSoundType.c_str());
}
#endif 



CSoundNotify* CSoundNotify::From_Json(const json& soundJson)
{
	return new CSoundNotify(
		soundJson["TrackPosition"],
		soundJson["SoundTag"],
		soundJson["SoundType"],
		soundJson["Volume"]
	);
}

void CSoundNotify::Free()
{
	CAnimNotify::Free();
	Safe_Release(m_pGameInstance);
}




