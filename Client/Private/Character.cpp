#include "ClientPch.h"
#include "Character.h"
#include "InputController.h"
#include "SpringCamera.h"
#include "GameSystem.h"
#include "Collider.h"
#include "Ability.h"
#include "AttackVolume.h"
#include "MotionTrail.h"

CCharacter::CCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CActor{ pDevice, pContext }
    , m_pGameSystem { CGameSystem::GetInstance() }
{
    Safe_AddRef(m_pGameSystem);
}

CCharacter::CCharacter(const CCharacter& Prototype)
    : CActor(Prototype)
	, m_pGameSystem{ CGameSystem::GetInstance() }
	, m_vOutlineColor { Prototype.m_vOutlineColor }
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

	// 1. Rope Action 거리 초기화 (모든 캐릭 공통)
	m_fHookRange = pDesc->fHookRange;
	m_fDragRange = pDesc->fDragRange;
	m_fReachedHook = pDesc->fReacedRopeHook;

	// 2. 잡기 가능한 거리 초기화 (모든 캐릭 공통)
	m_fThrowRange = pDesc->fThrowRange; 

	// 3. 그랩 용도 Matrix
	XMStoreFloat4x4(&m_GrabComibinedMatrix, XMMatrixIdentity());

	

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
	ASSERT_CRASH(pInputControllerCom);
    m_pInputControllerCom = pInputControllerCom;
    Safe_AddRef(m_pInputControllerCom);
}

void CCharacter::Set_SpringCamera(CSpringCamera* pSpringCamera)
{
	ASSERT_CRASH(pSpringCamera);
    m_pSpringCamera = pSpringCamera;
	m_fCameraOriginDistance = m_pSpringCamera->Get_Distance();
	m_fCaemraDistance = m_fCameraOriginDistance;
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
#pragma endregion

#pragma region PHYSICS

const _float CCharacter::Calculate_RootMotionScale()
{
	// 타겟이 없으면 원래 비율로
	if (m_pTargetTransform == nullptr)
		return 1.f;

	// 타겟이 있는 경우 거리 계산 후 RootMotionScale 조절.
	if (m_fTargetDistance < 1.f)
		return 0.05f;
	else if (m_fTargetDistance < 3.f)
		return 0.5f;
	else if (m_fTargetDistance < 3.5f)
		return 0.6f;  
	else if (m_fTargetDistance < 4.f)
		return 0.7f;  
	else if (m_fTargetDistance >= 7.f)
		return 1.4f;
	
	return 1.f;
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

void CCharacter::Set_Position(_fvector vPos)
{
	ASSERT_CRASH(m_pTransformCom);
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
}

void CCharacter::ColliderActive(_bool IsActive)
{
	ASSERT_CRASH(m_pColliderCom);
	m_pColliderCom->IsActivate(IsActive);
}

#pragma endregion

#ifdef _DEBUG
void CCharacter::Print_LookRay()
{
	_vector vStartPos = m_pTransformCom->Get_State(STATE::POSITION) + (XMVector3Normalize(m_pTransformCom->Get_State(STATE::UP)) * 0.3f);
	_vector vEndPos = vStartPos + XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)) * 1.5f;
	

	m_pGameInstance->Ray_Cast(vStartPos, vEndPos, nullptr);
}

void CCharacter::Debug_ImGui()
{
	ImGui::Begin("Begin Character");

	static float vColor[3] = { 0.f, 0.f, 0.f};
	ImGui::SliderFloat3("Motion Trail Color", vColor, 0.f, 1.f);

	m_vMotionTrailColor.x = vColor[0];
	m_vMotionTrailColor.y = vColor[1];
	m_vMotionTrailColor.z = vColor[2];

	ImGui::End();
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

void CCharacter::Spawn_SFX(const _wstring& strSFXTag)
{
	_matrix mat = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(strSFXTag, mat, nullptr);
}

void CCharacter::Spawn_Effect(const _wstring& wStrEffectTag)
{
	PREFAB_INFO EffectDesc{};
	EffectDesc.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	EffectDesc.pModelPtr = m_pModelCom;
	
	_matrix mat = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, mat, &EffectDesc);
}




