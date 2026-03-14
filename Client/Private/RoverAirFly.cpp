#include "ClientPch.h"
#include "RoverAirFly.h"
#include "Rover.h"
#include "StateMachine.h"

HRESULT CRoverAirFly::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CAirState::Initialize(pCharacter)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pCharacter);
    ASSERT_CRASH(m_pRover);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

	m_fSoundTimer = {};
	m_fMaxTime = 8.f; // 초기화 시간.

	// 0 ~ 3 사이 랜덤실행.
	m_SoundTags.reserve(4);
	m_SoundTags.emplace_back(TEXT("role_wind_fly_01 (SFX)"));
	m_SoundTags.emplace_back(TEXT("role_wind_fly_02 (SFX)"));
	m_SoundTags.emplace_back(TEXT("role_wind_fly_03 (SFX)"));
	m_SoundTags.emplace_back(TEXT("role_wind_fly_04 (SFX)"));

    return S_OK;
}


void CRoverAirFly::OnEnter(void* pArg)
{
    CAirState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
	ERoverAirFlyType eAirFlyType = context.m_eAirFlyType;
	m_strPrevInfo = context.m_strPrevInfo; // 복사본에서 이전 정보.

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eAirFlyType);

    // 4. Attack 상태 초기화
    State_Reset();

    // 5. 장비 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음
    m_iPartType = CRover::PARTTYPE::PART_WING; 

    // 6. 장비에 Bone 붙이기. + Offset 추가.
    _string strBoneName = "WingCase";

	// 7. 파츠 상태 초기화
    m_pRover->PartActivate(m_iPartType, true);
    m_pRover->Set_SocketMatrixToParts(m_iPartType, strBoneName);


	// 8. 시작에 한해서 중력 끈다.
	if (eAirFlyType == ERoverAirFlyType::XA_START)
	{
		m_pRover->Set_Gravity(false);
		_vector vForward = m_pRover->Get_LookVector();
		m_vForce = vForward * 8.f + XMVectorSet(0.f, 7.f, 0.f, 0.f);
	}

	// 9. 물리 값 설정 
	m_fSpeed = 20.f;				// '추진 가속도' 
	m_fAccel = 3.f;					    // '상승/하강 가속도' 
	m_vGravity = { 0.f, -9.8f, 0.f };   // '활공용 중력'
	m_fDrag = 0.98f;					// '공기 저항' (속도 감쇄)

	// 10. SFX Motion 시작.
	m_pRover->Begin_Toggle_SFX(SFX_TOGGLE::MOTION);

	m_fSoundTimer = 0.f;


	_uint iRandIdx = static_cast<_uint>(m_pRover->Rand(0.f, 3.1f));
	m_pRover->Stop_Sound(CHANNEL::PLAYER_ACTION);
	if (iRandIdx < m_SoundTags.size())
		m_pRover->Play_Sound(m_SoundTags[iRandIdx], CHANNEL::PLAYER_ACTION, 0.3f, 1.f);
}

void CRoverAirFly::OnUpdate(_float fTimeDelta)
{
    CAirState::OnUpdate(fTimeDelta);

	Process_Timer(fTimeDelta);
    Handle_Input();
    Update_FlyAnimations(fTimeDelta);
	Update_FlyMovement(fTimeDelta);
    Check_Physics(fTimeDelta);
    Check_StateTransition(fTimeDelta);
    State_Reset();

}

void CRoverAirFly::OnExit()
{
    CAirState::OnExit();

	m_pRover->Stop_Sound(CHANNEL::PLAYER_ACTION);

	if (m_iPartType != CRover::PARTTYPE::TYPE_END)
		m_pRover->PartActivate(m_iPartType, false);

    m_pRover->Set_Gravity(true);

	m_vGravity = {};
	m_vForce = XMVectorZero();

    m_iPartType = CRover::PARTTYPE::TYPE_END;
	m_iSubPartType = CRover::PARTTYPE::TYPE_END;

	// Blending 정보 초기화
	m_GpuBlendInfo = {};

	m_pRover->End_SFX();
}

_float CRoverAirFly::Approach(_float fCurrent, _float fTarget, _float fTimeDelta)
{
	if (fTimeDelta <= 0.f)
		return fCurrent;

	if (fCurrent < fTarget)
		return min(fCurrent + fTimeDelta, fTarget);

	if (fCurrent > fTarget)
		return max(fCurrent - fTimeDelta, fTarget);

	return fTarget;
}

void CRoverAirFly::Update_AxisBlend(_float& fValue, _bool isNegativeInput, _bool isPositiveInput, _float fStep)
{
	if (isNegativeInput)
		fValue -= fStep;
	else if (isPositiveInput)
		fValue += fStep;
	else
		fValue = Approach(fValue, 0.f, fStep);
}

void CRoverAirFly::SetUp_FlyBlendClips()
{
	m_GpuBlendInfo.strClipxL = "XA_Loop_L";
	m_GpuBlendInfo.strClipMidLR = "XA_Loop_Stand";
	m_GpuBlendInfo.strClipxR = "XA_Loop_R";
	m_GpuBlendInfo.strWeightClipLR = "XA_Loop_Stand";

	m_GpuBlendInfo.strClipxD = "XA_Loop_D";
	m_GpuBlendInfo.strClipMidDU = "XA_Loop_Stand";
	m_GpuBlendInfo.strClipxU = "XA_Loop_U";
	m_GpuBlendInfo.strWeightClipDU = "XA_Loop_Stand";
}

