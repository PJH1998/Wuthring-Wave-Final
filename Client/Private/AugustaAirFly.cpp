#include "ClientPch.h"
#include "AugustaAirFly.h"
#include "Augusta.h"
#include "StateMachine.h"

HRESULT CAugustaAirFly::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

	// Parts 등록.

    return S_OK;
}


void CAugustaAirFly::OnEnter(void* pArg)
{
    CAirState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
	EAugustaAirFlyType eAirFlyType = context.m_eAirFlyType;
	m_strPrevInfo = context.m_strPrevInfo; // 복사본에서 이전 정보.

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eAirFlyType);

    // 4. Attack 상태 초기화
    State_Reset();

    // 5. 장비 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음
    m_iPartType = CAugusta::PARTTYPE::PART_WING; 

    // 6. 장비에 Bone 붙이기. + Offset 추가.
    _string strBoneName = "WingCase";

	// 7. 파츠 상태 초기화
    m_pAugusta->PartActivate(m_iPartType, true);
    m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);


	// 8. 시작에 한해서 중력 끈다.
	if (eAirFlyType == EAugustaAirFlyType::XA_START)
	{
		m_pAugusta->Set_Gravity(false);
		_vector vForward = m_pAugusta->Get_LookVector();
		m_vForce = vForward * 8.f + XMVectorSet(0.f, 7.f, 0.f, 0.f);  // forward 8m/s, up 7m/s (테스트로 조정) => 초기 가속.
	}

	// 9. 부여 값
	// 9. 물리 값 설정 (조정이 필요합니다)
	m_fSpeed = 5.f * 2.f;     // '추진 가속도' (조정 필요)
	m_fAccel = 3.f;     // '상승/하강 가속도' (조정 필요)
	m_vGravity = { 0.f, -4.9f, 0.f }; // '활공용 중력' (조정 필요)
	m_fLift = 4.7f;     // '양력' (중력보다 약간 작게 설정)
	m_fDrag = 0.98f;    // '공기 저항' (속도 감쇄)
 
	
}

void CAugustaAirFly::OnUpdate(_float fTimeDelta)
{
    CAirState::OnUpdate(fTimeDelta);

    // 0. 입력 확인
    Handle_Input();

    // 1. Fly 업데이트
    Update_FlyAnimations(fTimeDelta);

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    // 3. 전환 조건 체크
    Check_StateTransition(fTimeDelta);

    State_Reset();

}

void CAugustaAirFly::OnExit()
{
    CAirState::OnExit();

	if (m_iPartType != CAugusta::PARTTYPE::TYPE_END)
		m_pAugusta->PartActivate(m_iPartType, false);

    m_pAugusta->Set_Gravity(true);

	m_vGravity = {};
	m_vForce = XMVectorZero();

    m_iPartType = CAugusta::PARTTYPE::TYPE_END;
	m_iSubPartType = CAugusta::PARTTYPE::TYPE_END;

	// Blending 정보 초기화
	m_GpuBlendInfo = {};
}

