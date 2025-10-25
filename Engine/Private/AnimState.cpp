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
void CAnimState::Set_Data(ANIMSTATE_DESC& StateDesc)
{
	m_StateData = StateDesc;
}
void CAnimState::Render_GUI()
{
	if(ImGui::CollapsingHeader("Anim State Property"))
	{
		ImGui::Text("Animation: %s", m_strAnimationTag.c_str());
		m_StateData.isBlend ? ImGui::Text("isBlend : true") : ImGui::Text("isBlend : false");
		m_StateData.isRootMotion ? ImGui::Text("isRootMotion : true") : ImGui::Text("isRootMotion : false");
		ImGui::Text("RootMotionRate: %.3f", m_StateData.fRootMotionRate);
		ImGui::Text("TransitTrackPos: %.3f", m_StateData.fTransitTrackPos);
		ImGui::Text("AnimationSpeed: %.3f", m_StateData.fAnimationSpeed);
		//ImGui::Text("RootMotionRate: %u", m_iConstAnimRunning);
	}
}
#endif

HRESULT CAnimState::Initialize(json& jsonParser, json& jsonTransitions)
{
	m_strAnimationTag = jsonParser["Name"];
	m_StateData.isBlend = jsonParser["isBlend"];
	m_StateData.isRootMotion = jsonParser["isRootMotion"];
	m_StateData.fRootMotionRate = jsonParser["fRootMotionRate"];
	m_StateData.fTransitTrackPos = jsonParser["fTransitTrackPos"];
	m_StateData.fAnimationSpeed = jsonParser["fAnimationSpeed"];

	for(auto& jsonTransition : jsonTransitions)
	{
		if(0 == m_strAnimationTag.compare(jsonTransition["From"]))
			m_Transitions.push_back(CAnimTransition::Create(jsonTransition));
	}
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
	_float fNextTargetTrackPos{};
	for(auto& Transition : m_Transitions)
	{
		if(m_fCurrentTrackPositon < Transition->Get_TransitEnablePos())
			continue;

		if(Transition->Is_Transit(pOwnerState, strNextAnimTag, fNextTargetTrackPos))
		{
			pAnimMachine->Handle_Input(pModelCom, pOwnerState, strNextAnimTag);
		}
	}
}

void CAnimState::Exit(CModel* pModelCom, _uint* pOwnerState)
{

}

_bool CAnimState::Play_Animation(CModel* pModelCom, _float fTimeDelta)
{
	return pModelCom->Play_Animation_CPU(m_strAnimationTag, fTimeDelta * m_StateData.fAnimationSpeed, &m_fCurrentTrackPositon, 
										m_StateData.isBlend, m_StateData.isRootMotion, m_StateData.fRootMotionRate);
	//return pModelCom->Play_Animation_CPU(m_strAnimationTag, fTimeDelta * m_StateData.fAnimationSpeed, &m_fCurrentTrackPositon);
}

_bool CAnimState::Play_Animation_GPU(CModel* pModelCom, CComputeShader* pComputeShaderCom, _float fTimeDelta)
{
	return pModelCom->Play_Animation_GPU(pComputeShaderCom, m_strAnimationTag, fTimeDelta * m_StateData.fAnimationSpeed, &m_fCurrentTrackPositon, 
										m_StateData.isRootMotion, m_StateData.fRootMotionRate);
	//return pModelCom->Play_Animation_GPU(pComputeShaderCom, m_strAnimationTag, fTimeDelta * m_StateData.fAnimationSpeed, &m_fCurrentTrackPositon);
}

void CAnimState::Feedback(_bool isAnimationFinished, _uint* pOwnerState, CAnimMachine* pAnimMachineCom, CModel* pModelCom)
{
	if(isAnimationFinished)
	{
		//pAnimMachineCom->Handle_Input(pModelCom, pOwnerState, strNextAnimTag);
	}
}
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

CAnimState* CAnimState::Create(json& jsonParser, json& jsonTransitions)
{
	CAnimState* pInstance = new CAnimState();
	if(FAILED(pInstance->Initialize(jsonParser, jsonTransitions)))
	{
		MSG_BOX("Failed to Created : CAnimState");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAnimState::Free()
{
    __super::Free();

	for(auto& pTransition : m_Transitions)
		Safe_Release(pTransition);

	m_Transitions.clear();
}
