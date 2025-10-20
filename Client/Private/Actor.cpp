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
    ACTOR_DESC* pDesc = static_cast<ACTOR_DESC*>(pArg);
    if (FAILED(CContainerObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    // 1. Components 초기화
    if (FAILED(Ready_Components(pDesc)))
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

HRESULT CActor::Ready_Components(const ACTOR_DESC* pDesc)
{
    ASSERT_CRASH(pDesc);

    m_eCurLevel = pDesc->eCurLevel;
    // 1. Shader 
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(m_eCurLevel), pDesc->strShaderTag,
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
    {
        CRASH("Failed Ready_ComShader");
        return E_FAIL;
    }

    // 2. Compute Shader
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(m_eCurLevel), pDesc->strComputeShaderTag,
        TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
    {
        CRASH("Failed Ready_ComShader");
        return E_FAIL;
    }

    // 3. Model
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(m_eCurLevel), pDesc->strModelTag,
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
    {
        CRASH("Failed Ready Com_Model");
        return E_FAIL;
    }


    return S_OK;
}

void CActor::Free()
{
    CContainerObject::Free();
    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pComputeShaderCom);
}
