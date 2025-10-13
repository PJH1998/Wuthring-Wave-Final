#pragma once
#include "AnimNotify.h"

class CSoundNotify final : public CAnimNotify
{
public:
	explicit CSoundNotify(_float fTrackPosition, const _string& strTag, _float fVolume);
	void Execute() override;
	const _string& Get_NotifyTypeName() const override { return "Sound"; }
	json To_Json() const override;

public:
	static CSoundNotify* From_Json(_float fTrackPosition, const _string& strTag, _float fVolume);
	virtual void Free() override;

private:
	_string m_strSoundType; // BGM, Sound µîµî. 
	_string m_strSoundTag;
	_wstring m_wStrSoundTag;
	_float m_fVolume;


};

