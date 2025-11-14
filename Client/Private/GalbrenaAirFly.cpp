#include "ClientPch.h"
#include "GalbrenaAirFly.h"
#include "Galbrena.h"
#include "StateMachine.h"

HRESULT CGalbrenaAirFly::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pOwner);
    ASSERT_CRASH(m_pGalbrena);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

	// Parts 등록.

    return S_OK;
}


void CGalbrenaAirFly::OnEnter(void* pArg)
{
    CAirState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
	EGalbrenaAirFlyType eAirFlyType = context.m_eAirFlyType;
	m_strPrevInfo = context.m_strPrevInfo; // 복사본에서 이전 정보.

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eAirFlyType);

    // 4. Attack 상태 초기화
    State_Reset();

    // 5. 장비 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음
    m_iPartType = CGalbrena::PARTTYPE::PART_WING; 

    // 6. 장비에 Bone 붙이기. + Offset 추가.
    _string strBoneName = "WingCase";

	// 7. 파츠 상태 초기화
    m_pGalbrena->PartActivate(m_iPartType, true);
    m_pGalbrena->Set_SocketMatrixToParts(m_iPartType, strBoneName);


	// 8. 시작에 한해서 중력 끈다.
	if (eAirFlyType == EGalbrenaAirFlyType::XA_START)
	{
		m_pGalbrena->Set_Gravity(false);
		_vector vForward = m_pGalbrena->Get_LookVector();
		m_vForce = vForward * 8.f + XMVectorSet(0.f, 7.f, 0.f, 0.f); 
	}

	// 9. 물리 값 설정 
	m_fSpeed = 20.f;				// '추진 가속도' 
	m_fAccel = 3.f;					    // '상승/하강 가속도' 
	m_vGravity = { 0.f, -9.8f, 0.f };   // '활공용 중력'
	m_fDrag = 0.98f;					// '공기 저항' (속도 감쇄)
	
	// 10. SFX 설정.
	m_pGalbrena->Begin_Toggle_SFX(SFX_TOGGLE::MOTION);
}

void CGalbrenaAirFly::OnUpdate(_float fTimeDelta)
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

void CGalbrenaAirFly::OnExit()
{
    CAirState::OnExit();

	if (m_iPartType != CGalbrena::PARTTYPE::TYPE_END)
		m_pGalbrena->PartActivate(m_iPartType, false);

    m_pGalbrena->Set_Gravity(true);

	m_vGravity = {};
	m_vForce = XMVectorZero();

    m_iPartType = CGalbrena::PARTTYPE::TYPE_END;
	m_iSubPartType = CGalbrena::PARTTYPE::TYPE_END;

	// Blending 정보 초기화
	m_GpuBlendInfo = {};

	m_pGalbrena->End_SFX();
}

void CGalbrenaAirFly::Handle_Input()
{
	// 입력 방향 받기.
	m_eDir = m_pGalbrena->Calculate_Direction();

	// 키 인풋
	m_States[INPUT_U] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
	m_States[INPUT_D] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
	m_States[INPUT_L] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
	m_States[INPUT_R] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));
	m_States[INPUT_ACCEL] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT)) || m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));

	
	// 상태 변화
    m_States[ATTACK] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[DOUBLE_JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
}


