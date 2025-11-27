#include "ClientPch.h"
#include "Character.h"
#include "InputController.h"
#include "SpringCamera.h"
#include "GameSystem.h"
#include "Collider.h"
#include "Ability.h"
#include "AttackVolume.h"

CCharacter::CCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CActor{ pDevice, pContext }
    , m_pGameSystem { CGameSystem::GetInstance() }
{
    Safe_AddRef(m_pGameSystem);
}

CCharacter::CCharacter(const CCharacter& Prototype)
    : CActor(Prototype)
	, m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
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

    // 0. Actor 초기화
    if (FAILED(CActor::Initialize_Clone(pDesc)))
        return E_FAIL;
    // 1. State 초기화
    //m_Stats = pDesc->eStat;

	//m_EventDatas.resize(CHARACTER_EVENT_ID::EVENT_END);

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

#pragma region 객체 공유
void CCharacter::Set_InputController(CInputController* pInputControllerCom)
{
    m_pInputControllerCom = pInputControllerCom;
    Safe_AddRef(m_pInputControllerCom);
}

void CCharacter::Set_SpringCamera(CSpringCamera* pSpringCamera)
{
    m_pSpringCamera = pSpringCamera;
    Safe_AddRef(pSpringCamera);
}

// Collider 설정 완료.
void CCharacter::Set_Collider(CCollider* pColliderCom, _float3 vColliderOffset, _float fColliderHeight, _float fColliderRadius)
{
	m_pColliderCom = pColliderCom;
	Safe_AddRef(m_pColliderCom);

	m_vColliderOffSet = vColliderOffset;
	m_fColliderHeight = fColliderHeight;
	m_fColliderRadius = fColliderRadius;
}

void CCharacter::Set_Ability(CAbility* pAbilityCom)
{
	m_pAbillityCom = pAbilityCom;
	Safe_AddRef(m_pAbillityCom);
}


_float CCharacter::Get_DistanceFromGround(_float fStartYOffset)
{
	ASSERT_CRASH(m_pTransformCom);

	_vector vCurrentPos = m_pColliderCom->Get_Position(); // 캡슐 중심
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	_vector vRight = XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT));

	// 1. 캡슐의 실제 바닥 위치를 계산합니다. (중심 - 절반 높이)
	_vector vCapsuleBottom = vCurrentPos;

	// 2. fStartYOffset(0.2f) 만큼 바닥에서 띄운 위치에서 Ray를 시작합니다.
	_vector vFootPos = vCapsuleBottom + XMVectorSet(0.f, fStartYOffset, 0.f, 0.f);

	// 3. 5개 지점: 앞, 왼쪽, 중앙, 오른쪽, 뒤 (이후 로직은 동일)
	_vector vPositions[5] = {
		vFootPos + vLook * (m_fColliderRadius - 0.05f),  // 앞
		vFootPos + vRight * (m_fColliderRadius - 0.05f), // 왼쪽
		vFootPos,                              // 중앙
		vFootPos - vRight * (m_fColliderRadius - 0.05f), // 오른쪽
		vFootPos - vLook * (m_fColliderRadius - 0.05f)   // 뒤
	};

	_float fMinDistance = 3.f;  // 가장 가까운 거리 저장
	_bool bAnyHit = false;
	
	// 5개 지점에서 각각 레이 발사
	for (_uint i = 0; i < 5; ++i)
	{
		_vector vStartPos = vPositions[i];
		_vector vEndPos = vStartPos - XMVectorSet(0.f, 3.f, 0.f, 0.f);
	
		_float4 vHitPoint = {};
		_bool bHit = m_pGameInstance->Ray_Cast(vStartPos, vEndPos, &vHitPoint);
	
		if (bHit)
		{
			bAnyHit = true;
			_vector vHitPos = XMLoadFloat4(&vHitPoint);
			_vector vDistance = vPositions[i] - vHitPos;
			_float fDistance = XMVectorGetX(XMVector3Length(vDistance));
	
			// 가장 가까운 거리 저장
			if (fDistance < fMinDistance)
				fMinDistance = fDistance;
		}
	}
	return bAnyHit ? fMinDistance : 3.f;
}




_bool CCharacter::Is_LandCollider(_float3* pNormal)
{
	ASSERT_CRASH(m_pColliderCom);
	return m_pColliderCom->IsLand(pNormal);
}

