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

_bool CCharacterState::Play_Animation(CCharacter* pCharacter, _float fTimeDelta)
{
    return pCharacter->Play_Animation(m_Animations[m_iCurrentAnimIdx].strAnimName, fTimeDelta * m_Animations[m_iCurrentAnimIdx].fSpeed, &m_fTrackPosition
        , m_Animations[m_iCurrentAnimIdx].fRootMotionRate, m_Animations[m_iCurrentAnimIdx].IsRootMotion);
}


void CCharacterState::Free()
{
    CState::Free();
}
