#include "ClientPch.h"
#include "YunoAirAttack.h"
#include "Yuno.h"
#include "StateMachine.h"

HRESULT CYunoAirAttack::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CAirState::Initialize(pCharacter)))
        return E_FAIL;

    m_pYuno = dynamic_cast<CYuno*>(pCharacter);
    ASSERT_CRASH(m_pYuno);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

	// 매핑.

    return S_OK;
}


void CYunoAirAttack::OnEnter(void* pArg)
{
    CAirState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pYuno->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EYunoAirAttackType eAirAttackType = context.m_eAirAttackType;
	m_strPrevInfo = context.m_strPrevInfo; // 복사본에서 받은 정보.

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eAirAttackType);

    // 4. Attack 상태 초기화
    State_Reset();

    // 5. 무기 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음(Attack은)
    m_iPartType = CYuno::PARTTYPE::TYPE_END; // 추후 애니메이션에 따른. 분기문 필요.

    // 6. 무기에 Bone 붙이기. + Offset 추가.
    _string strBoneName = "";
  
	m_fSpeed = 2.f;
	m_pYuno->Set_Gravity(true);
	m_pYuno->Rotate_TargetPosition(); // 처음 공격한 곳을 타겟으로 공격.
}

void CYunoAirAttack::OnUpdate(_float fTimeDelta)
{
    CAirState::OnUpdate(fTimeDelta);

    // 0. 입력 확인
    Handle_Input();

    // 1. Attack 업데이트
    Update_AttackAnimations(fTimeDelta);

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    // 3. 전환 조건 체크
    Check_StateTransition(fTimeDelta);

    State_Reset();
}

void CYunoAirAttack::OnExit()
{
    CAirState::OnExit();

	if (m_iPartType != CYuno::PARTTYPE::TYPE_END)
		m_pYuno->PartActivate(m_iPartType, false);

    m_pYuno->Set_Gravity(true);
    m_fSpeed = 0.f;

	// 공격 콜라이더 비활성화
	m_pYuno->Collider_Active(TEXT("Main|X|X"), false);
}

void CYunoAirAttack::Handle_Input()
{
}

void CYunoAirAttack::Update_AttackAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선) // 거리 계산에 따른 Animation Scale 조절.
	m_fRootMotionScale = m_pYuno->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations[m_iCurrentAnimIdx].fRootMotionRate * m_fRootMotionScale;

	// 1. 애니메이션 실행.
    CCharacterState::Play_Animation(m_pYuno, fTimeDelta, m_fAnimationScale);


	EYunoAirAttackType eAirAttackType = static_cast<EYunoAirAttackType>(m_iCurrentAnimIdx);
    _vector vLook = m_pYuno->Get_LookVector();

    if (eAirAttackType == EYunoAirAttackType::AIRATTACK_LOOP)
        m_pYuno->Move_Fall(fTimeDelta, m_fSpeed);
    
}

void CYunoAirAttack::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pYuno->Is_LandCollider(&m_vLandNormal);
}

void CYunoAirAttack::Check_StateTransition(_float fTimeDelta)
{
    EYunoAirAttackType eAirAttackType = static_cast<EYunoAirAttackType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();
    _float fOffsetY = 0.1f;
    //_float fDistanceToGround = m_pYuno->Get_DistanceFromGround(fOffsetY);


    if (IsEscapePossible)
    {
        if (m_States[LAND])
        {
			// 기본 공중 공격 
            if (eAirAttackType == EYunoAirAttackType::AIRATTACK_LOOP)
            {
                m_iCurrentAnimIdx = ENUM_CLASS(EYunoAirAttackType::AIRATTACK_END);
                return;
            }
        }
    }

	// 애니메이션이 끝나면?
    if (m_IsAnimationEnd)
    {
		if (eAirAttackType == EYunoAirAttackType::AIRATTACK_END)
		{
			OnExit();
			m_pYuno->Activate(false);
			// SequenceCharacter에게 종료를 알립니다.
		}

		// 기본 공중 공격
        if (eAirAttackType == EYunoAirAttackType::AIRATTACK_START)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(EYunoAirAttackType::AIRATTACK_LOOP); // 떨어지게.
            m_fSpeed = 2.f;
            return;
        }
          

        // 땅에 안닿으면? => AirAttack Loop가 아닌 경우에는 Fall로 변경.
        if (!m_States[LAND])
        {
			// Loop면? 재진입.
			if (eAirAttackType == EYunoAirAttackType::AIRATTACK_LOOP)
			{
				m_pYuno->GetStateContextForWrite().m_eAirAttackType = EYunoAirAttackType::AIRATTACK_LOOP;
				m_pYuno->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EYunoAirState::AIR_ATTACK)); // 상위, 하위 상태
				return;
			}
        }

        // 땅에 닿으면.
        if (m_States[LAND])
        {
			// Loop 상태일때 땅에 닿으면 END 애니메이션 실행.
			if (eAirAttackType == EYunoAirAttackType::AIRATTACK_LOOP)
			{
				m_iCurrentAnimIdx = ENUM_CLASS(EYunoAirAttackType::AIRATTACK_END);
				return;
			}

            if (eAirAttackType == EYunoAirAttackType::AIRATTACK_END)
            {
                m_pYuno->GetStateContextForWrite().m_eIdleType = EYunoIdleType::STANDCHANGE;
                m_pYuno->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EYunoGroundState::IDLE));
                return;
            }
        }
        else
        {
            // 모든 조건이 아닌 경우 Idle로
            m_pYuno->GetStateContextForWrite().m_eIdleType = EYunoIdleType::STAND1_ACTION01;
            m_pYuno->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EYunoGroundState::IDLE));
            return;
        }
        
    }

}

void CYunoAirAttack::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EYunoAirAttackType::AIRATTACK_START),"AirAttack_Start", 1.3f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EYunoAirAttackType::AIRATTACK_LOOP),"AirAttack_Loop", 1.3f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EYunoAirAttackType::AIRATTACK_END),"AirAttack_End", 1.3f, 55.f, 1.f);
}

void CYunoAirAttack::State_Reset()
{
    for (_uint i = 0; i < AIRATTACKSTATE::END; ++i)
        m_States[i] = false;
}

// 예외적인 애니메이션에 관련해서는 RootMotion Scale을 조절합니다. => 최소 수치를 보장한다?
void CYunoAirAttack::Handle_Animation_SpecialState()
{
	EYunoAirAttackType eAirAttackType = static_cast<EYunoAirAttackType>(m_iCurrentAnimIdx);
	
	
}



CYunoAirAttack* CYunoAirAttack::Create(CCharacter* pOwner)
{
    CYunoAirAttack* pInstance = new CYunoAirAttack();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CYunoAirAttack");
    }

    return pInstance;
}

void CYunoAirAttack::Free()
{
    CAirState::Free();
}