void CCharacter::Spwan_RopeEffect(const _wstring& wStrEffectTag, const _string& strBoneName)
{
	if (nullptr == m_GrappleInfo.pTransform||
		false == m_IsRopeActive)
		return;

	_float3 vPos = {};
	XMStoreFloat3(&vPos, m_GrappleInfo.pTransform->Get_State(STATE::POSITION));

	ROPE_INFO RopeInfo{};
	RopeInfo.pPlayerMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	RopeInfo.pBoneMatrixPtr = m_pModelCom->Get_BoneMatrixPtr(strBoneName.c_str());
	RopeInfo.vRopeObjectPos = vPos;
	RopeInfo.pIsActive = &m_IsRopeActive;

	_matrix mat = XMMatrixIdentity();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, mat, &RopeInfo);
}

void CCharacter::Spawn_LeviatanAnchorEffect(const _wstring& wStrEffectTag)
{
	PREFAB_INFO EffectDesc{};
	EffectDesc.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	EffectDesc.pModelPtr = m_pModelCom;
	EffectDesc.pActive = &m_IsLeviatanQTE;

	_matrix mat = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, mat, &EffectDesc);
}

void CCharacter::Execute_Telport(_vector vPos)
{
	if (nullptr == m_pTransformCom ||
		nullptr == m_pColliderCom)
		return;

	vPos = XMVectorSetW(vPos, 1.f);

	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	m_pTransformCom->Save_PreviousPosition();
	m_pColliderCom->Set_Position(m_pTransformCom->Get_State(STATE::POSITION));
}

void CCharacter::Reserve_LandSlide(const SLIDE_DATA& eData)
{
	// 1. 데이터 복사.
	m_PendingSlideData = eData;

	// 2. 예약 플래그 설정.
	Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::LANDSLIDE_READY));
}

void CCharacter::Bind_GrabEscapePossible()
{
	// 컨디션이 Grab이 아니라면? 호출 정지.
	if (!Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::GRABED)))
		return;

	Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::GRABRELEASE));
}

void CCharacter::Bind_GrabEscapeExecute()
{
	if (!Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::GRABED)))
		return;

	Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::GRABED));
	//ResetPose();
}

void CCharacter::Bind_GrabVisible(_bool IsVisible)
{
	if (!Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::GRABED)))
		return;

	Set_Visible(IsVisible);
}

void CCharacter::ResetPose()
{
	if (nullptr == m_pTransformCom)
		return;

	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
	vLook = XMVectorSetY(vLook, 0.f); // 하늘/바닥 보는 성분 제거

	if(XMVector3Equal(vLook, XMVectorZero()))
		vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);

	vLook = XMVector3Normalize(vLook);

	// (B) 월드 기준 Up 벡터 (0, 1, 0)
	_vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	_vector vRight = XMVector3Cross(vWorldUp, vLook);
	vRight = XMVector3Normalize(vRight);

	// (D) Up 벡터 다시 계산 (Look x Right) -> 수직 보장
	_vector vUp = XMVector3Cross(vLook, vRight);
	vUp = XMVector3Normalize(vUp);

	// (E) Transform에 적용 (이제 캐릭터는 똑바로 서게 됨)
	m_pTransformCom->Set_State(STATE::RIGHT, vRight);
	m_pTransformCom->Set_State(STATE::UP, vUp);
	m_pTransformCom->Set_State(STATE::LOOK, vLook);
}

void CCharacter::Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate, _float fDuration)
{
	if (nullptr == m_pGameInstance)
		return;

	m_pGameInstance->Change_TimeRate(strTimerTag, fTimeRate, fDuration);
}

