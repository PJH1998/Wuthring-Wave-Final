#include "EnginePch.h"
#include "Animation.h"

#include "Channel.h"

#include "SoundNotify.h"
#include "ColliderNotify.h"

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

void CAnimation::Load_Notify(const json& notifyJson, function<void(const _wstring&, _bool)> ColliderCallback, function<void()> EffectCallback)
{
	for (const auto& notifyObject : notifyJson)
	{
		string type = notifyObject["NotifyType"].get<string>();
		CAnimNotify* pAnimNotify = { nullptr };

		// 1. 媛앹껜 ?앹꽦
		if (type == "Sound")
			pAnimNotify = CSoundNotify::From_Json(notifyObject);
		else if (type == "Collider")
		{
			pAnimNotify = CColliderNotify::From_Json(notifyObject);
			pAnimNotify->Set_ColliderCallBack(ColliderCallback);
		}
		else if (type == "Effect")
		{
			// EffectNotify
			pAnimNotify->Set_EffectCallback(EffectCallback);
		}
		// 2. 臾몄젣 ?앷린硫?Crash 諛쒖깮.
		ASSERT_CRASH(pAnimNotify);

		// 3. 愿由?而⑦뀒?대꼫???ｌ뼱?먭린.
		m_AnimNotifies.emplace_back(pAnimNotify);
	}
}


void CAnimation::Sort_Notify()
{
	if (0 == m_Notifies.size())
		return;

	sort(m_Notifies.begin(), m_Notifies.end(), [](const NOTIFY& Src, const NOTIFY& Dst)->_bool {
			return Src.fTrackPosition < Dst.fTrackPosition ? true : false;
		});
}

// ?좊땲硫붿씠?섏뿉 ?깅줉??Notify瑜?TrackPosition 蹂꾨줈 ?뺣젹.
void CAnimation::Sort_AnimNotify()
{
	if (0 == m_AnimNotifies.size())
		return;

	sort(m_AnimNotifies.begin(), m_AnimNotifies.end(), [](CAnimNotify* pSrc, CAnimNotify* pDst)->_bool{
		return pSrc->Get_TrackPosition() < pDst->Get_TrackPosition();
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

_bool CAnimation::Update_RibTransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition)
{
	if (nullptr != pTrackPosition)
		*pTrackPosition = m_fCurrentTrackPosition;

	if (m_fCurrentTrackPosition > m_fDuration)
	{
		m_fCurrentTrackPosition = 0.f;
		return true;
	}




	for (size_t i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Update_RibTransformationMatrix(m_fCurrentTrackPosition, Bones, &m_CurrentFrameIndices[i]);
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

	// Notfiy ?꾩옱 ?몃뜳?ㅺ? size瑜??섏? ?딄퀬, TrackPosition??Notify???대떦?쒕떎硫? 
	// Notify???대떦?섎뒗 ?⑥닔瑜??ㅽ뻾?섎씪.
	
	//while (m_iNotifyIndex < m_Notifies.size() && m_fCurrentTrackPosition >= m_Notifies[m_iNotifyIndex].fTrackPosition)
	//	m_Notifies[m_iNotifyIndex++].Func();

	while (m_iNotifyIndex < m_AnimNotifies.size() && m_fCurrentTrackPosition >= m_AnimNotifies[m_iNotifyIndex]->Get_TrackPosition())
		m_AnimNotifies[m_iNotifyIndex++]->Execute();



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

_bool CAnimation::Update_TrackPosition(_float fTimeDelta, _float* pTrackPosition)
{
	if (nullptr != pTrackPosition)
		*pTrackPosition = m_fCurrentTrackPosition;

	if (m_fCurrentTrackPosition > m_fDuration)
	{
		m_fCurrentTrackPosition = 0.f;
		m_iNotifyIndex = 0;
		return true;
	}

	while (m_iNotifyIndex < m_AnimNotifies.size() && m_fCurrentTrackPosition >= m_AnimNotifies[m_iNotifyIndex]->Get_TrackPosition())
		m_AnimNotifies[m_iNotifyIndex++]->Execute();

	/* 
	* ?먮옒 ?ш린??Animation 媛깆떊 濡쒖쭅??議댁옱.
	*/

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
