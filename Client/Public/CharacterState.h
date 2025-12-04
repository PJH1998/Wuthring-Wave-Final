#pragma once
#include "State.h"

NS_BEGIN(Client)

// 모든 캐릭터 State의 최상위 부모 클래스
class CCharacterState abstract : public CState
{
protected:
    explicit CCharacterState() = default;
    virtual ~CCharacterState() = default;

public:
    virtual HRESULT Initialize(class CCharacter* pOwner);
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

protected:
    _bool Play_Animation(class CCharacter* pCharacter, _float fTimeDelta, _float fRootMotionRate = 1.f);
    _bool Play_Animation_NonFacial(class CCharacter* pCharacter, _float fTimeDelta, _float fRootMotionRate = 1.f);
    _bool Play_AnimationFly(class CCharacter* pCharacter, _float fTimeDelta, _float fRootMotionRate = 1.f, const GPU_BLEND_INFO& gpuBlendInfo = G_DefaultBlendInfo);

protected:
	vector<_uint> m_ActivePartTypes; // 활성화할 PartType 들.

    _uint m_iMoveKey = {};  // 입력 키 ( State 마다 사용할 변수)
    ACTORDIR m_eDir = {};   // 방향 변수

    _float3 m_vWallNormal = {}; // 정면 방향 WallNormal
    _float3 m_vLandNormal = {};
    _uint m_iPartType = {};	   // 현재 State에서 실행해야할 PartType;
	_uint m_iSubPartType = {}; // 현재 State에서 실행해야 할 SubPartType;
	_bool m_IsPartAnimationEnd = {}; //
	_bool m_IsSubPartAnimationEnd = {}; //
	_string m_strPrevInfo = {};
	_string m_strSkillName = {};

	_float m_fRootMotionScale = { 1.f }; // 몬스터와 거리에 따라 보간해서 RootMotion Scale을 조절한다..
	_float m_fAnimationScale = { 1.f }; // 거리에 따른 루트모션 비율. * (원본 애니메이션 루트모션 비율)

    class CTransform* m_pTargetTransform = { nullptr }; // LockOn 대상 Transform
	class CCharacter* m_pOwner = { nullptr };




public:
    virtual void Free() override;
};

NS_END