// fDistanceGround (Ray 쏴서 땅에 닿은 거리가 매개변수로 받은 거리보다 크다면 => 땅이아니다)
//_bool CCharacter::Is_Land(_float fRayOffsetY, _float fLandDistance)
//{
//	_float fDistanceToGround = Get_DistanceFromGround(fRayOffsetY); // 중앙 기준 다섯방향 Ray 발사.
//
//	if (fDistanceToGround > fLandDistance)
//		return false;
//
//	return true;
//}


#pragma endregion

#pragma region PHYSICS

const _float CCharacter::Calculate_RootMotionScale()
{
	// 타겟이 없으면 원래 비율로
	if (m_pTargetTransform == nullptr)
		return 1.f;

	// 타겟이 있는 경우 거리 계산 후 RootMotionScale 조절.
	if (m_fTargetDistance < 1.f)
		return 0.05f; // 거의 이동량 없게.
	else if (m_fTargetDistance < 3.f)
		return 0.5f;  // 짧게: 과접근 방지
	else if (m_fTargetDistance < 3.5f)
		return 0.6f;  
	else if (m_fTargetDistance < 4.f)
		return 0.7f;  
	else if (m_fTargetDistance >= 7.f)
		return 1.4f;  // 길게: 빠른 접근
	
	return 1.f; // 3.f ~ 7.f 사이면? 똑같은 비율
}

_bool CCharacter::Check_ClimbableWall(_float3* pWallNormal)
{
	ASSERT_CRASH(m_pTransformCom);

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
	vLook = XMVector3Normalize(vLook);

	// 가슴 높이에서 전방 Radius로 레이 발사

	_float fOffsetY = m_fColliderHeight * 2.f + m_fColliderRadius;
	_vector vStart = vPos + XMVectorSet(0.f, fOffsetY, 0.f, 0.f); // 캡슐이니까.
	_vector vEnd = vStart + vLook * (m_fColliderRadius + 0.1f); // Collider Radius 고려.

	_float4 vHitPoint = {};
	_bool bHit = m_pGameInstance->Ray_Cast(vStart, vEnd, &vHitPoint);


	if (bHit)
	{
		XMStoreFloat3(pWallNormal, vPos - XMLoadFloat4(&vHitPoint));
		return true;
	}


	return false;
}

_bool CCharacter::Check_ClimbableWall_Above(_float fEndRayOffset, _float3* pWallNormal)
{
	ASSERT_CRASH(m_pTransformCom);

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));

	// 머리위쪽에서 정면으로 Ray 발사.
	_float fYOffset = m_fColliderHeight + m_fColliderRadius * 1.5f;
	_vector vStart = vPos + XMVectorSet(0.f, fYOffset, 0.f, 0.f);
	_vector vEnd = vStart + vLook * (m_fColliderRadius + fEndRayOffset);

	_float4 vHitPoint = {};

	return m_pGameInstance->Ray_Cast(vStart, vEnd, &vHitPoint);
}

void CCharacter::Set_Gravity(_bool IsGravity)
{
	ASSERT_CRASH(m_pColliderCom);
	

	if (!m_IsQTE)
		m_pColliderCom->Set_Gravity(IsGravity);
	else
	{
		ASSERT_CRASH(m_pQTEColliderCom);
		m_pQTEColliderCom->Set_Gravity(IsGravity);
	}
		
}



void CCharacter::Set_ColliderReferenceBone(const _string& strBoneName, _float3 vOffset)
{
	m_strColliderReferenceBone = strBoneName;

	if (strBoneName.empty())
	{
		m_pColliderCom->Sync_Position(m_pTransformCom);
		m_pColliderCom->Set_Offset(m_vColliderOffSet);    // 원본 오프셋으로 변경.
		return;
	}

	// 1. RootBone의 위치 가져오기.
	_matrix RootMatrix = XMLoadFloat4x4(m_pModelCom->Get_BoneMatrixPtr("Root"));
	_matrix TargetMatrix = XMLoadFloat4x4(m_pModelCom->Get_BoneMatrixPtr(strBoneName.c_str()));

	// 2. Root 본의 로컬 위치
	_vector vRootPos = RootMatrix.r[3];
	// 3. Target 본의 로컬 위치
	_vector vTargetPos = TargetMatrix.r[3];
	_vector vBoneOffset = (vTargetPos - vRootPos) * 0.01f - XMLoadFloat3(&vOffset);

	_float3 vNewOffset = {};
	XMStoreFloat3(&vNewOffset, XMLoadFloat3(&m_vColliderOffSet) + vBoneOffset); // 차이만큼 더한다.

	m_pColliderCom->Set_Offset(vNewOffset);

	m_vAnimColliderOffset = vOffset;

}