void CCharacter::Change_TimeRatio_ToLayer(COLLISIONLAYER eCollisionLayer, _float fTimeRatio, _float fDuration)
{
	if (nullptr == m_pGameSystem)
		return;

	//m_pGameSystem->Change_TimeRate(COLLISIONLAYER::ENEMY, 0.1f, 10.f);
	m_pGameSystem->Change_TimeRate(eCollisionLayer, fTimeRatio, fDuration);
}

void CCharacter::Change_TimeRatio_ToLayer(COLLISIONLAYER eCollisionLayer, _float fTimeRatio)
{
	if (nullptr == m_pGameSystem)
		return;

	//m_pGameSystem->Change_TimeRate(COLLISIONLAYER::ENEMY, 0.1f, 10.f);
	m_pGameSystem->Change_TimeRate(eCollisionLayer, fTimeRatio);
}

void CCharacter::Spawn_MotionTrail(_float fDuration, _float fInterval, _float fMotionLifeTime, _float4 vColor, _uint iShaderPath)
{
	CMotionTrail::MOTION_TRAIL_DESC Desc = {};
	Desc.pModel = m_pModelCom;
	Desc.pTransform = m_pTransformCom;
	Desc.vColor = vColor;
	Desc.fMotionLifeTime = fMotionLifeTime;
	Desc.fInterval = fInterval;
	Desc.fDuration = fDuration;
	Desc.iShaderPassIndex = iShaderPath; 
	m_pGameInstance->Spawn_PoolingObject_ForStatic(TEXT("Pooling_GameObject_MotionTrail"), XMMatrixIdentity(), &Desc);
}



void CCharacter::Use_Spring(_float fDestination, _float fDuration)
{
	if (nullptr == m_pSpringCamera)
		return;

	m_pSpringCamera->Use_Spring(fDestination, fDuration);
}

void CCharacter::Stop_Anim()
{
	if (nullptr == m_pGameSystem)
		return;
	Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::ANIMSTOP));
	m_pGameSystem->Change_TimeRate(COLLISIONLAYER::PLAYER, 0.f);
}

void CCharacter::Start_Anim()
{
	if (nullptr == m_pGameSystem)
		return;
	Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::ANIMSTOP));
	m_pGameSystem->Change_TimeRate(COLLISIONLAYER::PLAYER, 1.f);
}

void CCharacter::Play_Sound(const _wstring& strSoundTag, CHANNEL eChannel, _float fVolume, _float fFrequency)
{
	m_pGameInstance->Play_Sound(strSoundTag, ENUM_CLASS(eChannel), fVolume, fFrequency);
}

void CCharacter::Stop_Sound(CHANNEL eChannel)
{
	m_pGameInstance->Stop_Sound(ENUM_CLASS(eChannel));
	
}

_float CCharacter::Rand(_float fMin, _float fMax)
{
	return m_pGameInstance->Rand(fMin, fMax);
}



