#include "EnginePch.h"
#include "Animation.h"

#include "Channel.h"

CAnimation::CAnimation()
{
}

CAnimation::CAnimation(const CAnimation& Prototype)
	: m_fDuration { Prototype.m_fDuration },
	m_fTickPerSecond { Prototype.m_fTickPerSecond },
	m_fCurrentTrackPosition { Prototype.m_fCurrentTrackPosition },
	m_iNumChannels { Prototype.m_iNumChannels },
	m_Channels { Prototype.m_Channels },
	m_CurrentFrameIndices{ Prototype.m_CurrentFrameIndices }
{
	strcpy_s(m_szName, Prototype.m_szName);

	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);

	
}

void CAnimation::Register_Notify(const NOTIFY& AnimNotify)
{
	m_Notifies.push_back(AnimNotify);
}

void CAnimation::Sort_Notify()
{
	if (0 == m_Notifies.size())
		return;

	sort(m_Notifies.begin(), m_Notifies.end(), [](const NOTIFY& Src, const NOTIFY& Dst)->_bool {
			return Src.fTrackPosition < Dst.fTrackPosition ? true : false;
		});
}

HRESULT CAnimation::Initialize(ifstream& InputFile, const vector<class CBone*>& Bones)
{
	_uint iLength = {};
	InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
	_char szAnimationName[MAX_PATH] = {};
	InputFile.read(szAnimationName, iLength);

	_char* pAnimationName = { nullptr };
	strtok_s(szAnimationName, "|", &pAnimationName);
	strcpy_s(m_szName, pAnimationName);

	InputFile.read(reinterpret_cast<_char*>(&m_fDuration), sizeof(_float));
	InputFile.read(reinterpret_cast<_char*>(&m_fTickPerSecond), sizeof(_float));

	InputFile.read(reinterpret_cast<_char*>(&m_iNumChannels), sizeof(_uint));

	for (size_t i = 0; i < m_iNumChannels; ++i)
	{
		CChannel* pChannel = CChannel::Create(InputFile, Bones);
		if (nullptr == pChannel)
			return E_FAIL;
		m_Channels.push_back(pChannel);
	}

	m_CurrentFrameIndices.resize(m_iNumChannels);
	

	return S_OK;
}

_bool CAnimation::Update_TransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition)
{
	if(nullptr != pTrackPosition)
		*pTrackPosition = m_fCurrentTrackPosition;

	if (m_fCurrentTrackPosition > m_fDuration)
	{
		m_fCurrentTrackPosition = 0.f;
		return true;
	}

	for (size_t i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Update_TransformationMatrix(m_fCurrentTrackPosition, Bones, &m_CurrentFrameIndices[i]);
	}

	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;

	return false;
}

_bool CAnimation::Update_TransformationMatrices_All(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition)
{
	if (nullptr != pTrackPosition)
		*pTrackPosition = m_fCurrentTrackPosition;

	if (m_fCurrentTrackPosition > m_fDuration)
	{
		m_fCurrentTrackPosition = 0.f;
		m_iNotifyIndex = 0;
		return true;
	}

	// Notfiy 현재 인덱스가 size를 넘지 않고, TrackPosition이 Notify에 해당한다면? 
	// Notify에 해당하는 함수를 실행하라.
	while (m_iNotifyIndex < m_Notifies.size() && m_fCurrentTrackPosition >= m_Notifies[m_iNotifyIndex].fTrackPosition)
		m_Notifies[m_iNotifyIndex++].Func();



	for (size_t i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Update_TransformationMatrix_All(m_fCurrentTrackPosition, Bones, &m_CurrentFrameIndices[i]);
	}

	

	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;

	return false;
}

_bool CAnimation::Blend_TransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float fTrackLength)
{
	if (m_fCurrentTrackPosition > fTrackLength)
	{
		m_fCurrentTrackPosition = 0.f;
		return true;
	}

	for (size_t i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Blend_TransformationMatrix(m_fCurrentTrackPosition, Bones, fTrackLength);
	}

	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;

	return false;
}

CAnimation* CAnimation::Create(ifstream& InputFile, const vector<class CBone*>& Bones)
{
	CAnimation* pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize(InputFile, Bones)))
	{
		MSG_BOX("Failed to Create : Animation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimation* CAnimation::Clone()
{
	return new CAnimation(*this);
}

void CAnimation::Free()
{
	__super::Free();

	m_Notifies.clear();

	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);
}
