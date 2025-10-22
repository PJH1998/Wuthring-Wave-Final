#include "ClientPch.h"
#include "Character.h"
#include "InputController.h"

CCharacter::CCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CActor{ pDevice, pContext }
{
}

CCharacter::CCharacter(const CCharacter& Prototype)
    : CActor(Prototype)
    
{
}

HRESULT CCharacter::Initialize_Prototype()
{
    if (FAILED(CActor::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CCharacter::Initialize_Clone(void* pArg)
{
    CHARACTER_DESC* pDesc = static_cast<CHARACTER_DESC*>(pArg);

    // 1. 상위 객체 초기화
    if (FAILED(CActor::Initialize_Clone(pDesc)))
        return E_FAIL;

    // 2. 하위 객체 초기화
    //m_pController = pDesc->pController;


    return S_OK;
}

void CCharacter::Priority_Update(_float fTimeDelta)
{
    CActor::Priority_Update(fTimeDelta);
}

void CCharacter::Update(_float fTimeDelta)
{
    CActor::Update(fTimeDelta);
}

void CCharacter::Late_Update(_float fTimeDelta)
{
    CActor::Late_Update(fTimeDelta);
}

void CCharacter::Render()
{
}

void CCharacter::Render_Shadow()
{
}

#pragma region STATE에서 사용
_bool CCharacter::Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion)
{
    ASSERT_CRASH(m_pModelCom);
    _bool IsPlayAnimationEnd = m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, strAnimName, fTimeDelta, pTrackPosition, IsRootMotion, fRootMotionRate);
    m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
    return IsPlayAnimationEnd;
}

_bool CCharacter::Check_AnyInput(_uint iKeyFlag)
{
    ASSERT_CRASH(m_pInputControllerCom);
    return m_pInputControllerCom->Check_AnyInput(iKeyFlag);
}

_bool CCharacter::Check_AllInput(_uint iKeyFlag)
{
    ASSERT_CRASH(m_pInputControllerCom);
    return m_pInputControllerCom->Check_AllInput(iKeyFlag);
}

_bool CCharacter::Is_LockOn()
{
    return m_IsLockOn;
}

void CCharacter::Change_State(_uint iCategory, _uint iSubState)
{
    ASSERT_CRASH(m_pStateMachineCom);
    m_pStateMachineCom->Change_State(iCategory, iSubState);
}

/* 캐스팅 해서 보내야됨. */
#pragma endregion







void CCharacter::Free()
{
    CActor::Free();
    Safe_Release(m_pInputControllerCom);
    Safe_Release(m_pStateMachineCom);
}
