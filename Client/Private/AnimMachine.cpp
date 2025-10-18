#include "ClientPch.h"
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

HRESULT CAnimMachine::Initialize_Prototype()
{
    return E_NOTIMPL;
}

HRESULT CAnimMachine::Initialize_Clone(void* pArg)
{
	ANIMMACNINE_DESC* pDesc = (ANIMMACNINE_DESC*)pArg;
	if(nullptr == pDesc)
		return E_FAIL;
	m_pOwnerState = pDesc->pOwnerState;

    return S_OK;
}

void CAnimMachine::Handle_Input(CModel* pModelCom, _uint iIndex)
{
	if(iIndex >= m_AnimStates.size())
		return;
	if(iIndex != m_iCurrentStateIndex)
	{
        m_AnimStates[m_iCurrentStateIndex]->Exit(pModelCom, m_pOwnerState, &m_strCurrentAnimTag);
		// Enter 새로운 상태
		m_AnimStates[iIndex]->Enter(pModelCom, m_pOwnerState, &m_strCurrentAnimTag);
		m_iCurrentStateIndex = iIndex;
	}
}

void CAnimMachine::Update(_float fTimeDelata, CModel* pModelCom)
{
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
}
