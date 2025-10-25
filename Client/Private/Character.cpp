#include "ClientPch.h"
#include "Character.h"
#include "InputController.h"
#include "SpringCamera.h"

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

    // 1. ���� ��ü �ʱ�ȭ
    if (FAILED(CActor::Initialize_Clone(pDesc)))
        return E_FAIL;

    // 2. ���� ��ü �ʱ�ȭ
    //m_pController = pDesc->pController;


    return S_OK;
}

void CCharacter::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;
    
    CActor::Priority_Update(fTimeDelta);
}

void CCharacter::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    CActor::Update(fTimeDelta);
}

void CCharacter::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    CActor::Late_Update(fTimeDelta);
}

void CCharacter::Render()
{
}

void CCharacter::Render_Shadow()
{
}

void CCharacter::Process_Input(CInputController* pInputControllerCom)
{
    m_pInputControllerCom = pInputControllerCom;
    Safe_AddRef(m_pInputControllerCom);
}


#pragma region STATE

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

_bool CCharacter::Is_Land(_float3* pNormal)
{
    ASSERT_CRASH(m_pColliderCom);
    return m_pColliderCom->IsLand(pNormal);
}

void CCharacter::Change_State(_uint iCategory, _uint iSubState)
{
    ASSERT_CRASH(m_pStateMachineCom);
    m_pStateMachineCom->Change_State(iCategory, iSubState);
}

ACTORDIR CCharacter::Calculate_Direction()
{
    _bool bW = Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
    _bool bS = Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
    _bool bA = Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
    _bool bD = Check_AnyInput(ENUM_CLASS(KEYINPUT::D));

    if (bW && bA)      return ACTORDIR::LU;
    else if (bW && bD) return ACTORDIR::RU;
    else if (bS && bA) return ACTORDIR::LD;
    else if (bS && bD) return ACTORDIR::RD;
    else if (bW)       return ACTORDIR::U;
    else if (bS)       return ACTORDIR::D;
    else if (bA)       return ACTORDIR::L;
    else if (bD)       return ACTORDIR::R;

    return ACTORDIR::END;
}

void CCharacter::Move_By_Camera_Direction_8Way(ACTORDIR eDir, _float fTimeDelta, _float fSpeed)
{
    ASSERT_CRASH(m_pSpringCamera);
    ASSERT_CRASH(m_pTransformCom);

    _vector vLook = m_pSpringCamera->Get_LookVector_NoPitch();
    _vector vRight = m_pSpringCamera->Get_RightDirection_NoPitch();

    vLook = XMVectorSetY(vLook, 0.f);
    vRight = XMVectorSetY(vRight, 0.f);
    vLook = XMVector3Normalize(vLook);
    vRight = XMVector3Normalize(vRight);
    _vector vMoveDir = XMVectorZero();

    switch (eDir)
    {
    case ACTORDIR::U:   vMoveDir = vLook; break;        
    case ACTORDIR::D:   vMoveDir = -vLook; break;       
    case ACTORDIR::L:   vMoveDir = -vRight; break;      
    case ACTORDIR::R:   vMoveDir = vRight; break;       
    case ACTORDIR::LU:  vMoveDir = XMVector3Normalize(vLook - vRight); break;
    case ACTORDIR::LD:  vMoveDir = XMVector3Normalize(-vLook - vRight); break;
    case ACTORDIR::RU:  vMoveDir = XMVector3Normalize(vLook + vRight); break;
    case ACTORDIR::RD:  vMoveDir = XMVector3Normalize(-vLook + vRight); break;
        default: return;
    }

    vMoveDir = XMVectorSetY(vMoveDir, 0.f);
    vMoveDir = XMVector3Normalize(vMoveDir);

    // 이동 방향으로 회전 (부드러운 회전)
    m_pTransformCom->LookLerp(vMoveDir * -1.f, fTimeDelta, 10.f);

    m_pTransformCom->Go_Dir(vMoveDir * fSpeed, fTimeDelta);
    
}

// 떨어질 때 추가 값.
void CCharacter::Move_Fall(_float fTimeDelta, _float fSpeed)
{
    _vector vMoveDir = XMVectorSet(0.f, -1.f, 0.f, 0.f);
    m_pTransformCom->Go_Dir(vMoveDir * fSpeed, fTimeDelta);
}

_float CCharacter::Get_DistanceToGround()
{
    _vector vCurrentPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vStartPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vEndPos = vCurrentPos - XMVectorSet(0.f, 100.f, 0.f, 0.f); // 아래로 쏜다.

    _float4 vHitPoint = { };
    _bool bHit = m_pGameInstance->Ray_Cast(vStartPos, vEndPos, &vHitPoint);

    if (bHit)
    {
        _vector vHitPos = XMLoadFloat4(&vHitPoint);
        _vector vDistance = vCurrentPos - vHitPos;
        return XMVectorGetX(XMVector3Length(vDistance));
    }

    return 0.f; // 레이가 땅에 닿지 않으면 0.f 반환.
}


#pragma endregion




void CCharacter::Free()
{
    CActor::Free();
    Safe_Release(m_pInputControllerCom);
    Safe_Release(m_pStateMachineCom);
    Safe_Release(m_pSpringCamera);
    
}