void CGalbrenaAirFly::Update_FlyAnimations(_float fTimeDelta)
{
	// 0. 애니메이션 체크.
	EGalbrenaAirFlyType eAirFlyType = static_cast<EGalbrenaAirFlyType>(m_iCurrentAnimIdx);

	// 1. 기본 애니메이션 이름
	_string strBaseAnimName = m_Animations[m_iCurrentAnimIdx].strAnimName;

	// X기본(Base) 애니메이션은 XA_Loop_Stand.
	if (eAirFlyType != EGalbrenaAirFlyType::XA_START)
	{
		strBaseAnimName = "XA_Loop_Stand";
	}

#pragma region 애니메이션 보간
	// 2.. XA_START 중에는 블렌딩을 비활성화하고, 그 외에는 활성화
	if (eAirFlyType == EGalbrenaAirFlyType::XA_START)
	{
		m_GpuBlendInfo.IsBlendEnabled = false;
	}
	else
	{
		m_GpuBlendInfo.IsBlendEnabled = true;

		const _float fBlendInterpSpeed = 1.0f;
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
	m_IsAnimationEnd = CCharacterState::Play_AnimationFly(
		m_pGalbrena,
		fTimeDelta,
		1.f,
		m_GpuBlendInfo
	);

	// Parts Wing은 항상 실행됨
	if (m_iPartType != CGalbrena::PARTTYPE::TYPE_END)
	{
		// 애니메이션 속도 서로 Sync 맞추기.
		m_pGalbrena->Play_PartAnimation(
			m_iPartType,
			m_Animations[m_iCurrentAnimIdx].strAnimName,
			m_Animations[m_iCurrentAnimIdx].fSpeed * fTimeDelta, nullptr
		);
	}

	if (eAirFlyType == EGalbrenaAirFlyType::XA_START)
		return;

#pragma region 이동량 보정
	// 이동량 보정 1. 캐릭터를 직접 회전.
	_vector vLook = m_pGalbrena->Get_LookVector();
	_vector vRight = m_pGalbrena->Get_RightVector();
	_vector vTargetDir = vLook; // 기본값: 현재 방향


	// 2. 입력에 따라 목표 방향(TargetDir)을 설정
	if (m_States[INPUT_L])
		vTargetDir = XMVector3Normalize(vTargetDir - vRight * 0.5f); 
	if (m_States[INPUT_R])
		vTargetDir = XMVector3Normalize(vTargetDir + vRight * 0.5f);

	// 캐릭터를 목표 방향으로 부드럽게 회전
	if (!XMVector3Equal(vTargetDir, vLook))
		m_pGalbrena->Rotate_DirectionLerp(vTargetDir, fTimeDelta, 3.f);

	// 3. 물리 계산 (가속도 -> 속도 -> 위치)

	// 기본 가속도
	_vector vGravityAccel = XMLoadFloat3(&m_vGravity);
	
	// 입력 기반 가속도
	_vector vStrafeAccel = XMVectorZero();	// 스트레이핑 가속도
	_vector vForwardAccel = XMVectorZero();	// 전진 가속도.

	_float fCurrentSpeed = m_fSpeed;		// OnEnter에서 설정한 기본 이동 속도 (10.f)
	_float fStrafeSpeed = m_fSpeed * 10.f;
	if (m_States[INPUT_ACCEL]) // ACCEL(LShift) 누르면 3배 가속
		fStrafeSpeed *= 3.0f;

	_float fMulForward = m_States[INPUT_ACCEL] ? 6.0f : 2.0f;
	vForwardAccel = vLook * m_fSpeed * fMulForward;

	_float fDURatio = 1.0f - fabs(m_GpuBlendInfo.fBlendParamDU);  
	vForwardAccel *= fDURatio; // 정면 방향 이동 값이 W S 키에 따라서 줄어들거나 늘어납니다..
	

	if (m_States[INPUT_ACCEL] && m_States[INPUT_U])
		vStrafeAccel += XMVectorSet(0.f, 1.f, 0.f, 0.f) * fStrafeSpeed;
	else if (m_States[INPUT_D])
	{
		vStrafeAccel += XMVectorSet(0.f, -1.f, 0.f, 0.f) * fStrafeSpeed;
		if (m_States[INPUT_ACCEL])
			vStrafeAccel *= 2.f;
	}

	if (m_States[INPUT_L])
		vStrafeAccel += -m_pGalbrena->Get_RightVector_NoPitch() * fStrafeSpeed * 0.1f;
	if (m_States[INPUT_R])
		vStrafeAccel += m_pGalbrena->Get_RightVector_NoPitch() * fStrafeSpeed * 0.1f;


	_float fUpMod = max(0.f, m_GpuBlendInfo.fBlendParamDU);		// up: 0~1
	_float fDownMod = -min(0.f, m_GpuBlendInfo.fBlendParamDU);  // down: 0~1 => 양수로 변경.
	if (XMVectorGetY(vStrafeAccel) > 0.f)		// up 가속 시
		vStrafeAccel *= fUpMod;					// DU=0:0, DU=1:full (모션 완결 시 max)
	else if (XMVectorGetY(vStrafeAccel) < 0.f)  // down 가속 시
		vStrafeAccel *= fDownMod;				// DU=0:0, DU=-1:full

	// 모든 가속도를 합산
	_vector vTotalAccel = vStrafeAccel + vForwardAccel + vGravityAccel;

	m_vForce *= m_fDrag; // 속도 (Velocity) 계산
	m_vForce += vTotalAccel * fTimeDelta; // 가속도를 속도에 적용

	

	// 최대/최소 속도 제한
	_float fVerticalSpeed = XMVectorGetY(m_vForce);
	if (fVerticalSpeed < -60.f)
		m_vForce = XMVectorSetY(m_vForce, -60.f);
	if (fVerticalSpeed > 60.f)
		m_vForce = XMVectorSetY(m_vForce, 60.f);

	// 5. 최종 이동 적용 (Transform.cpp의 Go_Force는 velocity * fTimeDelta를 적용)
	m_pGalbrena->Add_Force(m_vForce, fTimeDelta);
#pragma endregion

	
}


void CGalbrenaAirFly::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
}

