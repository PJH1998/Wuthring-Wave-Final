#include "EnginePch.h"
#include "MorphChannel.h"

#include "Bone.h"

CMorphChannel::CMorphChannel()
{
}

_float CMorphChannel::Get_CurrentWeight(_float fCurrentTrackPosition)
{
	if (m_KeyFrames.empty())
		return 0.f;

	if (m_KeyFrames.size() == 1)
		return m_KeyFrames[0].fValue;

	if (fCurrentTrackPosition >= m_KeyFrames.back().fTrackPosition)
		return m_KeyFrames.back().fValue;

	// 첫 키프레임 전이면 첫 값
	if (fCurrentTrackPosition <= m_KeyFrames[0].fTrackPosition)
		return m_KeyFrames[0].fValue;

	// 현재 구간 찾기.
	auto it = std::lower_bound(m_KeyFrames.begin(), m_KeyFrames.end(), fCurrentTrackPosition,
		[](const KEYFRAME_CURVE& a, _float time) { return a.fTrackPosition < time; });

	if (it == m_KeyFrames.begin())
		return m_KeyFrames[0].fValue;
	if (it == m_KeyFrames.end())
		return m_KeyFrames.back().fValue;

	auto next = it;
	auto prev = std::prev(it);

	_float t0 = prev->fTrackPosition;
	_float t1 = next->fTrackPosition;
	_float v0 = prev->fValue;
	_float v1 = next->fValue;

	_float ratio = (fCurrentTrackPosition - t0) / (t1 - t0);
	return v0 + (v1 - v0) * ratio;  // Linear 보간
}

// 이걸 바꿔서 Mesh에 업데이트 해 주어야한다.
_float CMorphChannel::Get_CurrentWeight(_float fCurrentTrackPosition, _uint* pCurrentFrameIndex)
{
	if (0.f == fCurrentTrackPosition)
		*pCurrentFrameIndex = 0;

	if (m_KeyFrames.back().fTrackPosition <= fCurrentTrackPosition)
	{
		*pCurrentFrameIndex = m_iNumKeyFrame - 1;
	}
	else
	{
		// 4. 인덱스 캐싱 로직

#ifdef _DEBUG
		while (*pCurrentFrameIndex > 0 && m_KeyFrames[*pCurrentFrameIndex].fTrackPosition > fCurrentTrackPosition)
			--*pCurrentFrameIndex;
#endif // _DEBUG

		while (m_KeyFrames[*pCurrentFrameIndex + 1].fTrackPosition <= fCurrentTrackPosition)
			++*pCurrentFrameIndex;

		// 5. 보간(Interpolation) 계산
		_uint iNextIndex = *pCurrentFrameIndex + 1;

		// [크래시 지점 방지] 위에서 *pCurrentFrameIndex 범위를 잡았으므로 안전하게 접근 가능
		const KEYFRAME_CURVE& PrevKey = m_KeyFrames[*pCurrentFrameIndex];
		const KEYFRAME_CURVE& NextKey = m_KeyFrames[iNextIndex];
		_float t0 = PrevKey.fTrackPosition;
		_float t1 = NextKey.fTrackPosition;
		_float v0 = PrevKey.fValue;
		_float v1 = NextKey.fValue;

		if (t1 - t0 < 1e-5f)
			return v0;

		_float ratio = (fCurrentTrackPosition - t0) / (t1 - t0);

		return v0 + (v1 - v0) * ratio;
	}

	return 0.f;
}


