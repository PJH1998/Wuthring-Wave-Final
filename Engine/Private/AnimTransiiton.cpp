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
    return _bool();
}

CAnimTransiiton* CAnimTransiiton::Create(ifstream& File)
{
    return nullptr;
}

#ifdef _DEBUG
CAnimTransiiton* CAnimTransiiton::Create()
{
    return nullptr;
}
#endif // _DEBUG

void CAnimTransiiton::Free()
{
}
