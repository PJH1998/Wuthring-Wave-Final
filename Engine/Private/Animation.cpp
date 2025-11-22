#include "EnginePch.h"
#include "Animation.h"

#include "Channel.h"
#include "MorphChannel.h"

#include "SoundNotify.h"
#include "ColliderNotify.h"
#include "EffectNotify.h"
#include "ObjectFuncNotify.h"

CAnimation::CAnimation()
{
}

CAnimation::CAnimation(const CAnimation& Prototype)
	: m_fDuration { Prototype.m_fDuration },
	m_fTickPerSecond { Prototype.m_fTickPerSecond },
	m_fCurrentTrackPosition { Prototype.m_fCurrentTrackPosition },
	m_iNumChannels { Prototype.m_iNumChannels },
	m_Channels { Prototype.m_Channels },
	m_CurrentFrameIndices{ Prototype.m_CurrentFrameIndices },
	m_CurrentMorphCurveIndicies{ Prototype.m_CurrentMorphCurveIndicies },
	m_iNumMorphCurves{ Prototype.m_iNumMorphCurves },
	m_MorphKeyIndicies { Prototype.m_MorphKeyIndicies },
	m_MorphMeshChannels { Prototype.m_MorphMeshChannels } 
	
{
	strcpy_s(m_szName, Prototype.m_szName);

	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);

	
	for (auto& pMorphChannel : m_MorphMeshChannels)
		Safe_AddRef(pMorphChannel);
}

void CAnimation::Register_Notify(const NOTIFY& AnimNotify)
{
	m_Notifies.push_back(AnimNotify);
}