void CAugustaAirFly::Handle_Input()
{
	// 입력 방향 받기.
	m_eDir = m_pAugusta->Calculate_Direction();

	// 키 인풋
	m_States[INPUT_U] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
	m_States[INPUT_D] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
	m_States[INPUT_L] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
	m_States[INPUT_R] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));
	m_States[INPUT_ACCEL] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT)) || m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));

	
	// 상태 변화
    m_States[ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[DOUBLE_JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
}


void CAugustaAirFly::Update_FlyAnimations(_float fTimeDelta)
{
	// 0. 애니메이션 체크.
	EAugustaAirFlyType eAirFlyType = static_cast<EAugustaAirFlyType>(m_iCurrentAnimIdx);

	// 1. 기본 애니메이션 이름
	_string strBaseAnimName = m_Animations[m_iCurrentAnimIdx].strAnimName;

	// X기본(Base) 애니메이션은 XA_Loop_Stand.
	if (eAirFlyType != EAugustaAirFlyType::XA_START)
	{
		strBaseAnimName = "XA_Loop_Stand";
	}

#pragma region 애니메이션 보간
	// 2.. XA_START 중에는 블렌딩을 비활성화하고, 그 외에는 활성화
	if (eAirFlyType == EAugustaAirFlyType::XA_START)
	{
		m_GpuBlendInfo.IsBlendEnabled = false;
	}
	else
	{
		m_GpuBlendInfo.IsBlendEnabled = true;

		const _float fBlendInterpSpeed = 2.0f;
		_float fInterpStep = fTimeDelta * fBlendInterpSpeed; // 이번 프레임에 보간할 스텝

		// L / R (좌 / 우)
		if (m_States[INPUT_L])
			m_GpuBlendInfo.fBlendParamLR -= fInterpStep;
		else if (m_States[INPUT_R])
			m_GpuBlendInfo.fBlendParamLR += fInterpStep;
		else
		{
			if (m_GpuBlendInfo.fBlendParamLR > 0.0f)
			{
				m_GpuBlendInfo.fBlendParamLR -= fInterpStep;
				// 0을 지나쳐 음수가 되었으면 0으로 스냅
				if (m_GpuBlendInfo.fBlendParamLR < 0.0f)
					m_GpuBlendInfo.fBlendParamLR = 0.0f;
			}
			else if (m_GpuBlendInfo.fBlendParamLR < 0.0f)
			{
				m_GpuBlendInfo.fBlendParamLR += fInterpStep;
				// 0을 지나쳐 양수가 되었으면 0으로 스냅
				if (m_GpuBlendInfo.fBlendParamLR > 0.0f)
					m_GpuBlendInfo.fBlendParamLR = 0.0f;
			}
		}

		// D / U (하 / 상)
		if (m_States[INPUT_D]) // S (하강)
			m_GpuBlendInfo.fBlendParamDU -= fInterpStep;
		else if (m_States[INPUT_ACCEL] && m_States[INPUT_U]) // W + ACCEL (상승)
			m_GpuBlendInfo.fBlendParamDU += fInterpStep;
		else // 그 외 (W만 누르거나, 아무것도 안 누름)
		{
			// 0.0으로 복귀 (진동 방지 로직)
			if (m_GpuBlendInfo.fBlendParamDU > 0.0f)
			{
				m_GpuBlendInfo.fBlendParamDU -= fInterpStep;
				if (m_GpuBlendInfo.fBlendParamDU < 0.0f)
					m_GpuBlendInfo.fBlendParamDU = 0.0f;
			}
			else if (m_GpuBlendInfo.fBlendParamDU < 0.0f)
			{
				m_GpuBlendInfo.fBlendParamDU += fInterpStep;
				if (m_GpuBlendInfo.fBlendParamDU > 0.0f)
					m_GpuBlendInfo.fBlendParamDU = 0.0f;
			}
		}
		// 파라미터 값 제한
		m_GpuBlendInfo.fBlendParamLR = Clamp(m_GpuBlendInfo.fBlendParamLR, -1.f, 1.f);
		m_GpuBlendInfo.fBlendParamDU = Clamp(m_GpuBlendInfo.fBlendParamDU, -1.f, 1.f);

		// 블렌딩 파라미터 바인딩.
		m_GpuBlendInfo.strClipxL = "XA_Loop_L";
		m_GpuBlendInfo.strClipMidLR = "XA_Loop_Stand";
		m_GpuBlendInfo.strClipxR = "XA_Loop_R";
		m_GpuBlendInfo.strWeightClipLR = "XA_Loop_Stand";

		m_GpuBlendInfo.strClipxD = "XA_Loop_D";
		m_GpuBlendInfo.strClipMidDU = "XA_Loop_Stand";
		m_GpuBlendInfo.strClipxU = "XA_Loop_U";
		m_GpuBlendInfo.strWeightClipDU = "XA_Loop_Stand";
	}
#pragma endregion

	// 애니메이션 실행
	m_IsAnimationEnd = CCharacterState::Play_Animation(
		m_pAugusta,
		fTimeDelta,
		1.f,
		m_GpuBlendInfo
	);

#pragma region 이동량 보정
	// 이동량 보정 1. 캐릭터를 직접 회전.
	_vector vLook = m_pAugusta->Get_LookVector();
	_vector vRight = m_pAugusta->Get_RightVector();
	_vector vTargetDir = vLook; // 기본값: 현재 방향


	// 2. 입력에 따라 목표 방향(TargetDir)을 설정
	if (m_States[INPUT_L])
		vTargetDir = XMVector3Normalize(vTargetDir - vRight * 0.5f); // 회전 민감도 (0.5f)
	if (m_States[INPUT_R])
		vTargetDir = XMVector3Normalize(vTargetDir + vRight * 0.5f);

	// 캐릭터를 목표 방향으로 부드럽게 회전 (Character.h/cpp에 있는 함수 활용)
	if (!XMVector3Equal(vTargetDir, vLook))
	{
		m_pAugusta->Rotate_DirectionLerp(vTargetDir, fTimeDelta, 4.f);
	}

	// 3. 물리 계산 (가속도 -> 속도 -> 위치)

	// --- 3-1. 기본 가속도 (항상 적용) ---
	_vector vGravityAccel = XMLoadFloat3(&m_vGravity);
	//_vector vLiftAccel = XMVectorSet(0.f, m_fLift, 0.f, 0.f);

	// --- 3-2. 입력 기반 가속도 ---
	_vector vMoveAccel = XMVectorZero();   // W(Shift)/S/A/D에 의한 스트레이핑 가속도
	_vector vThrustAccel = XMVectorZero(); // W에 의한 전진 추진 가속도

	_float fCurrentSpeed = m_fSpeed; // OnEnter에서 설정한 기본 이동 속도 (10.f)
	_float fStrafeSpeed = m_fSpeed * 0.75f; // 좌/우/상/하 이동 속도 (기본 속도의 75%)

	// (요구사항 3) ACCEL(LShift) 누르면 3배 가속
	if (m_States[INPUT_ACCEL])
	{
		fCurrentSpeed *= 3.0f;
		fStrafeSpeed *= 3.0f;
	}

	// (요구사항 2) W = 기본 전진 (카메라 Look 벡터 방향)
	if (m_States[INPUT_U])
	{
		_vector vCameraLook = m_pAugusta->Get_CameraLookVector(); // Pitch 포함
		vThrustAccel = vCameraLook * fCurrentSpeed * 2.f;
	}

	// (요구사항 3) 스트레이핑(Strafe) 이동
	
	// W+Shift(상승) / S(하강)
	if (m_States[INPUT_ACCEL] && m_States[INPUT_U]) // W + SHIFT = Up (요청 사항)
	{
		vMoveAccel += XMVectorSet(0.f, 1.f, 0.f, 0.f) * fStrafeSpeed * 3.f;
	}
	else if (m_States[INPUT_D]) // S = Down (요청 사항)
	{
		vMoveAccel += XMVectorSet(0.f, -1.f, 0.f, 0.f) * fStrafeSpeed;
	}

	// A(좌) / D(우)
	if (m_States[INPUT_L]) // A = Left (요청 사항)
	{
		vMoveAccel += -m_pAugusta->Get_RightVector_NoPitch() * fStrafeSpeed;
	}
	if (m_States[INPUT_R]) // D = Right (요청 사항)
	{
		vMoveAccel += m_pAugusta->Get_RightVector_NoPitch() * fStrafeSpeed;
	}

	// 모든 가속도를 합산
	//_vector vTotalAccel = vThrustAccel + vMoveAccel + vGravityAccel + vLiftAccel;
	_vector vTotalAccel = vThrustAccel + vMoveAccel + vGravityAccel;

	// 속도 (Velocity) 계산
	m_vForce *= m_fDrag; // (요구사항 1) 항력(Drag)으로 매 프레임 감속
	// 가속도를 속도에 적용
	m_vForce += vTotalAccel * fTimeDelta;

	

	// 최대/최소 속도 제한
	_float fVerticalSpeed = XMVectorGetY(m_vForce);
	if (fVerticalSpeed < -15.f)
		m_vForce = XMVectorSetY(m_vForce, -15.f);
	if (fVerticalSpeed > 8.f)
		m_vForce = XMVectorSetY(m_vForce, 8.f);

	// 5. 최종 이동 적용 (Transform.cpp의 Go_Force는 velocity * fTimeDelta를 적용)
	m_pAugusta->Add_Force(m_vForce, fTimeDelta);
#pragma endregion

	// Parts Wing은 항상 실행됨
	if (m_iPartType != CAugusta::PARTTYPE::TYPE_END)
	{
		// 애니메이션 속도 서로 Sync 맞추기.
		m_pAugusta->Play_PartAnimation(
			m_iPartType,
			m_Animations[m_iCurrentAnimIdx].strAnimName,
			m_Animations[m_iCurrentAnimIdx].fSpeed * fTimeDelta, nullptr
		);
	}
}


void CAugustaAirFly::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pAugusta->Is_Land();
}

