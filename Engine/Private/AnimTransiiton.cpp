#include "EnginePch.h"
#include "AnimTransiiton.h"

CAnimTransiiton::CAnimTransiiton()
{
}

HRESULT CAnimTransiiton::Initialize_Prototype(ifstream& File)
{
    return E_NOTIMPL;
}

_bool CAnimTransiiton::Can_Transit()
{
    for(auto& Func : m_Conditions)
        if(Func())
            return true;

    return false;
}

CAnimTransiiton* CAnimTransiiton::Create(ifstream& File)
{
    CAnimTransiiton* pInstance = new CAnimTransiiton();
    if(FAILED(pInstance->Initialize_Prototype(File)))
    {
        MSG_BOX("Failed to Created : CAnimTransiiton");
        Safe_Release(pInstance);
    }
    return pInstance;
}

#ifdef _DEBUG
CAnimTransiiton* CAnimTransiiton::Create()
{
    CAnimTransiiton* pInstance = new CAnimTransiiton();
    return pInstance;
}
#endif // _DEBUG

void CAnimTransiiton::Free()
{
    __super::Free();
}