void CAnimation::Load_Notify(const json& notifyJson, function<void(const _wstring&, _bool)> ColliderCallback, function<void(const _wstring&)> EffectCallback, function<void(const _wstring&)> ObjectCallback)
{
	for (const auto& notifyObject : notifyJson)
	{
		string type = notifyObject["NotifyType"].get<string>();
		CAnimNotify* pAnimNotify = { nullptr };

		// 1. 
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
			pAnimNotify = CEffectNotify::From_Json(notifyObject);
			pAnimNotify->Set_EffectCallback(EffectCallback);
		}
		else if (type == "Object")
		{
			// ObjectNotify
			pAnimNotify = CObjectFuncNotify::From_Json(notifyObject);
			dynamic_cast<CObjectFuncNotify*>(pAnimNotify)->Set_ObjectCallback(ObjectCallback);
		}
		ASSERT_CRASH(pAnimNotify);

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

void CAnimation::Sort_AnimNotify()
{
	if (0 == m_AnimNotifies.size())
		return;

	sort(m_AnimNotifies.begin(), m_AnimNotifies.end(), [](CAnimNotify* pSrc, CAnimNotify* pDst)->_bool{
		return pSrc->Get_TrackPosition() < pDst->Get_TrackPosition();
	});
}


void CAnimation::Reset_Status()
{
	m_fCurrentTrackPosition = 0.f;
	m_iNotifyIndex = 0;

	// Bone Channel 캐싱 인덱스 초기화
	if (!m_CurrentFrameIndices.empty())
		fill(m_CurrentFrameIndices.begin(), m_CurrentFrameIndices.end(), 0); // 0으로 채워서 처음부터 다시 검색하게 함

	// Morph Channel 캐싱 인덱스 초기화
	if (!m_CurrentMorphCurveIndicies.empty())
		fill(m_CurrentMorphCurveIndicies.begin(), m_CurrentMorphCurveIndicies.end(), 0); // 0번 키프레임부터 다시 찾도록 리셋
}

HRESULT CAnimation::Initialize(ifstream& InputFile, const vector<class CBone*>& Bones, MODELTYPE eModelType)
{
#pragma region 기존 애니메이션 내용 저장.
	_uint iLength = {};
	InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
	_char szAnimationName[MAX_PATH] = {};
	InputFile.read(szAnimationName, iLength);

	_char* pAnimationName = { nullptr };
	strtok_s(szAnimationName, "|", &pAnimationName);
	if (0 == strcmp(pAnimationName, ""))
		strcpy_s(m_szName, szAnimationName);
	else
		strcpy_s(m_szName, pAnimationName);

	InputFile.read(reinterpret_cast<_char*>(&m_fDuration), sizeof(_float));
	InputFile.read(reinterpret_cast<_char*>(&m_fTickPerSecond), sizeof(_float));

	InputFile.read(reinterpret_cast<_char*>(&m_iNumChannels), sizeof(_uint));

	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		CChannel* pChannel = CChannel::Create(InputFile, Bones);
		if (nullptr == pChannel)
			return E_FAIL;
		m_Channels.push_back(pChannel);
	}

	m_CurrentFrameIndices.resize(m_iNumChannels);
	

	// 추가 작업.
	if (eModelType == MODELTYPE::CHARACTER)
	{
		// 1. Morph Mesh Curves 개수를 받아옵니다. => 	ModelLoader의 iTotalCurves와 동일.
		InputFile.read(reinterpret_cast<_char*>(&m_iNumMorphCurves), sizeof(_uint));

		// 2. Morph Mesh Curves 정보를 가져옵니다.
		for (_uint i = 0; i < m_iNumMorphCurves; ++i)
		{
			CMorphChannel* pMorphChannel = CMorphChannel::Create(InputFile);
			if (nullptr == pMorphChannel)
				return E_FAIL;

			m_MorphMeshChannels.push_back(pMorphChannel);
		}

		m_CurrentMorphCurveIndicies.resize(m_iNumMorphCurves);

	}
#pragma endregion

	

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

//_bool CAnimation::Update_RibTransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition)
//{
//	if (nullptr != pTrackPosition)
//		*pTrackPosition = m_fCurrentTrackPosition;
//
//	if (m_fCurrentTrackPosition > m_fDuration)
//	{
//		m_fCurrentTrackPosition = 0.f;
//		return true;
//	}
//
//
//	for (size_t i = 0; i < m_iNumChannels; ++i)
//	{
//		m_Channels[i]->Update_RibTransformationMatrix(m_fCurrentTrackPosition, Bones, &m_CurrentFrameIndices[i]);
//	}
//
//	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;
//
//	return false;
//}

_bool CAnimation::Update_RibTransformationMatrices(_float fTrackPosition, const vector<class CBone*>& Bones, _float* pTrackPosition)
{
	m_fCurrentTrackPosition = fTrackPosition;

	if (m_fCurrentTrackPosition > m_fDuration)
	{
		m_fCurrentTrackPosition = 0.f;
		return true;
	}


	for (size_t i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Update_RibTransformationMatrix(m_fCurrentTrackPosition, Bones, &m_CurrentFrameIndices[i]);
	}

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

// MorphChannels 바인딩.
_bool CAnimation::Bind_MorphChannels(const vector<string>& modelShapeKeys)
{
	m_MorphKeyIndicies.clear();
	m_MorphKeyIndicies.resize(m_MorphMeshChannels.size(), -1); // -1로 초기화.

	for (size_t i = 0; i < m_MorphMeshChannels.size(); ++i)
	{
		const _char* pChannelName = m_MorphMeshChannels[i]->Get_Name();

		// 모델의 쉐이프 키 목록에서 이름이 같은 인덱스를 찾음
		for (size_t j = 0; j < modelShapeKeys.size(); ++j)
		{
			if (modelShapeKeys[j] == pChannelName)
			{
				// 매핑 성공! (예: Channel 0번은 Model Index 3번을 제어)
				m_MorphKeyIndicies[i] = static_cast<_int>(j); 
				break;
			}
		}
	}
	
	return true;
}

_bool CAnimation::Update_MorphWeights(_float fTimeDelta, vector<float>& modelWeights)
{

	if (modelWeights.empty()) return false;

	// 모든 가중치 0으로 초기화
	fill(modelWeights.begin(), modelWeights.end(), 0.0f);

	// MorphMeshChannel[i] -> m_MorphKeyIndices[i] = 들어있는 값(modelWeights의 인덱스)
	for (size_t i = 0; i < m_MorphMeshChannels.size(); ++i)
	{
		_int iTargetIndex = m_MorphKeyIndicies[i];

		// 매핑된 대상이 없다면? 스킵합니다.
		if (iTargetIndex == -1) continue;
		if (iTargetIndex >= modelWeights.size()) continue;

		// 현재 시간 가중치 계산
		_float fWeight = m_MorphMeshChannels[i]->Get_CurrentWeight(m_fCurrentTrackPosition, &m_CurrentMorphCurveIndicies[i]);

		// 매핑된 타겟인덱스에 가중치 부여.
		modelWeights[iTargetIndex] = fWeight;
		//_float fWeight = m_MorphMeshChannels[i]->Get_CurrentWeight(m_fCurrentTrackPosition, &m_CurrentMorphCurveIndicies[i]);
		//_float fWeight = m_MorphMeshChannels[i]->Get_CurrentWeight(m_fCurrentTrackPosition, &m_CurrentMorphCurveIndicies[iTargetIndex]);

		// 모델의 해당 인덱스에 가중치 적용
		//modelWeights[iTargetIndex] = fWeight;
		//modelWeights[i] = fWeight;
	}

	return true;
}

CAnimation* CAnimation::Create(ifstream& InputFile, const vector<class CBone*>& Bones, MODELTYPE eModelType)
{
	CAnimation* pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize(InputFile, Bones, eModelType)))
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

	for (auto& pAnimNotfiy : m_AnimNotifies)
		Safe_Release(pAnimNotfiy);
	m_AnimNotifies.clear();

	m_Notifies.clear();

	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);
	m_Channels.clear();

	for (auto& pMorphCannel : m_MorphMeshChannels)
		Safe_Release(pMorphCannel);

	m_MorphMeshChannels.clear();
}