void CCharacter::Sync_Collider(_fvector vVelocity, _float fTimeDelta)
{
	ASSERT_CRASH(m_pColliderCom);
	m_pColliderCom->Update(vVelocity);
}

// Collider Velocity
_fvector CCharacter::Get_Velocity()
{
	ASSERT_CRASH(m_pTransformCom);
	return m_pTransformCom->Get_Velocity();
}

void CCharacter::Add_Force(_fvector vForce, _float fTimeDelta)
{
	ASSERT_CRASH(m_pTransformCom);
	m_pTransformCom->Go_Force(vForce, fTimeDelta);
}

_matrix CCharacter::Get_WorldMatrix()
{
	ASSERT_CRASH(m_pTransformCom);
	return m_pTransformCom->Get_WorldMatrix();
}

#pragma endregion

#ifdef _DEBUG
void CCharacter::Print_LookRay()
{
	_vector vStartPos = m_pTransformCom->Get_State(STATE::POSITION) + (XMVector3Normalize(m_pTransformCom->Get_State(STATE::UP)) * 0.3f);
	_vector vEndPos = vStartPos + XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)) * 1.5f;
	

	m_pGameInstance->Ray_Cast(vStartPos, vEndPos, nullptr);
}

#endif // _DEBUG

#pragma region STATE

void CCharacter::Render_Damage(const HIT_DESC* pDesc)
{
}

void CCharacter::Begin_Toggle_SFX(SFX_TOGGLE eType, _float fDuration)
{
	m_pGameInstance->Begin_Toggle_SFX(eType, fDuration);
}

void CCharacter::End_SFX()
{
	m_pGameInstance->End_SFX();
}

void CCharacter::Spawn_Effect(const _wstring& wStrEffectTag)
{
	PREFAB_INFO EffectDesc{};
	EffectDesc.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	EffectDesc.pModelPtr = m_pModelCom;
	
	_matrix mat = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, mat, &EffectDesc);
}

// 내 Velocity 고정.
void CCharacter::Camera_Shake(_float fIntensity)
{

	//_float3 vDir = {0.5f, 0.1f, -0.1f};
	//
	//m_pGameInstance->OnShake(vDir);
}

void CCharacter::Play_Action(const _wstring& strActionTag, _bool isEscape)
{
	if (nullptr == m_pTransformCom)
		return;

	m_pGameSystem->Play_Action(strActionTag, m_pTransformCom->Get_WorldMatrix(), false, isEscape);
}

_bool CCharacter::Check_AnyConidtion_FromAbility(_uint iCondition)
{
	if (nullptr == m_pAbillityCom)
		return false;

	return m_pAbillityCom->Check_AnyCondition(iCondition);
}


void CCharacter::Bind_GrappleTarget(CTransform* pTargetTransform, OBJECTTYPE eObjectType)
{
	m_pTargetGrappleTransform = pTargetTransform;
	m_eTargetGrappleType = eObjectType;

	_vector vPos = m_pTargetGrappleTransform->Get_State(STATE::POSITION);
	m_pGameInstance->IsIn_WorldSpace(vPos, 20.f);
}


_bool CCharacter::Is_MoveGrapple()
{
	return _bool();
}

void CCharacter::Bind_Condition_ToAbillity(_uint iCondition)
{
	if (nullptr == m_pAbillityCom)
		return;

	m_pAbillityCom->Bind_Condition(iCondition);
}

void CCharacter::Remove_Condition_ToAbillity(_uint iCondition)
{
	if (nullptr == m_pAbillityCom)
		return;

	m_pAbillityCom->Remove_Condition(iCondition);
}

void CCharacter::Bind_CostCondition_ToAbility(_uint iCondition, _uint iConditionFlag)
{
	if (nullptr == m_pAbillityCom)
		return;

	m_pAbillityCom->Bind_CostCondition(iCondition, iConditionFlag);
}