// 내 Velocity 고정.
void CCharacter::Camera_Shake(_float fIntensity)
{

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

UI_TAB_UTILITY CCharacter::Get_UtilityType()
{
	 return m_eUtilityType;
}


// 날아갈 놈들.
_bool CCharacter::Is_GrappleHook()
{
	// 1. 예외 조건 처리.
	if ((nullptr == m_GrappleInfo.pTransform) || 
		(OBJECTTYPE::ROPE_ANCHOR != m_GrappleInfo.eObjectType) ||
		false == m_GrappleInfo.IsActive)
		return false;

	_vector vTargetPos = m_GrappleInfo.pTransform->Get_State(STATE::POSITION);

	// 2. 타겟의 거리가 카메라 Frustum 내부에서 거리가 20.f 이내인경우?
	_bool IsFrustum = m_pGameInstance->IsIn_WorldSpace(vTargetPos, 5.f);

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_float fDistance = XMVectorGetX(XMVector3Length(vTargetPos - vPos));

	return IsFrustum && fDistance <= m_fHookRange;
}

// 당길 놈들.
_bool CCharacter::Is_GrappleDrag()
{
	// 1. 예외 조건 처리. 
	if ((nullptr == m_GrappleInfo.pTransform) ||
		(OBJECTTYPE::ROPE_PULL != m_GrappleInfo.eObjectType) ||
		false == m_GrappleInfo.IsActive)
		return false;

	// 2. 카메라 Frustum 안에 있는가?
	_vector vTargetPos = m_GrappleInfo.pTransform->Get_State(STATE::POSITION);
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_bool IsFrustum = m_pGameInstance->IsIn_WorldSpace(vTargetPos, 5.f);

	// 3. 거리가 지정한 거리 이내인가?
	_float fDistance = XMVectorGetX(XMVector3Length(vTargetPos - vPos));
	return IsFrustum && fDistance <= m_fDragRange;
}

// Zip 로프액션 이후에 겹쳐지는 경우를 판단.
_bool CCharacter::Is_ReachedGrappleHook()
{
	if ((nullptr == m_GrappleInfo.pTransform) || 
		(OBJECTTYPE::ROPE_ANCHOR != m_GrappleInfo.eObjectType) ||
		false == m_GrappleInfo.IsActive)
		return false;

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = m_GrappleInfo.pTransform->Get_State(STATE::POSITION);

	_float fLength = XMVectorGetX(XMVector3Length(vPos - vTargetPos));
	
	return fLength <= m_fReachedHook;
}


void CCharacter::Bind_GrappleTarget(const GRAPPLE_INFO& grapInfo)
{
	m_GrappleInfo = grapInfo;
}

// Is_MoveGrapple이 True 인 경우에만 호출한다.
void CCharacter::Rotate_GrappleTarget()
{
	if (nullptr == m_GrappleInfo.pTransform)
		return;

	_vector vTarget = m_GrappleInfo.pTransform->Get_State(STATE::POSITION);
	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vToTarget = XMVector3Normalize(XMVectorSetY(vTarget - vMyPos, 0.f));

	m_pTransformCom->LookDir(vToTarget); // 이동은 바로 회전. => Idle 되면 Lerp로
}

void CCharacter::Move_Grapple(_float fTimeDelta, _float fSpeed)
{
	if (nullptr == m_GrappleInfo.pTransform)
		return;

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = m_GrappleInfo.pTransform->Get_State(STATE::POSITION);
	_vector vMoveDir = XMVector3Normalize(vTargetPos - vPos);
	m_pTransformCom->Go_Dir(vMoveDir * fSpeed, fTimeDelta);
}

// RopePull Trigger 실행. => 한번만 실행.
void CCharacter::Execute_RopeDragTrigger()
{
	// 1. nullptr 이고 Pull 타입이 아니라면?
	if (nullptr == m_GrappleInfo.pTransform || OBJECTTYPE::ROPE_PULL != m_GrappleInfo.eObjectType)
		return;

	m_pGameSystem->OnTriggerActivate(*m_GrappleInfo.pTriggerIndex);
}

_float CCharacter::Get_GrappleDistance()
{
	if (nullptr == m_GrappleInfo.pTransform)
		return 0.f;

	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = m_GrappleInfo.pTransform->Get_State(STATE::POSITION);
	_float fDistance = XMVectorGetX(XMVector3Length(vTargetPos - vMyPos));

	return fDistance;
}

// 로프 방향 연산.
ROPEDIR CCharacter::Calculate_RopeDirection()
{
	if (nullptr == m_GrappleInfo.pTransform)
		return ROPEDIR::END;

	// 1. 방향 판별할 Y
	_float fTotalHeight = m_fColliderRadius * 2.f + m_fColliderHeight;
	
	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = m_GrappleInfo.pTransform->Get_State(STATE::POSITION);

	// 2. 높이값 추출
	_float fTargetY = XMVectorGetY(vTargetPos);
	_float fMyY = XMVectorGetY(vMyPos);

	_float fFeetLevel = fMyY; // Transform이 발에 있으므로?
	_float fEyeLevel = fMyY + fTotalHeight;

	if (fTargetY > fEyeLevel)
		return ROPEDIR::U; // 위 (눈보다 위)
	else if (fTargetY < fFeetLevel)
		return ROPEDIR::D; // 아래 (발보다 아래)
	else
		return ROPEDIR::F; // 정면 (눈과 발 사이)

	return ROPEDIR::END;
}

void CCharacter::Bind_ThrowTarget(const THROW_INFO& throwInfo)
{
	m_ThrowInfo = throwInfo;
}

_bool CCharacter::Is_AttachThrowTarget()
{
	// 1. 예외 조건 처리. 
	if ((nullptr == m_ThrowInfo.pTransform) || 
		(!m_ThrowInfo.IsActive))
		return false;

	// 2. 카메라 Frustum 안에 있는가?
	_vector vTargetPos = m_ThrowInfo.pTransform->Get_State(STATE::POSITION);
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_bool IsFrustum = m_pGameInstance->IsIn_WorldSpace(vTargetPos, 5.f);

	// 3. 거리가 지정한 거리 이내인가?
	_float fDistance = XMVectorGetX(XMVector3Length(vTargetPos - vPos));

	return IsFrustum && fDistance <= m_fThrowRange;
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

_vector CCharacter::Get_Position()
{
	if (nullptr == m_pTransformCom)
		return XMVectorZero();

	return m_pTransformCom->Get_State(STATE::POSITION);
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

void CCharacter::Bind_TargetPosition(_fvector vPos)
{
	XMStoreFloat4(&m_vTargetPosition, vPos); // 타겟 지점.
}

void CCharacter::ActiveCaptureState()
{
	Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::GRABED));
}

// 무조건 Grab Animation이 나오는게 아니라 들어간 상태에서 애니메이션을 선별
void CCharacter::ClearCaptureState()
{
	// 1. 데이터 지우기
	m_PendingCaptureDesc = {};

	// 2. Collider 충돌 처리 켜기.
	//m_pColliderCom->IsActivate(true);

	// 3. 제거
	Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::GRABRELEASE));
}

