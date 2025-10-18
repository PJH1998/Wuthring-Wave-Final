#include "ClientPch.h"
#include "AnimMacnine.h"
#include "Model.h"
#include "AnimState.h"

#pragma region ANIM_STATE

#pragma endregion

CAnimMacnine::CAnimMacnine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CAnimMacnine::CAnimMacnine(const CAnimMacnine& Prototype)
	: CComponent{ Prototype }
{
}

HRESULT CAnimMacnine::Initialize_Prototype()
{
    return E_NOTIMPL;
}

HRESULT CAnimMacnine::Initialize_Clone(void* pArg)
{
	ANIMMACNINE_DESC* pDesc = (ANIMMACNINE_DESC*)pArg;
	if(nullptr == pDesc)
		return E_FAIL;
	m_pOwnerState = pDesc->pOwnerState;

    return S_OK;
}

void CAnimMacnine::Handle_Input(CModel* pModelCom, _uint iIndex)
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

void CAnimMacnine::Update(_float fTimeDelata, CModel* pModelCom)
{
}

void CAnimMacnine::Reset()
{
}

CAnimMacnine* CAnimMacnine::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAnimMacnine* pInstance = new CAnimMacnine(pDevice, pContext);
    if(FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CAnimMacnine");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CAnimMacnine::Clone(void* pArg)
{
    CAnimMacnine* pInstance = new CAnimMacnine(*this);
    if(FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Cloned : CAnimMacnine");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CAnimMacnine::Free()
{
    __super::Free();
}
