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


void CAugustaAirFly::OnEnter()
{
    CAirState::OnEnter();

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
		m_pAugusta->Add_Force(vForward * 5.f, 0.f);  // 즉시 힘 부여 (fTimeDelta = 1.f)
	}

	// 9. Speed 부여값
	m_fSpeed = 1.f;
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
	{
		m_pAugusta->PartActivate(m_iPartType, false);
	}

    m_pAugusta->Set_Gravity(true);
    m_fSpeed = 0.f;
    
    m_iPartType = CAugusta::PARTTYPE::TYPE_END;
	m_iSubPartType = CAugusta::PARTTYPE::TYPE_END;
}

void CAugustaAirFly::Handle_Input()
{
	// 입력 방향 받기.
	m_eDir = m_pAugusta->Calculate_Direction();

	// 키 인풋
	m_States[FLY_U] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
	m_States[FLY_D] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
	m_States[FLY_L] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
	m_States[FLY_R] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));

	// 상태 변화
    m_States[ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[DOUBLE_JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
    
	// 이전 E스킬이 그리폰이였다면 까지 조건이 있어야함.
}

void CAugustaAirFly::Update_FlyAnimations(_float fTimeDelta)
{
	// 0. 애니메이션 체크.
	EAugustaAirFlyType eAirFlyType = static_cast<EAugustaAirFlyType>(m_iCurrentAnimIdx);

    // 1. 애니메이션 실행. (기본 실행.
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // 2. 애니메이션 마다 움직임 다르게.
	_vector vTargetDir = XMVectorZero();

	switch (eAirFlyType)
	{
	case EAugustaAirFlyType::XA_LOOP_U:  // 위/앞 방향 (W 키)
		// 앞으로 이동 + 약간 상승
		//vTargetDir = m_pAugusta->Get_LookVector();  // forward
		vTargetDir = m_pAugusta->Calculate_Move_Direction(ACTORDIR::U);  // 정면 벡터
		vTargetDir += XMVectorSet(0.f, 0.5f, 0.f, 0.f);
		break;

	case EAugustaAirFlyType::XA_LOOP_D:  // 아래/뒤 방향 (S 키)
		// 뒤로 이동 + 약간 하강
		//vTargetDir = m_pAugusta->Get_LookVector() * -1.f;  

		vTargetDir = m_pAugusta->Calculate_Move_Direction(ACTORDIR::D);  // 측면 벡터
		vTargetDir -= XMVectorSet(0.f, 0.5f, 0.f, 0.f);
		break;

	case EAugustaAirFlyType::XA_LOOP_L:  // 왼쪽 (A 키)
		vTargetDir = m_pAugusta->Calculate_Move_Direction(ACTORDIR::L);  // 측면 벡터
		break;

	case EAugustaAirFlyType::XA_LOOP_R:  // 오른쪽 (D 키)
		vTargetDir = m_pAugusta->Calculate_Move_Direction(ACTORDIR::R);  // 측면 벡터
		break;
	case EAugustaAirFlyType::XA_LOOP_STAND:  // 정지가 아니라 현재 방향을 유지한채로 이동.
		vTargetDir = m_pAugusta->Get_LookVector();
		break;

	case EAugustaAirFlyType::XA_SHAKE_LOOP:  // 흔들림
		vTargetDir = vTargetDir = m_pAugusta->Get_LookVector();
		break;

	case EAugustaAirFlyType::XA_START:  // 초기 발진
		// 강한 앞으로 추진 (이미 OnEnter에서 추가)
		vTargetDir = m_pAugusta->Get_LookVector();
		m_pAugusta->Move_Direction(vTargetDir, fTimeDelta, 0.3f); 
		break;
	default:
		break;
	}
    
	// 공통: 부드러운 회전 (Rotate_DirectionLerp: LookLerp 호출)
	if (!XMVector3Equal(vTargetDir, XMVectorZero()))
	{
		// 현재 이동.
		//_vector vMoveDirection = m_pAugusta->Get_LookVector();
		m_pAugusta->Rotate_DirectionLerp(vTargetDir, fTimeDelta, 2.f);  // 턴 속도 8.f (조절 가능)
		m_pAugusta->Move_Direction(vTargetDir, fTimeDelta, m_fSpeed);  // 이동
	}
	else
	{
		// 입력 없음: 속도 감쇠 (stall 효과)
		m_fSpeed = max(0.f, m_fSpeed - 0.2f * fTimeDelta);
		m_pAugusta->Move_Direction(m_pAugusta->Get_LookVector(), fTimeDelta, m_fSpeed);  // 잔여 직진
	}

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
		// 우선순위 별.
		// Jump키 눌렀을 때 => 점프로 변환.
		if (m_States[JUMP])
		{
			m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
			return;
		}
		
		// 땅에 닿았을때
		if (m_States[LAND])
		{
			m_pAugusta->GetStateContextForWrite().m_eLandType = EAugustaLandType::LAND_ROLL;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
			return;
		}

		if (m_States[FLY_U])
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_U);
			return;
		}
		else if (m_States[FLY_D])
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_D);
			return;
		}
		else if (m_States[FLY_L])
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_L);
			return;
		}
		else if (m_States[FLY_R])
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_R);
			return;
		}
		else
		{
			// 입력이 없다면?
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_STAND);
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
				// 이동 키를 누르면? => Land Roll
				if (m_States[MOVE])
				{
					m_pAugusta->GetStateContextForWrite().m_eLandType = EAugustaLandType::LAND_ROLL;
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
					return;
				}
				else
				{
					m_pAugusta->GetStateContextForWrite().m_eLandType = EAugustaLandType::LAND_HEAVY;
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
					return;
				}
			}
		}
	}

}

void CAugustaAirFly::SetUp_Animations()
{
    
	CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_U), "XA_Loop_U", 1.f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_D), "XA_Loop_D", 1.f, 0.f, 1.f);
	CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_L), "XA_Loop_L", 1.f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_R), "XA_Loop_R", 1.f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_RL_MID), "XA_Loop_RL_Mid", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirFlyType::XA_LOOP_STAND), "XA_Loop_Stand", 1.f, 0.f);
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
