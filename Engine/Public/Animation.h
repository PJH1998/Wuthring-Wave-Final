#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CAnimation final : public CBase
{
private:
	explicit CAnimation();
	explicit CAnimation(const CAnimation& Prototype);
	virtual ~CAnimation() = default;

public:
	const _char*		Get_Name() { return m_szName; }
	void				Set_CurrentTrackPosition(_float fTrackPos) { m_fCurrentTrackPosition = fTrackPos; m_iNotifyIndex = 0; }

#ifdef _DEBUG
	_float*			Get_TrackPositionPtr() { return &m_fCurrentTrackPosition; }
	_float				Get_Duration() { return m_fDuration; }
#endif // _DEBUG

public:
	void				Register_Notify(const NOTIFY& AnimNotify); // 기존 것

	void				Load_Notify(const json& notifyJson, function<void(const _wstring&, _bool)> ColliderCallback, function<void()> EffectCallback);
	

	void				Sort_Notify();
	void				Sort_AnimNotify();

public:
	HRESULT			Initialize(ifstream& InputFile, const vector<class CBone*>& Bones);
	_bool				Update_TransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition = nullptr);
	_bool				Update_TransformationMatrices_All(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition = nullptr);
	_bool				Blend_TransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float fTrackLength);
private:
	_char									m_szName[MAX_PATH] = {};
	_float									m_fDuration = {};
	_float									m_fTickPerSecond = {};
	_float									m_fCurrentTrackPosition = {};

	_uint									m_iNumChannels = {};
	vector<class CChannel*>		m_Channels;
	vector<_uint>						m_CurrentFrameIndices;

	_uint									m_iNotifyIndex = {};
	vector<NOTIFY>					m_Notifies;


	// 신규 Notify 기존것은 혹시 모를 호환성을 위해 냅둬둠.
	vector<class CAnimNotify*> m_AnimNotifies;

public:
	static CAnimation* Create(ifstream& InputFile, const vector<class CBone*>& Bones);
	CAnimation* Clone();
	virtual void Free() override;
};

NS_END