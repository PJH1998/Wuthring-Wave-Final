#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CMorphChannel final : public CBase
{
private:
	explicit CMorphChannel();
	virtual ~CMorphChannel() = default;

public:
	const vector<KEYFRAME_CURVE>& Get_KeyframeCurves() const { return m_KeyFrames; }
	_uint Get_NumKeyframes() const { return m_iNumKeyFrame; }

	_float Get_CurrentWeight(_float fTimeAcc); // 현재 시간에 맞는 가중치(Weight) 반환
#ifdef _DEBUG
public:
	const _char* Get_Name() const { return m_szName; }
#endif // _DEBUG



public:
	HRESULT					Initialize(ifstream& InputFile);
	void					Update_ShapeKeys(_float fCurrentTrackPosition, _uint* pCurrentFrameIndex, vector<_float>& InOutWeights, _int iTargetIndex);

private:
	_char						m_szName[MAX_PATH] = {};
	_uint						m_iNumKeyFrame = {};
	vector<KEYFRAME_CURVE>		m_KeyFrames;

	_uint						m_iCurrentKeyFrameIndex = {}; // 현재 키프레임 캐싱.

public:
	static		CMorphChannel*	Create(ifstream& InputFile);
	virtual		void			Free() override;
};

NS_END