void CGalbrenaAirFly::Check_StateTransition(_float fTimeDelta)
{
    EGalbrenaAirFlyType eAirFlyType = static_cast<EGalbrenaAirFlyType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();

    if (IsEscapePossible)
    {
		// 우선순위
		// 1. Jump키 눌렀을 때 => 점프로 변환.
		if (m_States[JUMP])
		{

			m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
			return;
		}
		
		// 2. 땅에 닿았을때
		if (m_States[LAND])
		{

			m_pGalbrena->GetStateContextForWrite().m_eLandType = EGalbrenaLandType::LAND_ROLL;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::LAND));
			return;
		}
	
    }

	if (m_IsAnimationEnd)
	{
		if (eAirFlyType == EGalbrenaAirFlyType::XA_START)
		{
			if (!m_States[LAND])
			{
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaAirFlyType::XA_LOOP_STAND);
				return;
			}

			if (m_States[LAND])
			{
				m_pGalbrena->GetStateContextForWrite().m_eLandType = EGalbrenaLandType::LAND_ROLL;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::LAND));
				return;
			}
		}

		
	}

}

void CGalbrenaAirFly::SetUp_Animations()
{
    
	// 이동 용도는 Root모션 모두 제거.
	CState::Add_Animations(ENUM_CLASS(EGalbrenaAirFlyType::XA_LOOP_U), "XA_Loop_U", 1.f, 20.f, 1.f, true);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAirFlyType::XA_LOOP_D), "XA_Loop_D", 1.f, 20.f, 1.f, true);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaAirFlyType::XA_LOOP_L), "XA_Loop_L", 1.f, 20.f, 1.f, true);
	CState::Add_Animations(ENUM_CLASS(EGalbrenaAirFlyType::XA_LOOP_R), "XA_Loop_R", 1.f, 20.f, 1.f, true);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAirFlyType::XA_LOOP_RL_MID), "XA_Loop_RL_Mid", 1.f, 0.f, 1.f, true);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAirFlyType::XA_LOOP_STAND), "XA_Loop_Stand", 1.f, 0.f, 1.f, true);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAirFlyType::XA_SHAKE_LOOP), "XA_Shake_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAirFlyType::XA_START), "XA_Start", 1.f, 30.f, 3.f);
	
}

void CGalbrenaAirFly::State_Reset()
{
    for (_uint i = 0; i < AIRFLYSTATE::END; ++i)
        m_States[i] = false;
}

CGalbrenaAirFly* CGalbrenaAirFly::Create(class CGameObject* pOwner)
{
    CGalbrenaAirFly* pInstance = new CGalbrenaAirFly();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaAirFly");
    }

    return pInstance;
}

void CGalbrenaAirFly::Free()
{
    CAirState::Free();
}
