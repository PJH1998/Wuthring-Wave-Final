#include "ClientPch.h"
#include "Actor.h"

#pragma region 기본 함수
CActor::CActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CContainerObject{ pDevice, pContext }
{

}

CActor::CActor(const CActor& Prototype)
    : CContainerObject(Prototype)
{
}

HRESULT CActor::Initialize_Prototype()
{
    if (FAILED(CContainerObject::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CActor::Initialize_Clone(void* pArg)
{
    if (FAILED(CContainerObject::Initialize_Clone(pArg)))
        return E_FAIL;

    return S_OK;
}

void CActor::Priority_Update(_float fTimeDelta)
{
    CContainerObject::Priority_Update(fTimeDelta);
}

void CActor::Update(_float fTimeDelta)
{
    CContainerObject::Update(fTimeDelta);
}

void CActor::Late_Update(_float fTimeDelta)
{
    CContainerObject::Late_Update(fTimeDelta);
}

void CActor::Render()
{

}
#pragma endregion


void CActor::Free()
{
    CContainerObject::Free();
}