void CRoverAirFly::Process_Timer(_float fTimeDelta)
{
	if (m_fSoundTimer < m_fMaxTime)
		m_fSoundTimer += fTimeDelta;
	else if (m_fSoundTimer >= m_fMaxTime)
	{
		m_fSoundTimer = 0.f;
		_uint iRandIdx = static_cast<_uint>(m_pRover->Rand(0.f, 3.1f));
		m_pRover->Stop_Sound(CHANNEL::PLAYER_ACTION);
		if (iRandIdx < m_SoundTags.size())
			m_pRover->Play_Sound(m_SoundTags[iRandIdx], CHANNEL::PLAYER_ACTION, 0.3f, 1.f);
	}
}

void CRoverAirFly::Handle_Input()
{
	// 입력 방향 받기.
	m_eDir = m_pRover->Calculate_Direction();

	// 키 인풋
	m_States[INPUT_U] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
	m_States[INPUT_D] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
	m_States[INPUT_L] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
	m_States[INPUT_R] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));
	m_States[INPUT_ACCEL] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT)) || m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));
	m_States[INPUT_ACCEL_KEYDOWN] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT), KEYSTATE::DOWN);
	
	// 상태 변화
    m_States[ATTACK] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[DOUBLE_JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));

	if (m_States[INPUT_ACCEL_KEYDOWN])
		m_pRover->Spawn_WingEffect(TEXT("Common_Fly_Start3"));
}

void CRoverAirFly::Update_FlyMovement(_float fTimeDelta)
{
	ERoverAirFlyType eAirFlyType = static_cast<ERoverAirFlyType>(m_iCurrentAnimIdx);
	if (eAirFlyType == ERoverAirFlyType::XA_START)
		return;

	_vector vLook = m_pRover->Get_LookVector();
	_vector vRight = m_pRover->Get_RightVector();
	_vector vTargetDir = vLook;


	if (m_States[INPUT_L])
		vTargetDir = XMVector3Normalize(vTargetDir - vRight * 0.5f);
	if (m_States[INPUT_R])
		vTargetDir = XMVector3Normalize(vTargetDir + vRight * 0.5f);

	if (!XMVector3Equal(vTargetDir, vLook))
		m_pRover->Rotate_DirectionLerp(vTargetDir, fTimeDelta, 3.f);

	_vector vGravityAccel = XMLoadFloat3(&m_vGravity);

	_vector vStrafeAccel = XMVectorZero();
	_vector vForwardAccel = XMVectorZero();

	_float fCurrentSpeed = m_fSpeed;
	_float fStrafeSpeed = m_fSpeed * 10.f;
	if (m_States[INPUT_ACCEL])
		fStrafeSpeed *= 3.0f;

	_float fMulForward = m_States[INPUT_ACCEL] ? 6.0f : 2.0f;
	vForwardAccel = vLook * m_fSpeed * fMulForward;

	_float fDURatio = 1.0f - fabs(m_GpuBlendInfo.fBlendParamDU);
	vForwardAccel *= fDURatio;


	if (m_States[INPUT_ACCEL] && m_States[INPUT_U])
		vStrafeAccel += XMVectorSet(0.f, 1.f, 0.f, 0.f) * fStrafeSpeed;
	else if (m_States[INPUT_D])
	{
		vStrafeAccel += XMVectorSet(0.f, -1.f, 0.f, 0.f) * fStrafeSpeed;
		if (m_States[INPUT_ACCEL])
			vStrafeAccel *= 2.f;
	}

	if (m_States[INPUT_L])
		vStrafeAccel += -m_pRover->Get_RightVector_NoPitch() * fStrafeSpeed * 0.1f;
	if (m_States[INPUT_R])
		vStrafeAccel += m_pRover->Get_RightVector_NoPitch() * fStrafeSpeed * 0.1f;


	_float fUpMod = max(0.f, m_GpuBlendInfo.fBlendParamDU);
	_float fDownMod = -min(0.f, m_GpuBlendInfo.fBlendParamDU);
	if (XMVectorGetY(vStrafeAccel) > 0.f)
		vStrafeAccel *= fUpMod;
	else if (XMVectorGetY(vStrafeAccel) < 0.f)
		vStrafeAccel *= fDownMod;

	_vector vTotalAccel = vStrafeAccel + vForwardAccel + vGravityAccel;

	m_vForce *= m_fDrag;
	m_vForce += vTotalAccel * fTimeDelta;

	_float fVerticalSpeed = XMVectorGetY(m_vForce);
	if (fVerticalSpeed < -60.f)
		m_vForce = XMVectorSetY(m_vForce, -60.f);
	if (fVerticalSpeed > 60.f)
		m_vForce = XMVectorSetY(m_vForce, 60.f);

	m_pRover->Add_Force(m_vForce, fTimeDelta);
}


