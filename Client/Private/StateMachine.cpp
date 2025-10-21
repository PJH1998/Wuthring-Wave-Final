#include "ClientPch.h"
#include "StateMachine.h"
#include "GameSystem.h"
#include "State.h"

#pragma region 기본 함수들
CStateMachine::CStateMachine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{ pDevice, pContext }
{
}

CStateMachine::CStateMachine(const CStateMachine& Prototype)
    : CComponent(Prototype)
{

}

HRESULT CStateMachine::Initialize_Prototype(const _char* pFilePath)
{    
    Load_Data(pFilePath);

    return S_OK;
}

HRESULT CStateMachine::Initialize_Clone(void* pArg)
{
    return S_OK;
}

void CStateMachine::Change_State(_string strAnimationName)
{
    m_iCurrentStateIndex = m_StateInfos[strAnimationName];
}

void CStateMachine::Load_Data(const _char* pFilePath)
{
    CGameSystem* pGameSystem = CGameSystem::GetInstance();
    vector<vector<string>> csvData = pGameSystem->Load_CSV(pFilePath);

    //csvData[0]; 0번 줄은 안쓴다.
    for (_uint i = 1; i < csvData.size(); ++i)
    {
        _string strStateName = csvData[i][0];
        _string strAnimName = csvData[i][1];

        //m_StateInfos[strStateName] = strAnimName;

        
    }
    

    Safe_Release(pGameSystem);
}

#pragma endregion


CStateMachine* CStateMachine::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath)
{
    CStateMachine* pInstance = new CStateMachine(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pFilePath)))
    {
        MSG_BOX("Failed to Create : CPlayerParty");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CStateMachine::Clone(void* pArg)
{
    CStateMachine* pInstance = new CStateMachine(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CStateMachine");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CStateMachine::Free()
{
    CComponent::Free();
}
