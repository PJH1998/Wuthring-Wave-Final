#include "EnginePch.h"
#include "MorphChannel.h"

#include "Bone.h"

CMorphChannel::CMorphChannel()
{
}

_float CMorphChannel::Get_CurrentWeight(_float fTimeAcc)
{
	return _float();
}

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
