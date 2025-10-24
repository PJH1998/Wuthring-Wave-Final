#include "EnginePch.h"
#include "AnimMachine.h"
#include "Model.h"
#include "AnimState.h"

#pragma region ANIM_STATE

#pragma endregion

CAnimMachine::CAnimMachine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CAnimMachine::CAnimMachine(const CAnimMachine& Prototype)
	: CComponent{ Prototype }
{
}

HRESULT CAnimMachine::Initialize_Prototype(/*const _char* AnimMachineDataPath*/)
{
    return S_OK;
}

HRESULT CAnimMachine::Initialize_Clone(void* pArg)
{
	ANIMMACNINE_DESC* pDesc = (ANIMMACNINE_DESC*)pArg;
	if(nullptr == pDesc)
		return E_FAIL;
	m_strCurrentAnimTag = pDesc->pAnimationTag;
    return S_OK;
}

//void CAnimMachine::Handle_Input(CModel* pModelCom, _uint* pState, _uint iIndex)
void CAnimMachine::Handle_Input(CModel* pModelCom, _uint* pState, _string strAnimTag)
{
	//if(iIndex >= m_AnimStates.size())
	if(m_AnimStates.end() == m_AnimStates.find(strAnimTag))
		return;
	//if(iIndex != m_iCurrentStateIndex)
	if(0 != m_strCurrentAnimTag.compare(strAnimTag))
	{
        //m_AnimStates[m_iCurrentStateIndex]->Exit(pModelCom, pState);
		//// Enter 새로운 상태
		//m_AnimStates[iIndex]->Enter(pModelCom, pState, &m_strCurrentAnimTag);
		//m_iCurrentStateIndex = iIndex;
		m_AnimStates[m_strCurrentAnimTag]->Exit(pModelCom, pState);
		m_AnimStates[strAnimTag]->Enter(pModelCom, pState, &m_strCurrentAnimTag);

	}
}

void CAnimMachine::Update(CModel* pModelCom, _uint* pState, _float fTimeDelata)
{
	// 1. 현재 상태 업데이트
	//m_AnimStates[m_iCurrentStateIndex]->Update( this, pModelCom, pState, &m_strCurrentAnimTag, fTimeDelata);
	m_AnimStates[m_strCurrentAnimTag]->Update( this, pModelCom, pState, &m_strCurrentAnimTag, fTimeDelata);

	// 2. 애니메이션 재생
	_float fCurTrackPos{};
	//_bool Result = pModelCom->Play_Animation_CPU(m_strCurrentAnimTag, fTimeDelata, &fCurTrackPos);
	_bool Result = m_AnimStates[m_strCurrentAnimTag]->Play_Animation(pModelCom, fTimeDelata);
	//Result : 모델 클래스가 애니메이션 한 트랙이 끝까지 재생되었을 때 true 반환, 이후 모델 내에서 트랙 위치 초기화

	// 3. 결과 피드백 (우선 Norify에서 해결하는 방식으로 생각 중)
	//m_AnimStates[m_iCurrentStateIndex]->Feedback(Result, pState, this, pModelCom);
}

void CAnimMachine::Update(CModel* pModelCom, CComputeShader* pComputeShaderCom, _uint* pState, _float fTimeDelata)
{
	m_AnimStates[m_strCurrentAnimTag]->Update(this, pModelCom, pState, &m_strCurrentAnimTag, fTimeDelata);

	_bool Result = m_AnimStates[m_strCurrentAnimTag]->Play_Animation_GPU(pModelCom, pComputeShaderCom, fTimeDelata);
	//_bool Result = pModelCom->Play_Animation_GPU(pComputeShaderCom, m_strCurrentAnimTag, fTimeDelata, &fCurTrackPos);

}

void CAnimMachine::Reset()
{
}

#ifdef _DEBUG
void CAnimMachine::Create_AnimStates(const vector<_string>& AnimationNames)
{
	for(auto& strAnimationName : AnimationNames)
	{
		CAnimState::ANIMSTATE_DESC Temp{};
		m_AnimStates.emplace(strAnimationName, CAnimState::Create(strAnimationName, Temp));
	}
}

void CAnimMachine::Clear_States()
{
	for(auto& Pair : m_AnimStates)
		Safe_Release(Pair.second);
	m_AnimStates.clear();
}

void CAnimMachine::Add_StateData(_string& strAnimName, _bool isBlend, _bool isRootMotion, _float fRootMotionRate, _float fTransitTrackPos, _float fAnimationSpeed, _uint iConstAnimRunning)
{
	if(m_AnimStates.find(strAnimName) == m_AnimStates.end())
		return;

	CAnimState::ANIMSTATE_DESC AnimStateDesc{};
	AnimStateDesc.isBlend = isBlend;
	AnimStateDesc.isRootMotion = isRootMotion;
	AnimStateDesc.fRootMotionRate = fRootMotionRate;
	AnimStateDesc.fTransitTrackPos = fTransitTrackPos;
	AnimStateDesc.fAnimationSpeed = fAnimationSpeed;
	AnimStateDesc.iConstAnimRunning = iConstAnimRunning;

	m_AnimStates[strAnimName]->Set_Data(AnimStateDesc);

}

_bool CAnimMachine::Render_CurrentStateGUI(_string& strCurrentAnim)
{
	if(m_AnimStates.find(strCurrentAnim) == m_AnimStates.end())
		return false;

	m_AnimStates[strCurrentAnim]->Render_GUI();
	return true;
}
#endif

CAnimMachine* CAnimMachine::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAnimMachine* pInstance = new CAnimMachine(pDevice, pContext);
    if(FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CAnimMachine");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CAnimMachine::Clone(void* pArg)
{
    CAnimMachine* pInstance = new CAnimMachine(*this);
    if(FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Cloned : CAnimMachine");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CAnimMachine::Free()
{
    __super::Free();

	for(auto& Pair : m_AnimStates)
		Safe_Release(Pair.second);
	m_AnimStates.clear();
}

#ifdef _DEBUG
void CAnimMachine::Save_AnimDatas(json& jsonOutput)
{

	jsonOutput["AnimStates"] = json::array();
	for(auto& Pair : m_AnimStates)
	{
		json AnimState = json::object();
		AnimState["Name"] = Pair.first;
		CAnimState::ANIMSTATE_DESC DataDesc = Pair.second->Get_StateData();

		AnimState["isBlend"] = DataDesc.isBlend;
		AnimState["isRootMotion"] = DataDesc.isRootMotion;
		AnimState["fRootMotionRate"] = DataDesc.fRootMotionRate;
		AnimState["fTransitTrackPos"] = DataDesc.fTransitTrackPos;
		AnimState["fAnimationSpeed"] = DataDesc.fAnimationSpeed;
		AnimState["Transitions"] = json::array();

		jsonOutput["AnimStates"].push_back(AnimState);
	}
}

void CAnimMachine::Load_AnimDatas(json& jsonInput)
{

}
#endif