void CRoverAirFly::Update_FlyAnimations(_float fTimeDelta)
{
	ERoverAirFlyType eAirFlyType = static_cast<ERoverAirFlyType>(m_iCurrentAnimIdx);
	_string strBaseAnimName = m_Animations[m_iCurrentAnimIdx].strAnimName;
	if (eAirFlyType != ERoverAirFlyType::XA_START)
	{
		strBaseAnimName = "XA_Loop_Stand";
	}

#pragma region 애니메이션 보간
	if (eAirFlyType == ERoverAirFlyType::XA_START)
	{
		m_GpuBlendInfo.IsBlendEnabled = false;
	}
	else
	{
		m_GpuBlendInfo.IsBlendEnabled = true;
		const _float fBlendInterpSpeed = 1.0f;
		const _float fInterpStep = fTimeDelta * fBlendInterpSpeed;

		Update_AxisBlend(
			m_GpuBlendInfo.fBlendParamLR,
			m_States[INPUT_L],
			m_States[INPUT_R],
			fInterpStep
		);

		Update_AxisBlend(
			m_GpuBlendInfo.fBlendParamDU,
			m_States[INPUT_D],
			m_States[INPUT_ACCEL] && m_States[INPUT_U],
			fInterpStep
		);

		m_GpuBlendInfo.fBlendParamLR = Clamp(m_GpuBlendInfo.fBlendParamLR, -1.f, 1.f);
		m_GpuBlendInfo.fBlendParamDU = Clamp(m_GpuBlendInfo.fBlendParamDU, -1.f, 1.f);
		SetUp_FlyBlendClips();
	}
#pragma endregion

	m_IsAnimationEnd = CCharacterState::Play_AnimationFly(
		m_pRover,
		fTimeDelta,
		1.f,
		m_GpuBlendInfo
	);

	if (m_iPartType != CRover::PARTTYPE::TYPE_END)
	{
		m_pRover->Play_PartAnimation(
			m_iPartType,
			m_Animations[m_iCurrentAnimIdx].strAnimName,
			m_Animations[m_iCurrentAnimIdx].fSpeed * fTimeDelta, nullptr
		);
	}
}


void CRoverAirFly::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
}

void CRoverAirFly::Check_StateTransition(_float fTimeDelta)
{
    ERoverAirFlyType eAirFlyType = static_cast<ERoverAirFlyType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();

    if (IsEscapePossible)
    {
		// 우선순위
		// 1. Jump키 눌렀을 때 => 점프로 변환.
		if (m_States[JUMP])
		{

			m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_SECOND_F;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP));
			return;
		}
		
		// 2. 땅에 닿았을때
		if (m_States[LAND])
		{

			m_pRover->GetStateContextForWrite().m_eLandType = ERoverLandType::LAND_ROLL;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::LAND));
			return;
		}
	
    }

	if (m_IsAnimationEnd)
	{
		if (eAirFlyType == ERoverAirFlyType::XA_START)
		{
			if (!m_States[LAND])
			{
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverAirFlyType::XA_LOOP_STAND);
				return;
			}

			if (m_States[LAND])
			{
				m_pRover->GetStateContextForWrite().m_eLandType = ERoverLandType::LAND_ROLL;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::LAND));
				return;
			}
		}

		
	}

}

void CRoverAirFly::SetUp_Animations()
{
    
	// 이동 용도는 Root모션 모두 제거.
	CState::Add_Animations(ENUM_CLASS(ERoverAirFlyType::XA_LOOP_U), "XA_Loop_U", 1.f, 20.f, 1.f, true);
    CState::Add_Animations(ENUM_CLASS(ERoverAirFlyType::XA_LOOP_D), "XA_Loop_D", 1.f, 20.f, 1.f, true);
	CState::Add_Animations(ENUM_CLASS(ERoverAirFlyType::XA_LOOP_L), "XA_Loop_L", 1.f, 20.f, 1.f, true);
	CState::Add_Animations(ENUM_CLASS(ERoverAirFlyType::XA_LOOP_R), "XA_Loop_R", 1.f, 20.f, 1.f, true);
    CState::Add_Animations(ENUM_CLASS(ERoverAirFlyType::XA_LOOP_RL_MID), "XA_Loop_RL_Mid", 1.f, 0.f, 1.f, true);
    CState::Add_Animations(ENUM_CLASS(ERoverAirFlyType::XA_LOOP_STAND), "XA_Loop_Stand", 1.f, 0.f, 1.f, true);
    CState::Add_Animations(ENUM_CLASS(ERoverAirFlyType::XA_SHAKE_LOOP), "XA_Shake_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverAirFlyType::XA_START), "XA_Start", 1.f, 30.f, 3.f);
	
}

void CRoverAirFly::State_Reset()
{
    for (_uint i = 0; i < AIRFLYSTATE::END; ++i)
        m_States[i] = false;
}

CRoverAirFly* CRoverAirFly::Create(CCharacter* pOwner)
{
    CRoverAirFly* pInstance = new CRoverAirFly();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverAirFly");
    }

    return pInstance;
}

void CRoverAirFly::Free()
{
    CAirState::Free();
}
