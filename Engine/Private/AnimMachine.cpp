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

void CAnimMachine::Handle_Input(CModel* pModelCom, _uint iIndex, _uint* pState)
{
	if(iIndex >= m_AnimStates.size())
		return;
	if(iIndex != m_iCurrentStateIndex)
	{
        m_AnimStates[m_iCurrentStateIndex]->Exit(pModelCom, pState);
		// Enter 새로운 상태
		m_AnimStates[iIndex]->Enter(pModelCom, pState, &m_strCurrentAnimTag);
		m_iCurrentStateIndex = iIndex;
	}
}

void CAnimMachine::Update(_float fTimeDelata, CModel* pModelCom, _uint* pState)
{
	// 1. 현재 상태 업데이트
	m_AnimStates[m_iCurrentStateIndex]->Update(fTimeDelata, this, pState, &m_strCurrentAnimTag);

	// 2. 애니메이션 재생
	_bool Result = pModelCom->Play_Animation_CPU(m_strCurrentAnimTag, fTimeDelata, nullptr);

	//Result : 모델 클래스가 애니메이션 한 트랙이 끝까지 재생되었을 때 true 반환, 이후 모델 내에서 트랙 위치 초기화

	// 3. 결과 피드백
	m_AnimStates[m_iCurrentStateIndex]->Feedback(Result, pState, this, pModelCom);
}

void CAnimMachine::Reset()
{
}

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

	for(auto& pState : m_AnimStates)
		Safe_Release(pState);
	m_AnimStates.clear();
}
