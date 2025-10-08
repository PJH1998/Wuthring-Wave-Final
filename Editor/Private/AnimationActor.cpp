#include "EditorPch.h"
#include "AnimationActor.h"

CAnimationActor::CAnimationActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CContainerObject{ pDevice, pContext }
{
}

CAnimationActor::CAnimationActor(const CAnimationActor& Prototype)
    : CContainerObject(Prototype)
{
}

HRESULT CAnimationActor::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAnimationActor::Initialize_Clone(void* pArg)
{
    ANIMATION_ACTOR_DESC* pDesc = static_cast<ANIMATION_ACTOR_DESC*>(pArg);
    if (FAILED(CContainerObject::Initialize_Clone(pDesc)))
        return E_FAIL;



    return S_OK;
}

void CAnimationActor::Priority_Update(_float fTimeDelta)
{
    CContainerObject::Priority_Update(fTimeDelta);
}

void CAnimationActor::Update(_float fTimeDelta)
{
    CContainerObject::Update(fTimeDelta);
}

void CAnimationActor::Late_Update(_float fTimeDelta)
{
    CContainerObject::Late_Update(fTimeDelta);

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this)))
        return;
}

void CAnimationActor::Render()
{

}

CGameObject* CAnimationActor::Clone(void* pArg)
{
    CAnimationActor* pInstance = new CAnimationActor(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CAnimationActor");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CAnimationActor* CAnimationActor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAnimationActor* pInstance = new CAnimationActor(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CAnimationActor");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAnimationActor::Free()
{
}
