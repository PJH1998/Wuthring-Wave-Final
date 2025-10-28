#include "EnginePch.h"
#include "AnimTransition.h"

CAnimTransition::CAnimTransition()
{
}

HRESULT CAnimTransition::Initialize_Prototype(json& jsonParser)
{
    m_strNextState =  jsonParser["To"];
    //for(auto& strCondition : jsonParser["Conditions"])

    m_iPriority = jsonParser["Priority"];
    //저장방식 정립하면 변경하자.
    //_uint iTargetState = jsonParser["Target State"];
    //m_iTargetState = iTargetState == 0 ? 0 : (1 << (iTargetState - 1));
    m_iTargetState = jsonParser["Target State"];
    m_fTargetTrackPos = jsonParser["Transit Target Pos"];
    m_fTransitEnablePos = jsonParser["Transit Enable Pos"];
    return S_OK;
}

_bool CAnimTransition::Is_Transit(const _uint* pOwnerState, _string& strNextState, _float& fTargetTrackPos)
{
    if(*pOwnerState & m_iTargetState)
    {
        //for(auto& Func : m_Conditions)
        //    if(Func(pOwnerState))
        //    {
        //        return false;
        //    }

        strNextState = m_strNextState;
        fTargetTrackPos = m_fTargetTrackPos;
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
