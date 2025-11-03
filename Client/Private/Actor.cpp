#include "ClientPch.h"
#include "Actor.h"
#include "Ability.h"


#pragma region
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
    ACTOR_DESC* pDesc = static_cast<ACTOR_DESC*>(pArg);
    if (FAILED(CContainerObject::Initialize_Clone(pDesc)))
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

void CActor::Register_AllNotifies(const _string& strFolderPath)
{
    ASSERT_CRASH(m_pModelCom);
    auto colliderCallback = [this](const _wstring& tag, bool active) {
        this->Collider_Active(tag, active); 
        };

    auto effectCallBack = [this](const _wstring& tag) {
        this->Effect_Active(tag);
        };

    m_pModelCom->Register_AllNotifies(strFolderPath, colliderCallback, effectCallBack);
}

void CActor::Register_AllStatFiles(const _string& strFolderPath)
{

}


void CActor::Free()
{
    CContainerObject::Free();
    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pComputeShaderCom);
    Safe_Release(m_pRigidBodyCom);
    Safe_Release(m_pColliderCom);
	Safe_Release(m_pAbillityCom);
}
