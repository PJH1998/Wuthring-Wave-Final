#include "EnginePch.h"
#include "AnimState.h"
#include "Model.h"
#include "AnimMachine.h"
#include "AnimTransition.h"

#ifdef _DEBUG
HRESULT CAnimState::Initialize(const _string& strAnimationTag, ANIMSTATE_DESC& StateDesc)
{
	m_StateData = StateDesc;
	m_strAnimationTag = strAnimationTag;
    return S_OK;
}
#endif

HRESULT CAnimState::Initialize(json& jsonParser)
{
	return S_OK;
}

void CAnimState::Enter(CModel* pModelCom, _uint* pOwnerState, _string* pCurrentAnimTag)
{
	*pCurrentAnimTag = m_strAnimationTag;

}

void CAnimState::Update(class CAnimMachine* pAnimMachine, CModel* pModelCom, _uint* pOwnerState, _string* pCurrentAnimTag, _float fTrackPosition/*, ANIMSTATE_DESC& StateData*/)
{
	//*pOwnerState |= ENUM_CLASS(TEST_STATE::ANIMATION_PLAYING);

	//_int iNextIndex{};
	_string strNextAnimTag{};
	for(auto& Transition : m_Transitions)
	{
		if(!pModelCom->isTrackPositionOver(m_strAnimationTag, Transition->Get_TransitEnablePos()))
			continue;

		if(Transition->Is_Transit(pOwnerState, strNextAnimTag))
		{
			pAnimMachine->Handle_Input(pModelCom, pOwnerState, strNextAnimTag);
		}
	}
}

void CAnimState::Exit(CModel* pModelCom, _uint* pOwnerState)
{

}

//void CAnimState::Feedback(_bool isAnimationFinished, _uint* pOwnerState, CAnimMachine* pAnimMachineCom, CModel* pModelCom)
//{
//	if(isAnimationFinished)
//	{
//		*pOwnerState &= ~(m_StateData.iConstAnimRunning);
//	}
//}
#ifdef _DEBUG
CAnimState* CAnimState::Create(const _string& strAnimationTag, ANIMSTATE_DESC& StateDesc)
{
	CAnimState* pInstance = new CAnimState();
	if(FAILED(pInstance->Initialize(strAnimationTag, StateDesc)))
	{
		MSG_BOX("Failed to Created : CAnimState");
		Safe_Release(pInstance);
	}
    return pInstance;
}
#endif
CAnimState* CAnimState::Create(json& jsonParset)
{
	CAnimState* pInstance = new CAnimState();
	if(FAILED(pInstance->Initialize(jsonParset)))
	{
		MSG_BOX("Failed to Created : CAnimState");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAnimState::Free()
{
    __super::Free();
}
