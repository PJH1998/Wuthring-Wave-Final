#include "ClientPch.h"
#include "PlayerAugusta.h"



CPlayerAugusta::CPlayerAugusta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPlayer{ pDevice, pContext }
{
}

CPlayerAugusta::CPlayerAugusta(const CPlayerAugusta& Prototype)
    : CPlayer(Prototype)
{
}

HRESULT CPlayerAugusta::Initialize_Prototype()
{
    if (FAILED(CPlayer::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CPlayerAugusta::Initialize_Clone(void* pArg)
{
    PLAYER_DESC* pDesc = static_cast<PLAYER_DESC*>(pArg);

    // 1. Player 상위 클래스 초기화
    if (FAILED(CPlayer::Initialize_Clone(pDesc)))
        return E_FAIL;

    // 2. 
    if (FAILED(Ready_Components(pDesc)))
        return E_FAIL;

    return S_OK;
}

void CPlayerAugusta::Priority_Update(_float fTimeDelta)
{
    CPlayer::Priority_Update(fTimeDelta);

}

void CPlayerAugusta::Update(_float fTimeDelta)
{
    CPlayer::Update(fTimeDelta);
}

void CPlayerAugusta::Late_Update(_float fTimeDelta)
{
    CPlayer::Late_Update(fTimeDelta);
}

void CPlayerAugusta::Render()
{

}

void CPlayerAugusta::Render_Shadow()
{
}

HRESULT CPlayerAugusta::Ready_Components(const PLAYER_DESC* pDesc)
{
    
    return S_OK;
}

CPlayerAugusta* CPlayerAugusta::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CPlayerAugusta* pInstance = new CPlayerAugusta(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CPlayerAugusta");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CPlayerAugusta::Clone(void* pArg)
{
    CPlayerAugusta* pInstance = new CPlayerAugusta(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CPlayerAugusta");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CPlayerAugusta::Free()
{
    CPlayer::Free();
}