_vector CCharacter::Get_LookVector()
{
    _vector vLook = XMVectorZero();

    if (nullptr == m_pTransformCom)
        return vLook;

    vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
 
    return vLook;
}

_vector CCharacter::Get_CameraLookVector()
{
	_vector vLook = XMVectorZero();

	if (nullptr == m_pSpringCamera)
		return vLook;

	vLook = XMVector3Normalize(m_pSpringCamera->Get_LookVector());

	return vLook;
}

_vector CCharacter::Get_LookVector_NoPitch()
{
	_vector vLook = XMVectorZero();

	if (nullptr == m_pTransformCom)
		return vLook;

    vLook = XMVector3Normalize(XMVectorSetY(Get_LookVector(), 0.f));
    return vLook;
}

_vector CCharacter::Get_RightVector()
{
	_vector vRight = XMVectorZero();

	if (nullptr == m_pTransformCom)
		return vRight;

	vRight = XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT));

	return vRight;
}

_vector CCharacter::Get_RightVector_NoPitch()
{
	_vector vRight = XMVectorZero();

	if (nullptr == m_pTransformCom)
		return vRight;

	vRight = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::RIGHT), 0.f));

	return vRight;
}


void CCharacter::Set_AutoLockOn(CTransform* pTargetTransform, _bool IsLockOn)
{
	// 1. TargetTransform은 항상 가져옵니다.
	m_pTargetTransform = pTargetTransform;
	m_pLockOnTargetTransform = nullptr; // 들어왔다는건 하드 락온이 풀렸다는것.

	if (nullptr == pTargetTransform)
	{
		//m_pTargetTransform = pTargetTransform; // 매프레임 TargetTransform 제거.

		if (m_IsLockOn) {
			m_IsLockOn = false;
			/*m_pTargetTransform = nullptr;*/

			// LockOn 해제 시 한 번만
			if (m_pSpringCamera)
			{
				_vector vCamLook = m_pSpringCamera->Get_LookVector();
				vCamLook = XMVectorSetY(vCamLook, 0.f);
				vCamLook = XMVector3Normalize(vCamLook);
				m_pTransformCom->LookLerp(vCamLook, 0.1f, 30.f);
			}
		}
		return;
	}
	else
	{
		// 2. LockOn은 상황따라
		m_IsLockOn = IsLockOn;
		
	}


	return;
}

void CCharacter::Set_LockOn(CTransform* pTargetTransform, _bool IsLockOn)
{
	if (nullptr == pTargetTransform)
	{
		m_IsLockOn = false;
		m_pTargetTransform = nullptr;

		// LockOn 해제 시: 카메라 Look_NoPitch으로 즉시 회전 (반대 방향 착시 완전 해결)
		if (m_pSpringCamera)
		{
			_vector vCamLook = m_pSpringCamera->Get_LookVector();
			vCamLook = XMVectorSetY(vCamLook, 0.f);
			vCamLook = XMVector3Normalize(vCamLook);
			m_pTransformCom->LookDir(vCamLook);
		}
		return;
	}
	else
	{
		// LockOn을 받아옵니다.
		m_IsLockOn = IsLockOn;

		// LockOn용 타겟을 받습니다.
		m_pLockOnTargetTransform = pTargetTransform;
		m_pTargetTransform = pTargetTransform;
	}
}

_bool CCharacter::Is_LockOn()
{
    return m_IsLockOn;
}




_bool CCharacter::Check_AnyInput(_uint iKeyFlag, KEYSTATE eKeyState)
{
    ASSERT_CRASH(m_pInputControllerCom);
    return m_pInputControllerCom->Check_AnyInput(iKeyFlag, eKeyState);
}

_bool CCharacter::Check_AllInput(_uint iKeyFlag, KEYSTATE eKeyState)
{
    ASSERT_CRASH(m_pInputControllerCom);
    return m_pInputControllerCom->Check_AllInput(iKeyFlag, eKeyState);
}


_bool CCharacter::Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate)
{
    ASSERT_CRASH(m_pModelCom);
    //_bool IsPlayAnimationEnd = m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, strAnimName, fTimeDelta, pTrackPosition, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate, fRootMotionRate);
    _bool IsPlayAnimationEnd = m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, m_pFacialComputeShaderCom, strAnimName, fTimeDelta, pTrackPosition, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate, fRootMotionRate);
    m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
	
    return IsPlayAnimationEnd;
}