//_float CMorphChannel::Get_CurrentWeight(_float fTrackPosition, _uint* pCurrentFrameIndex)
//{
//	if (0.f == fTrackPosition)
//		*pCurrentFrameIndex = 0;
//
//	// 1. 데이터 예외 처리
//	if (m_KeyFrames.empty())
//		return 0.f;
//
//	if (m_KeyFrames.size() <= 2)
//		return m_KeyFrames[0].fValue;
//
//	_uint iNumKeyFrames = static_cast<_uint>(m_KeyFrames.size());
//
//	// 3. 트랙 포지션이 마지막 키프레임보다 뒤에 있다면 마지막 값 반환
//	if (fTrackPosition >= m_KeyFrames.back().fTrackPosition)
//	{
//		*pCurrentFrameIndex = iNumKeyFrames - 1;
//		return m_KeyFrames.back().fValue;
//	}
//	else
//	{
//		 //4. 인덱스 캐싱 로직 (Channel과 동일한 방식)
//#ifdef _DEBUG
//	while (*pCurrentFrameIndex > 0 && m_KeyFrames[*pCurrentFrameIndex].fTrackPosition > fTrackPosition)
//		--(*pCurrentFrameIndex);
//#endif // _DEBUG
//
//
//	// (iNumKeyFrames - 2)는 인덱스 오버플로우 방지 (Current + 1을 참조해야 하므로)
//		while (*pCurrentFrameIndex < iNumKeyFrames - 1 && m_KeyFrames[*pCurrentFrameIndex + 1].fTrackPosition <= fTrackPosition)
//			++(*pCurrentFrameIndex);
//
//		// 5. 보간(Interpolation) 계산
//		_uint iNextIndex = *pCurrentFrameIndex + 1;
//
//		// 혹시 모를 인덱스 범위 체크 (안전장치)
//		if (iNextIndex >= iNumKeyFrames)
//			return m_KeyFrames.back().fValue;
//
//		const KEYFRAME_CURVE& PrevKey = m_KeyFrames[*pCurrentFrameIndex];
//		const KEYFRAME_CURVE& NextKey = m_KeyFrames[iNextIndex];
//
//		_float t0 = PrevKey.fTrackPosition;
//		_float t1 = NextKey.fTrackPosition;
//		_float v0 = PrevKey.fValue;
//		_float v1 = NextKey.fValue;
//
//		// 분모가 0이 되는 경우 방지 (시간 차이가 거의 없을 때)
//		if (t1 - t0 < 1e-5f)
//			return v0;
//
//		_float ratio = (fTrackPosition - t0) / (t1 - t0);
//
//		// Linear 보간 결과 반환
//		return v0 + (v1 - v0) * ratio;
//	}
//
//	
//
//	return 0.f;
//}

// 어떤 Shape Key인지 저장?
HRESULT CMorphChannel::Initialize(ifstream& InputFile)
{
	_uint iLength = {};

	// 1. ShapeKey 이름 로드
	InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint)); // ModelLaoder->iChannelNameLength
	InputFile.read(m_szName, iLength); // ModelLoader->ChannelName

	// 2. Key Frame 개수 로드
	InputFile.read(reinterpret_cast<_char*>(&m_iNumKeyFrame), sizeof(_uint));

	// 3. Key Frame 데이터 로드
	for (size_t i = 0; i < m_iNumKeyFrame; ++i)
	{
		KEYFRAME_CURVE KeyFrameCurve = {};
		InputFile.read(reinterpret_cast<_char*>(&KeyFrameCurve), sizeof(KEYFRAME_CURVE));
		m_KeyFrames.push_back(KeyFrameCurve);
	}

	return S_OK;
}

// Channel의 Update_TransformationMatrix를 참고.
// 1. guswo 
// 1. 모든 메쉬정보를 받아와서 ShapeKey를 탐색.
// 2. 
void CMorphChannel::Update_ShapeKeys(_float fCurrentTrackPosition, _uint* pCurrentFrameIndex, vector<_float>& InOutWeights, _int iTargetIndex)
{
	// 예외 처리: 타겟 인덱스가 유효하지 않으면 중단
	if (iTargetIndex < 0 || iTargetIndex >= InOutWeights.size())
		return;

	_float fResultWeight = 0.f;

	if (m_iNumKeyFrame == 0)
	{
		fResultWeight = 0.f;
	}
	else if (m_iNumKeyFrame == 1)
	{
		fResultWeight = m_KeyFrames[0].fValue;
	}
	else
	{
#ifdef _DEBUG
		while (m_KeyFrames[*pCurrentFrameIndex].fTrackPosition > fCurrentTrackPosition)
			--*pCurrentFrameIndex;
#endif // _DEBUG
	}
}




CMorphChannel* CMorphChannel::Create(ifstream& InputFile)
{
	CMorphChannel* pInstance = new CMorphChannel();

	if (FAILED(pInstance->Initialize(InputFile)))
	{
		MSG_BOX("Failed to Create : Morph Channel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMorphChannel::Free()
{
	__super::Free();
}
