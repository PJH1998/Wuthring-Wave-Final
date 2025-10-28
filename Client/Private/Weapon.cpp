#include "ClientPch.h"
#include "Weapon.h"
#include "Character.h"

CWeapon::CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject{ pDevice, pContext }
{
}

CWeapon::CWeapon(const CPartObject& Prototype)
    : CPartObject(Prototype)
{
}

HRESULT CWeapon::Initialize_Prototype()
{
    if (FAILED(CPartObject::Initialize_Prototype()))
        return E_FAIL;
    return S_OK;
}

HRESULT CWeapon::Initialize_Clone(void* pArg)
{
    WEAPON_DESC* pDesc = static_cast<WEAPON_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    if (FAILED(CPartObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    m_eWeaponType = pDesc->eWeaponType;
    return S_OK;
}

void CWeapon::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;
    CPartObject::Priority_Update(fTimeDelta);
}

void CWeapon::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    CPartObject::Update(fTimeDelta);
}

void CWeapon::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    CPartObject::Late_Update(fTimeDelta);
}


void CWeapon::Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate)
{
    ASSERT_CRASH(m_pModelCom);
    m_IsAnimationEnd = m_pModelCom->Play_Animation_GPU(
        m_pComputeShaderCom, strAnimName, fTimeDelta, &m_fTrackPosition, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate, fRootMotionRate);
    m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
}

void CWeapon::Free()
{
    CPartObject::Free();
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pComputeShaderCom);
    Safe_Release(m_pModelCom);
    Safe_Release(m_pRigidbodyCom);
    m_pParentTransform = { nullptr };
}