// Fly Animation 전용.
_bool CCharacter::Play_AnimationFly(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate, const GPU_BLEND_INFO& gpuBlendInfo)
{
	ASSERT_CRASH(m_pModelCom);
	//_bool IsPlayAnimationEnd = m_pModelCom->Play_FlyAnimation_GPU(m_pFlyComputeShaderCom, strAnimName, fTimeDelta, pTrackPosition, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate, fRootMotionRate, gpuBlendInfo);
	_bool IsPlayAnimationEnd = m_pModelCom->Play_FlyAnimation_GPU(m_pFlyComputeShaderCom, m_pFacialComputeShaderCom, strAnimName, fTimeDelta, pTrackPosition, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate, fRootMotionRate, gpuBlendInfo);
	m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);

	return IsPlayAnimationEnd;
}




void CCharacter::Change_State(_uint iCategory, _uint iSubState, void* pArg)
{
    ASSERT_CRASH(m_pStateMachineCom);
    m_pStateMachineCom->Change_State(iCategory, iSubState, pArg);
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

_vector CCharacter::Get_CameraRightVector()
{
	ASSERT_CRASH(m_pSpringCamera);

	return m_pSpringCamera->Get_RightVector_NoPitch();

}

_vector CCharacter::Calculate_Move_Direction(ACTORDIR eDir)
{
    ASSERT_CRASH(m_pSpringCamera);

    _vector vLook = m_pSpringCamera->Get_LookVector_NoPitch();
    _vector vRight = m_pSpringCamera->Get_RightVector_NoPitch();

    switch (eDir)
    {
    case ACTORDIR::U:   return vLook;
    case ACTORDIR::D:   return -vLook;
    case ACTORDIR::L:   return -vRight;
    case ACTORDIR::R:   return vRight;
    case ACTORDIR::LU:  return XMVector3Normalize(vLook - vRight);
    case ACTORDIR::LD:  return XMVector3Normalize(-vLook - vRight);
    case ACTORDIR::RU:  return XMVector3Normalize(vLook + vRight);
    case ACTORDIR::RD:  return XMVector3Normalize(-vLook + vRight);
    default: return XMVectorZero();
    }

    return XMVectorZero();
}

_vector CCharacter::Calculate_LockOn_Move_Direction(ACTORDIR eDir)
{
	if (nullptr == m_pTargetTransform)
		return XMVectorZero();

	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = m_pTargetTransform->Get_State(STATE::POSITION);

	_vector vToTarget = XMVector3Normalize(vTargetPos - vMyPos);
	vToTarget = XMVectorSetY(vToTarget, 0.f);

	_vector vTargetRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vToTarget));

	switch (eDir)
	{
	case ACTORDIR::U:   return -vToTarget;  
	case ACTORDIR::D:   return vToTarget;   
	case ACTORDIR::L:   return -vTargetRight; 
	case ACTORDIR::R:   return vTargetRight;  
	case ACTORDIR::LU:  return XMVector3Normalize(-vToTarget - vTargetRight);
	case ACTORDIR::LD:  return XMVector3Normalize(vToTarget - vTargetRight);
	case ACTORDIR::RU:  return XMVector3Normalize(-vToTarget + vTargetRight);
	case ACTORDIR::RD:  return XMVector3Normalize(vToTarget + vTargetRight);
	default: return XMVectorZero();
	}
}

// LockOn 시 이동
void CCharacter::Move_LockOn_8Way(ACTORDIR eDir, _float fTimeDelta, _float fSpeed)
{
    if (!m_IsLockOn)
        return;

    ASSERT_CRASH(m_pSpringCamera);
    ASSERT_CRASH(m_pTransformCom);

	/*_vector vMoveDir = Calculate_LockOn_Move_Direction(eDir);
	m_pTransformCom->Go_Dir(vMoveDir * fSpeed, fTimeDelta);*/

    // 1. 회전.
	Rotate_Target_Lerp(fTimeDelta);

	// 2. 이동 방향.
    _vector vMoveDir = Calculate_Move_Direction(eDir);

    // 3. 이동 적용  
    m_pTransformCom->Go_Dir(vMoveDir * fSpeed, fTimeDelta);
}

