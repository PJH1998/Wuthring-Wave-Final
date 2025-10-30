#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CSound_Manager final : public CBase
{
private:
	explicit CSound_Manager();
	virtual ~CSound_Manager() = default;

public:
	// Sound Load (Sound File Key, Sound File Path)
	HRESULT		Load_Sound(const _wstring& strSoundTag, const _char* pSoundFilePath);
	// Sound Load From Folder
	HRESULT		Load_Sound_FromFolder(const _char* pFolderPath);
	// ?ъ슫???ъ깮 (Sound File ??ν븷 Key, ?ъ슜??ChannelID, ?ㅼ젙??蹂쇰ⅷ 媛?
	void			Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _bool isStop);
	// BGM ?ъ깮 (Sound File ??ν븷 Key, ?ъ슜??ChannelID, ?ㅼ젙??蹂쇰ⅷ 媛? = 怨꾩냽 ?ъ깮?쒕떎
	void			Play_BGM(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _bool isStop);
	// ?⑤뒗 Channel?먯꽌 ?ъ깮
	void			Play_Other(const _wstring& strSoundTag, _float fVolume);
	// ?뱀젙 梨꾨꼸?먯꽌 ?ъ깮?섍퀬 ?덈뒗 ?ъ슫???뺤?
	void			Stop_Sound(_uint iChannelID);
	// 紐⑤뱺 ?ъ슫???뺤?
	void			Stop_All();
	// ?뱀젙 梨꾨꼸 ?ъ슫??蹂쇰ⅷ 議곗젅
	void			Set_ChannelVolume(_uint iChannelID, _float fVolume);

public:
	HRESULT		Initialize(_uint iNumChannels);
	HRESULT		Clear_Resource();

private:
	_uint													m_iNumChannels = {};

	// ?ъ슫??由ъ냼??蹂닿? 而⑦뀒?대꼫
	map<const _wstring, FMOD_SOUND*>		m_Sounds;
	// ?ъ깮?섎뒗 ?ъ슫?쒕? 愿由ы븷 媛앹껜 (?좊떦??梨꾨꼸 ??留뚰겮 諛곗뿴濡??앹꽦)
	FMOD_CHANNEL** m_pChannels = { nullptr };
	// ?ъ슫?? 梨꾨꼸 媛앹껜 諛??μ튂瑜?愿由ы븯??媛앹껜
	FMOD_SYSTEM* m_pSystem = { nullptr };

private:
	FMOD_SOUND* Find_Sound(const _wstring& strSoundTag);

public:
	static		CSound_Manager* Create(_uint iNumChannels);
	virtual		void						Free() override;
};

NS_END