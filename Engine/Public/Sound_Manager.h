#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CSound_Manager final : public CBase
{
private:
	explicit CSound_Manager();
	virtual ~CSound_Manager() = default;

public:
	// 사운드 파일 Load (Sound File 저장할 Key, Sound File 경로)
	HRESULT		Load_Sound(const _wstring& strSoundTag, const char* pSoundFilePath);
	// 사운드 재생 (Sound File 저장할 Key, 사용할 ChannelID, 설정할 볼륨 값)
	void			Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _bool isStop);
	// BGM 재생 (Sound File 저장할 Key, 사용할 ChannelID, 설정할 볼륨 값) = 계속 재생한다
	void			Play_BGM(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _bool isStop);
	// 남는 Channel에서 재생
	void			Play_Other(const _wstring& strSoundTag, _float fVolume);
	// 특정 채널에서 재생하고 있는 사운드 정지
	void			Stop_Sound(_uint iChannelID);
	// 모든 사운드 정지
	void			Stop_All();
	// 특정 채널 사운드 볼륨 조절
	void			Set_ChannelVolume(_uint iChannelID, _float fVolume);

public:
	HRESULT		Initialize(_uint iNumChannels);
	HRESULT		Clear_Resource();

private:
	_uint													m_iNumChannels = {};

	// 사운드 리소스 보관 컨테이너
	map<const _wstring, FMOD_SOUND*>		m_Sounds;
	// 재생하는 사운드를 관리할 객체 (할당된 채널 수 만큼 배열로 생성)
	FMOD_CHANNEL** m_pChannels = { nullptr };
	// 사운드, 채널 객체 및 장치를 관리하는 객체
	FMOD_SYSTEM* m_pSystem = { nullptr };

private:
	FMOD_SOUND* Find_Sound(const _wstring& strSoundTag);

public:
	static		CSound_Manager* Create(_uint iNumChannels);
	virtual		void						Free() override;
};

NS_END