_bool CCharacter::Check_AnyInput(_uint iKeyFlag, KEYSTATE eKeyState)
{
	if (nullptr == m_pInputControllerCom)
		return false;
    return m_pInputControllerCom->Check_AnyInput(iKeyFlag, eKeyState);
}

_bool CCharacter::Check_AllInput(_uint iKeyFlag, KEYSTATE eKeyState)
{
	if (nullptr == m_pInputControllerCom)
		return false;
    return m_pInputControllerCom->Check_AllInput(iKeyFlag, eKeyState);
}


void CCharacter::Clear_Animation(const _string& strAnimName, _float fTrackPosition)
{
	if (nullptr == m_pModelCom)
		return;

	m_pModelCom->Clear_Animation(strAnimName, fTrackPosition);

	
}


_bool CCharacter::Play_Animation_NonFacical(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate)
{
	ASSERT_CRASH(m_pModelCom);
	//_bool IsPlayAnimationEnd = m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, strAnimName, fTimeDelta, pTrackPosition, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate, fRootMotionRate);
	_bool IsPlayAnimationEnd = m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, strAnimName, fTimeDelta, pTrackPosition, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate, fRootMotionRate);
	m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);

	return IsPlayAnimationEnd;
}

_bool CCharacter::Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate, _bool IsFacial)
{
	ASSERT_CRASH(m_pModelCom);
	//_bool IsPlayAnimationEnd = m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, strAnimName, fTimeDelta, pTrackPosition, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate, fRootMotionRate);
	_bool IsPlayAnimationEnd = m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, m_pFacialComputeShaderCom, strAnimName, fTimeDelta, pTrackPosition, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate, fRootMotionRate, IsFacial);
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
	_vector vDirFlat = XMVectorSetW(vDir, 0.f);
    vDirFlat = XMVectorSetY(vDirFlat, 0.f);
    vDirFlat = XMVector3Normalize(vDirFlat);

    if (XMVector3Equal(vDirFlat, XMVectorZero()))
        return;

	_vector vCurrentLook = m_pTransformCom->Get_State(STATE::LOOK);
	vCurrentLook = XMVectorSetW(vCurrentLook, 0.f);
	vCurrentLook = XMVector3Normalize(vCurrentLook);

	_vector vNewLook = XMVectorLerp(vCurrentLook, vDirFlat, fTimeDelta * fSpeed);
	vNewLook = XMVectorSetW(vNewLook, 0.f); 
	vNewLook = XMVector3Normalize(vNewLook);

	_vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vRight = XMVector3Cross(vWorldUp, vNewLook);
	vRight = XMVectorSetW(vRight, 0.f);
	vRight = XMVector3Normalize(vRight);

	_vector vUp = XMVector3Cross(vNewLook, vRight);
	vUp = XMVectorSetW(vUp, 0.f);
	vUp = XMVector3Normalize(vUp);

	// 5. Transform에 값 강제 주입
	m_pTransformCom->Set_State(STATE::RIGHT, vRight);
	m_pTransformCom->Set_State(STATE::UP, vUp);
	m_pTransformCom->Set_State(STATE::LOOK, vNewLook);
}

