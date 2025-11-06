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

void CCharacterState::OnEnter(void* pArg)
{
    CState::OnEnter(pArg);
}

void CCharacterState::OnUpdate(_float fTimeDelta)
{
}

void CCharacterState::OnExit()
{
	m_IsPartAnimationEnd = false; //
	m_IsSubPartAnimationEnd = false; //
}

_bool CCharacterState::Play_Animation(CCharacter* pCharacter, _float fTimeDelta, _float fRootMotionRate, const GPU_BLEND_INFO& gpuBlendInfo)
{
	_float fStateRootMotionRate = fRootMotionRate != 1.f ? fRootMotionRate : m_Animations[m_iCurrentAnimIdx].fRootMotionRate;
	m_IsAnimationEnd = pCharacter->Play_Animation(m_Animations[m_iCurrentAnimIdx].strAnimName, fTimeDelta * m_Animations[m_iCurrentAnimIdx].fSpeed, &m_fTrackPosition
		, fStateRootMotionRate, m_Animations[m_iCurrentAnimIdx].IsRootMotion
		, m_Animations[m_iCurrentAnimIdx].IsRootMotionRotate, m_Animations[m_iCurrentAnimIdx].IsRootMotionTranslate, gpuBlendInfo
	);

	return m_IsAnimationEnd;
}




void CCharacterState::Free()
{
    CState::Free();
}
