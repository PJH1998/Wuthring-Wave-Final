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
		m_pGameInstance->Play_Sound(m_wStrSoundTag, iChannel++, m_fVolume);

}

json CSoundNotify::To_Json() const
{
	json soundJson;
	soundJson["NotifyType"] = "Sound"; // ?쎌뼱????援щ텇???꾪빐?
	soundJson["TrackPosition"] = m_fTrackPosition;
	soundJson["SoundTag"] = m_strSoundTag;
	soundJson["SoundType"] = m_strSoundType;
	soundJson["Volume"] = m_fVolume;
	return soundJson;
}

const _string& CSoundNotify::Get_NotifyTypeName() const
{
	// ?명떚?뚯씠???щ윭媛??앹꽦?섎뒗??媛숈? typeName?대?濡?怨듭쑀?쒕떎.
	static const _string typeName = "Sound";
	return typeName;
}

#ifdef _DEBUG
// ImGui?⑸룄 異쒕젰
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