void CCharacter::Rotate_DirectionLerp(_fvector vDir, _float fTimeDelta, _float fSpeed)
{
	_vector vDirFlat = XMVector3Normalize(vDir);
	if (XMVector3Equal(vDirFlat, XMVectorZero()))
		return;

	m_pTransformCom->LookLerp(vDirFlat, fTimeDelta, fSpeed);
}



void CCharacter::Rotate_Target(_bool IsReverse)
{
    // 1. 타겟이 없는 경우 Return
    if (nullptr == m_pTargetTransform)
        return;

    // 2. 타겟이 있으면 즉시 회전.
    _vector vTarget = m_pTargetTransform->Get_State(STATE::POSITION);
    _vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vToTarget = XMVector3Normalize(vTarget - vMyPos);


    vToTarget = XMVectorSetY(vToTarget, 0.f);

	if (IsReverse)
		vToTarget *= -1.f;
    m_pTransformCom->LookDir(vToTarget); // 이동은 바로 회전. => Idle 되면 Lerp로

    return;
}

void CCharacter::Rotate_To_Diagonal_Target(_float fAngleDegree, _bool IsRight)
{
	if (nullptr == m_pTargetTransform)
		return;

	_vector vTargetPos = m_pTargetTransform->Get_State(STATE::POSITION); // Getter 필요 없으면 Transform에서 직접 계산
	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vToTarget = XMVectorSetY(XMVector3Normalize(vTargetPos - vMyPos), 0.f);

	_float fRadians = XMConvertToRadians(fAngleDegree);

	if (!IsRight)
		fRadians *= -1.f;

	// 3. 회전 행렬 생성.
	_matrix matRot = XMMatrixRotationY(fRadians);

	// 4. 벡터를 회전 행렬로 회전 시킴.
	_vector vDiagonalDir = XMVector3TransformNormal(vToTarget, matRot);

	// 5. 캐릭터 즉시 회전 적용.
	Rotate_Direction(vDiagonalDir);
}

void CCharacter::Rotate_Target(CTransform* pTransform)
{
	if (nullptr == pTransform)
		return;

	_vector vTarget = pTransform->Get_State(STATE::POSITION);
	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vToTarget = XMVector3Normalize(vTarget - vMyPos);

	vToTarget = XMVectorSetY(vToTarget, 0.f);
	m_pTransformCom->LookDir(vToTarget); // 이동은 바로 회전. => Idle 되면 Lerp로
}

void CCharacter::Rotate_TargetPosition()
{
	_vector vTarget = XMLoadFloat4(&m_vTargetPosition);
	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vToTarget = XMVector3Normalize(vTarget - vMyPos);

	vToTarget = XMVectorSetY(vToTarget, 0.f);
	m_pTransformCom->LookDir(vToTarget); // 이동은 바로 회전. => Idle 되면 Lerp로
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
	m_pTransformCom->Save_PreviousPosition();
}

