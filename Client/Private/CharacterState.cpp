#include "ClientPch.h"
#include "CharacterState.h"
#include "Character.h"

HRESULT CCharacterState::Initialize(class CGameObject* pOwner)
{
    m_iMoveKey |= static_cast<_uint>(KEYINPUT::W);
    m_iMoveKey |= static_cast<_uint>(KEYINPUT::A);
    m_iMoveKey |= static_cast<_uint>(KEYINPUT::S);
    m_iMoveKey |= static_cast<_uint>(KEYINPUT::D);



    return S_OK;
}

void CCharacterState::OnEnter()
{
    CState::OnEnter();
}

void CCharacterState::OnUpdate(_float fTimeDelta)
{
}

void CCharacterState::OnExit()
{
}



_vector CCharacterState::Calculate_Attack_Direction(CCharacter* pCharacter)
{
    // 1. 키 입력 방향 계산(카메라 기준)
    _vector vInputDirection = XMVectorZero();
    ACTORDIR eInputDir = pCharacter->Calculate_Direction();
    if (eInputDir != ACTORDIR::END);
        vInputDirection = pCharacter->Calculate_Move_Direction(eInputDir);

    // 2. Lock On 타겟 방향 계산.
    _vector vLockOnDirection = XMVectorZero();
    if (pCharacter->Is_LockOn())
        vLockOnDirection = pCharacter->Get_LookVector();
        
    return Determine_Final_Direction(pCharacter, vInputDirection, vLockOnDirection);
}

// 최종 방향 결정.
_vector CCharacterState::Determine_Final_Direction(CCharacter* pCharacter, _vector vInputDirection, _vector vLockOnDirection)
{
    // 1. LockOn 상태일 때는 타겟 방향 우선
    if (pCharacter->Is_LockOn() && XMVector3Equal(vLockOnDirection, XMVectorZero()))
        return vLockOnDirection;

    // 2. LockOn이 아닌 경우 키입력이 있으면 키 입력 우선.
    if (!XMVector3Equal(vInputDirection, XMVectorZero()))
        return vInputDirection;

    // 3. 키 입력 없고 LockOn 타겟 있으면 타겟 방향.
    if (!XMVector3Equal(vLockOnDirection, XMVectorZero()))
        return vLockOnDirection;

    // 4. 둘다 없는 경우 플레이어 Look 방향.
    _vector vPlayerLook = pCharacter->Get_LookVector();
    return XMVector3Normalize(vPlayerLook);
}

_bool CCharacterState::Play_Animation(CCharacter* pCharacter, _float fTimeDelta)
{
    m_IsAnimationEnd = pCharacter->Play_Animation(m_Animations[m_iCurrentAnimIdx].strAnimName, fTimeDelta * m_Animations[m_iCurrentAnimIdx].fSpeed, &m_fTrackPosition
        , m_Animations[m_iCurrentAnimIdx].fRootMotionRate, m_Animations[m_iCurrentAnimIdx].IsRootMotion, m_Animations[m_iCurrentAnimIdx].IsRootMotionRotate, m_Animations[m_iCurrentAnimIdx].IsRootMotionTranslate);

    return m_IsAnimationEnd;
}



void CCharacterState::Free()
{
    CState::Free();
}
