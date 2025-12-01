#include "ClientPch.h"
#include "CharacterState.h"
#include "Character.h"

HRESULT CCharacterState::Initialize(class CCharacter* pOwner)
{
    m_iMoveKey |= static_cast<_uint>(KEYINPUT::W);
    m_iMoveKey |= static_cast<_uint>(KEYINPUT::A);
    m_iMoveKey |= static_cast<_uint>(KEYINPUT::S);
    m_iMoveKey |= static_cast<_uint>(KEYINPUT::D);

	m_pOwner = pOwner;
	

    return S_OK;
}

void CCharacterState::OnEnter(void* pArg)
{
    CState::OnEnter(pArg);

	m_fRootMotionScale = 1.f; // 시작 시 초기화
	m_fAnimationScale = 1.f;
}

void CCharacterState::OnUpdate(_float fTimeDelta)
{
	CState::OnUpdate(fTimeDelta);


}

void CCharacterState::OnExit()
{
	m_IsPartAnimationEnd = false; //
	m_IsSubPartAnimationEnd = false; //

	// 현재 애니메이션을 Cleaar한다.
	if (nullptr != m_pOwner)
		m_pOwner->Clear_Animation(m_Animations.at(m_iCurrentAnimIdx).strAnimName);
}

_bool CCharacterState::Play_Animation(CCharacter* pCharacter, _float fTimeDelta, _float fRootMotionRate)
{
	_float fStateRootMotionRate = fRootMotionRate != 1.f ? fRootMotionRate : m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate;
	m_IsAnimationEnd = pCharacter->Play_Animation(m_Animations.at(m_iCurrentAnimIdx).strAnimName, fTimeDelta * m_Animations.at(m_iCurrentAnimIdx).fSpeed, &m_fTrackPosition
		, fStateRootMotionRate, m_Animations.at(m_iCurrentAnimIdx).IsRootMotion
		, m_Animations.at(m_iCurrentAnimIdx).IsRootMotionRotate, m_Animations.at(m_iCurrentAnimIdx).IsRootMotionTranslate
	);

	return m_IsAnimationEnd;
}

_bool CCharacterState::Play_AnimationFly(CCharacter* pCharacter, _float fTimeDelta, _float fRootMotionRate, const GPU_BLEND_INFO& gpuBlendInfo)
{
	_float fStateRootMotionRate = fRootMotionRate != 1.f ? fRootMotionRate : m_Animations[m_iCurrentAnimIdx].fRootMotionRate;
	m_IsAnimationEnd = pCharacter->Play_AnimationFly(m_Animations.at(m_iCurrentAnimIdx).strAnimName, fTimeDelta * m_Animations.at(m_iCurrentAnimIdx).fSpeed, &m_fTrackPosition
		, fStateRootMotionRate, m_Animations.at(m_iCurrentAnimIdx).IsRootMotion
		, m_Animations.at(m_iCurrentAnimIdx).IsRootMotionRotate, m_Animations.at(m_iCurrentAnimIdx).IsRootMotionTranslate, gpuBlendInfo
	);

	return m_IsAnimationEnd;
}




void CCharacterState::Free()
{
    CState::Free();
}
