#include "EnginePch.h"
#include "BT_Action.h"

CBT_Action::CBT_Action(const CBT_Action& Prototype)
	:CBT_Node{ Prototype }
	//,m_Action { Prototype.m_Action }
{
}

HRESULT CBT_Action::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBT_Action::Initialize_Clone(void* pArg)
{
	return S_OK;
}

CBT_Action* CBT_Action::Create(function<PATTERN_STATE(CGameObject* pGameObject)> Action)
{
    CBT_Action* pInstance = new CBT_Action(Action);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CBT_Action");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBT_Node* CBT_Action::Clone(void* pArg)
{
    CBT_Action* pInstance = new CBT_Action(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBT_Action");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBT_Action::Free()
{
    __super::Free();
}