void CCharacter::Move_By_Camera_Direction_8Way(ACTORDIR eDir, _float fTimeDelta, _float fSpeed)
{
    ASSERT_CRASH(m_pSpringCamera);
    ASSERT_CRASH(m_pTransformCom);

    _vector vMoveDir = Calculate_Move_Direction(eDir);

    // 이동 방향으로 회전 (부드러운 회전)
    m_pTransformCom->LookLerp(vMoveDir, fTimeDelta, 10.f);
    m_pTransformCom->Go_Dir(vMoveDir * fSpeed, fTimeDelta);
}

// 떨어질 때 추가 값.
void CCharacter::Move_Fall(_float fTimeDelta, _float fSpeed)
{
    _vector vMoveDir = XMVectorSet(0.f, -1.f, 0.f, 0.f);
    m_pTransformCom->Go_Dir(vMoveDir * fSpeed, fTimeDelta);
}

void CCharacter::Move_Direction(_fvector vDir, _float fTimeDelta, _float fSpeed)
{
    m_pTransformCom->Go_Dir(vDir * fSpeed, fTimeDelta);
}

void CCharacter::Rotate_Direction(_fvector vDir)
{
    _vector vDirFlat = XMVectorSetY(vDir, 0.f);
    vDirFlat = XMVector3Normalize(vDirFlat);

    if (XMVector3Equal(vDirFlat, XMVectorZero()))
        return;

    m_pTransformCom->LookDir(vDirFlat);
}

void CCharacter::Rotate_DirectionNoPitchLerp(_fvector vDir, _float fTimeDelta, _float fSpeed)
{
    _vector vDirFlat = XMVectorSetY(vDir, 0.f);
    vDirFlat = XMVector3Normalize(vDirFlat);

    if (XMVector3Equal(vDirFlat, XMVectorZero()))
        return;

    m_pTransformCom->LookLerp(vDirFlat, fTimeDelta, fSpeed);
}

void CCharacter::Rotate_DirectionLerp(_fvector vDir, _float fTimeDelta, _float fSpeed)
{

	_vector vDirFlat = XMVector3Normalize(vDir);
	if (XMVector3Equal(vDirFlat, XMVectorZero()))
		return;

	m_pTransformCom->LookLerp(vDirFlat, fTimeDelta, fSpeed);
}



void CCharacter::Rotate_Target()
{
    // 1. 타겟이 없는 경우 Return
    if (nullptr == m_pTargetTransform)
        return;

    // 2. 타겟이 있으면 즉시 회전.
    _vector vTarget = m_pTargetTransform->Get_State(STATE::POSITION);
    _vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vToTarget = XMVector3Normalize(vTarget - vMyPos);


    vToTarget = XMVectorSetY(vToTarget, 0.f);
    m_pTransformCom->LookDir(vToTarget); // 이동은 바로 회전. => Idle 되면 Lerp로

    return;
}

void CCharacter::Rotate_Target_Lerp(_float fTimeDelta)
{
	
	if (nullptr == m_pTargetTransform || nullptr == m_pTransformCom)
		return;

	_vector vTarget = m_pTargetTransform->Get_State(STATE::POSITION);
	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vToTarget = XMVector3Normalize(vTarget - vMyPos);
	vToTarget = XMVectorSetY(vToTarget, 0.f);

	m_pTransformCom->LookLerp(vToTarget, fTimeDelta, 15.f);
}


void CCharacter::Rotate_HitTarget(CTransform* pTransform)
{
    // 1. 타겟이 없는 경우 Return
    if (nullptr == pTransform)
        return;

    // 2. 타겟이 있으면 즉시 회전.
    _vector vTarget = pTransform->Get_State(STATE::POSITION);
    _vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vToTarget = XMVector3Normalize(vTarget - vMyPos);

	// 3. 타겟이랑 나랑 곂쳤을때 안전코드

	_float fLengthSq = XMVectorGetX(XMVector3LengthSq(vToTarget));
	if (fLengthSq < 1e-6f) // Epsilon으로 판단.
	{
		return;  // 곂침시 그냥 기본 Forward로 판단.
	}

	
    vToTarget = XMVectorSetY(vToTarget, 0.f);

	

    m_pTransformCom->LookDir(vToTarget); // 이동은 바로 회전. => Idle 되면 Lerp로

    return;
}