void CCharacter::Sync_Transform_ToPlayer(CTransform* pTransformCom)
{
	if (m_IsQTE)
		return;

	_matrix mat = m_pTransformCom->Get_WorldMatrix();
	pTransformCom->Set_WorldMatrix(mat);
}

void CCharacter::Sync_UtilityType_FromPlayer(UI_TAB_UTILITY eUtilityType)
{
	m_eUtilityType = eUtilityType;
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

void CCharacter::Process_MotionTrail(const _wstring& wStrObjectTag)
{
	_wstring var1, var2, var3, var4, var5;
	wstringstream wss(wStrObjectTag);

	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|');
	getline(wss, var4, L'|');
	getline(wss, var5, L'|');

	_float fDuration = stof(var2);
	_float fInterval = stof(var3);
	_float fMotionLifeTime = stof(var4);
	_uint iShaderPath = stoul(var5);

	// Color는 고정?
	Spawn_MotionTrail(fDuration, fInterval, fMotionLifeTime, m_vMotionTrailColor, iShaderPath);
}

void CCharacter::Process_PlaySound(const _wstring& wStrObjectTag)
{
	// return; 추가하면 캐릭터 사운드 안들림.
	
	_wstring var1, var2, var3, var4, var5;
	wstringstream wss(wStrObjectTag);

	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|');
	getline(wss, var4, L'|');
	getline(wss, var5, L'|');

	_wstring strSoundType = var2; // Sound Type
	_wstring strSoundTag = var3; // Sound Tag
	_float fVolume = stof(var4); // Volume 크기.

	_float fFrequency = {};
	


	if (var2 == TEXT("Voice"))
	{
		if (!var5.empty())
		{
			fFrequency = stof(var5);
			m_pGameInstance->Play_Sound(strSoundTag, ENUM_CLASS(CHANNEL::PLAYER_VOICE), fVolume, fFrequency);
		}
		else
		{
			m_pGameInstance->Play_Sound(strSoundTag, ENUM_CLASS(CHANNEL::PLAYER_VOICE), fVolume);
		}
	}
		
	else if (var2 == TEXT("Action"))
	{
		if (!var5.empty())
		{
			fFrequency = stof(var5);
			//m_pGameInstance->Play_Sound(strSoundTag, ENUM_CLASS(CHANNEL::PLAYER_ACTION), fVolume, fFrequency);
			m_pGameInstance->Play_Sound(strSoundTag, ENUM_CLASS(CHANNEL::PLAYER_ACTION), 1.f, 1.f);
		}
		else
		{
			m_pGameInstance->Play_Sound(strSoundTag, ENUM_CLASS(CHANNEL::PLAYER_ACTION), fVolume);
		}
	}
	else if (var2 == TEXT("QTE"))
	{
		if (!var5.empty())
		{
			fFrequency = stof(var5);
			//m_pGameInstance->Play_Sound(strSoundTag, ENUM_CLASS(CHANNEL::PLAYER_ACTION), fVolume, fFrequency);
			m_pGameInstance->Play_Sound(strSoundTag, ENUM_CLASS(CHANNEL::PLAYER_QTE), 1.f, 1.f);
		}
		else
		{
			m_pGameInstance->Play_Sound(strSoundTag, ENUM_CLASS(CHANNEL::PLAYER_QTE), fVolume);
		}
	}
		
}

void CCharacter::Process_SpawnSFX(const _wstring& wStrObjectTag)
{
	_wstring var1, var2;
	wstringstream wss(wStrObjectTag);

	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	

	_matrix mat = XMMatrixIdentity();
	m_pGameInstance->Spawn_PoolingObject_ForStatic(var2, mat, nullptr);
}

void CCharacter::Free()
{
    CActor::Free();
    Safe_Release(m_pGameSystem);
    Safe_Release(m_pInputControllerCom);
    Safe_Release(m_pSpringCamera);
    Safe_Release(m_pStateMachineCom);
	Safe_Release(m_pFpsStateMachineCom);
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
