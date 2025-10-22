#include "ClientPch.h"
#include "Player.h"
#include "InputController.h"

CPlayer::CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CActor{ pDevice, pContext }
{
}

CPlayer::CPlayer(const CPlayer& Prototype)
    : CActor(Prototype)
    
{
}

HRESULT CPlayer::Initialize_Prototype()
{
    if (FAILED(CActor::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CPlayer::Initialize_Clone(void* pArg)
{
    PLAYER_DESC* pDesc = static_cast<PLAYER_DESC*>(pArg);

    // 1. 상위 객체 초기화
    if (FAILED(CActor::Initialize_Clone(pDesc)))
        return E_FAIL;

    // 2. 하위 객체 초기화
    //m_pController = pDesc->pController;


    return S_OK;
}

void CPlayer::Priority_Update(_float fTimeDelta)
{
    CActor::Priority_Update(fTimeDelta);
}

void CPlayer::Update(_float fTimeDelta)
{
    CActor::Update(fTimeDelta);
}

void CPlayer::Late_Update(_float fTimeDelta)
{
    CActor::Late_Update(fTimeDelta);
}

void CPlayer::Render()
{
}

void CPlayer::Render_Shadow()
{
}

#pragma region STATE에서 사용
_bool CPlayer::Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion)
{
    _bool IsPlayAnimationEnd = m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, strAnimName, fTimeDelta, pTrackPosition, IsRootMotion, fRootMotionRate);
    m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
    return IsPlayAnimationEnd;
}

_bool CPlayer::Check_AnyInput(KEYINPUT eKeyInput)
{
    return _bool();
}

_bool CPlayer::Check_AllInput(KEYINPUT eKeyInput)
{
    return _bool();
}

/* 캐스팅 해서 보내야됨. */
#pragma endregion







void CPlayer::Free()
{
    CActor::Free();
    Safe_Release(m_pInputControllerCom);
    Safe_Release(m_pStateMachineCom);
}