void CCharacter::Sync_Transform_FromPlayer(_fmatrix WorldMatrix, _fvector vPrevVeloctiy, _float fTimeDelta)
{
	ASSERT_CRASH(m_pTransformCom);
	ASSERT_CRASH(m_pColliderCom);

	if (m_IsQTE)
		return;
	
	// 0. World Matrix
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);  // 위치 설정
}

void CCharacter::Sync_Transform_ToPlayer(CTransform* pTransformCom)
{
	if (m_IsQTE)
		return;

	_matrix mat = m_pTransformCom->Get_WorldMatrix();
	pTransformCom->Set_WorldMatrix(mat);
}

void CCharacter::Sync_UtilityType_FromPlayer(UI_TAB_UTILITY eInteractionType)
{
	m_eInteractionType = eInteractionType;
}

#ifdef _DEBUG
void CCharacter::Debug_FullCost(_bool IsAll)
{
	if (nullptr == m_pAbillityCom)
		return;

	m_pAbillityCom->Debug_FullCost(IsAll);
}

void CCharacter::Clear_CoolTime()
{
	if (nullptr == m_pAbillityCom)
		return;

	m_pAbillityCom->Clear_CoolTime();
}

#else
void CCharacter::Debug_FullCost(_bool IsAll)
{
	if (nullptr == m_pAbillityCom)
		return;

	m_pAbillityCom->Debug_FullCost(IsAll);
}
void CCharacter::Clear_CoolTime()
{
	if (nullptr == m_pAbillityCom)
		return;

	m_pAbillityCom->Clear_CoolTime();
}

#endif // _DEBUG




#pragma endregion

#pragma region UI
void CCharacter::Ability_Update(_float fTimeDelta)
{
	if (nullptr == m_pAbillityCom)
		return;

	m_pAbillityCom->Update(fTimeDelta);
}
CAbility* CCharacter::Get_AbilityCom()
{
    return m_pAbillityCom;
}
_float CCharacter::Get_Cost(COST_TYPE eCostType)
{
	if (nullptr == m_pAbillityCom)
		return 0.f;

	return m_pAbillityCom->Get_Cost(eCostType);
}
_float CCharacter::Get_MaxCost()
{
	if (nullptr == m_pAbillityCom)
		return 0.f;

	return 100.f;
}
void CCharacter::Sync_UI()
{
    // Character Info Sync 
    //m_pGameSystem->Sync_CharacterInfo(m_Stats);
}

#pragma endregion

#pragma region CONDITION

void CCharacter::Add_Condition(_uint iConditionFlag)
{
	m_iCondition |= iConditionFlag;
}

_bool CCharacter::Check_AnyCondition(_uint iConditionFlag)
{
	return (m_iCondition & iConditionFlag) != 0;
}

_bool CCharacter::Check_AllCondition(_uint iConditionFlag)
{
	return (m_iCondition & iConditionFlag) == iConditionFlag;
}


void CCharacter::Remove_Condition(_uint iConditionFlag)
{
	m_iCondition &= ~iConditionFlag;
}

void CCharacter::Remove_AllCondition()
{
	m_iCondition = 0;
}

void CCharacter::Sync_Condition_ToPlayer(_uint* pCondition)
{
	if (nullptr == pCondition)
		return;

	*pCondition = m_iCondition; // 값 넣어주기.
}

void CCharacter::Add_Condition_FromPlayer(_uint iCondition)
{
	m_iCondition |= iCondition;
}

void CCharacter::Remove_Condition_FromPlayer(_uint iCondition)
{
	m_iCondition &= ~iCondition;
}



#pragma endregion





void CCharacter::Free()
{
    CActor::Free();
    Safe_Release(m_pGameSystem);
    Safe_Release(m_pInputControllerCom);
    Safe_Release(m_pSpringCamera);
    Safe_Release(m_pStateMachineCom);
	Safe_Release(m_pQTEColliderCom);
	Safe_Release(m_pFlyComputeShaderCom);
	Safe_Release(m_pFacialComputeShaderCom);

	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			Safe_Release(pAttackVolume);
	}
	

	m_AttackVolumes.clear();
	
}