void CAugustaAirFly::Check_StateTransition(_float fTimeDelta)
{
    EAugustaAirFlyType eAirFlyType = static_cast<EAugustaAirFlyType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();

    if (IsEscapePossible)
    {
		// 우선순위
		// 1. Jump키 눌렀을 때 => 점프로 변환.
		if (m_States[JUMP])
		{

			m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
			return;
		}
		
		// 2. 땅에 닿았을때
		if (m_States[LAND])
		{

			m_pAugusta->GetStateContextForWrite().m_eLandType = EAugustaLandType::LAND_ROLL;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
			return;
		}
	
    }

	if (m_IsAnimationEnd)
	{
		if (eAirFlyType == EAugustaAirFlyType::XA_START)
		{
			if (!m_States[LAND])
			{
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_STAND);
				return;
			}

			if (m_States[LAND])
			{
				m_pAugusta->GetStateContextForWrite().m_eLandType = EAugustaLandType::LAND_ROLL;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
				return;
			}
		}

		
	}

}

void CAugustaAirFly::SetUp_Animations()
{
    
	// 이동 용도는 Root모션 모두 제거.
	CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_U), "XA_Loop_U", 1.f, 20.f, 1.f, true);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_D), "XA_Loop_D", 1.f, 20.f, 1.f, true);
	CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_L), "XA_Loop_L", 1.f, 20.f, 1.f, true);
	CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_R), "XA_Loop_R", 1.f, 20.f, 1.f, true);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_RL_MID), "XA_Loop_RL_Mid", 1.f, 0.f, 1.f, true);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_STAND), "XA_Loop_Stand", 1.f, 0.f, 1.f, true);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_SHAKE_LOOP), "XA_Shake_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_START), "XA_Start", 1.f, 30.f, 1.f);
	
}

void CAugustaAirFly::State_Reset()
{
    for (_uint i = 0; i < AIRFLYSTATE::END; ++i)
        m_States[i] = false;
}

CAugustaAirFly* CAugustaAirFly::Create(class CGameObject* pOwner)
{
    CAugustaAirFly* pInstance = new CAugustaAirFly();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaAirFly");
    }

    return pInstance;
}

void CAugustaAirFly::Free()
{
    CAirState::Free();
}
