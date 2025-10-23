#include "EnginePch.h"
#include "AnimTransition.h"

CAnimTransition::CAnimTransition()
{
}

HRESULT CAnimTransition::Initialize_Prototype(json& jsonParser)
{
    m_strNextState =  jsonParser["NextState"];
    for(auto& strCondition : jsonParser["Conditions"])
    {

    }

    return S_OK;
}

_bool CAnimTransition::Is_Transit(const _uint* pOwnerState, _string& strNextState)
{
    for(auto& Func : m_Conditions)
        if(Func(pOwnerState))
        {
            strNextState = m_strNextState;
            return true;
        }

    return false;
}

CAnimTransition* CAnimTransition::Create(json& jsonParser)
{
    CAnimTransition* pInstance = new CAnimTransition();
    if(FAILED(pInstance->Initialize_Prototype(jsonParser)))
    {
        MSG_BOX("Failed to Created : CAnimTransition");
        Safe_Release(pInstance);
    }
    return pInstance;
}

#ifdef _DEBUG
CAnimTransition* CAnimTransition::Create()
{
    CAnimTransition* pInstance = new CAnimTransition();
    return pInstance;
}
#endif // _DEBUG

void CAnimTransition::Free()
{
    __super::Free();
}
