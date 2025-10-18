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
	const vector<class CChannel*>& Get_Channels() const { return m_Channels; }
	void				Set_CurrentTrackPosition(_float fTrackPos) { m_fCurrentTrackPosition = fTrackPos; m_iNotifyIndex = 0; }

#ifdef _DEBUG
	_float*				Get_TrackPositionPtr() { return &m_fCurrentTrackPosition; }
	_float				Get_Duration() { return m_fDuration; }
#endif // _DEBUG

public:
	void				Register_Notify(const NOTIFY& AnimNotify); // 湲곗〈 寃?

	void				Load_Notify(const json& notifyJson, function<void(const _wstring&, _bool)> ColliderCallback, function<void()> EffectCallback);
	

	void				Sort_Notify();
	void				Sort_AnimNotify();

public:
	HRESULT			Initialize(ifstream& InputFile, const vector<class CBone*>& Bones);
	_bool				Update_TransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition = nullptr);
	_bool				Update_RibTransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition = nullptr);

	_bool				Update_TransformationMatrices_All(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition = nullptr);

	_bool				Blend_TransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float fTrackLength);

	_bool Update_TrackPosition(_float fTimeDelta, _float* pTrackPosition);
private:
	_char									m_szName[MAX_PATH] = {};
	_float									m_fDuration = {};
	_float									m_fTickPerSecond = {};
	_float									m_fCurrentTrackPosition = {};

	_uint									m_iNumChannels = {};
	vector<class CChannel*>				m_Channels;
	vector<_uint>						m_CurrentFrameIndices;

	vector<NOTIFY>					m_Notifies; // ?명솚?깆쓣 ?꾪빐 ?대젮??

	_uint							m_iNotifyIndex = {};
	
	// ?좉퇋 Notify 
	vector<class CAnimNotify*> m_AnimNotifies;

private:

public:
	static CAnimation* Create(ifstream& InputFile, const vector<class CBone*>& Bones);
	CAnimation* Clone();
	virtual void Free() override;
};

NS_END