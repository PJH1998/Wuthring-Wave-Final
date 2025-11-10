#include "ClientPch.h"
#include "RoverAirAttack.h"
#include "Rover.h"
#include "StateMachine.h"

HRESULT CRoverAirAttack::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pOwner);
    ASSERT_CRASH(m_pRover);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

	// 매핑.

    return S_OK;
}


void CRoverAirAttack::OnEnter(void* pArg)
{
    CAirState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverAirAttackType eAirAttackType = context.m_eAirAttackType;
	m_strPrevInfo = context.m_strPrevInfo; // 복사본에서 받은 정보.

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eAirAttackType);

    // 4. Attack 상태 초기화
    State_Reset();

    // 5. 무기 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음(Attack은)
    m_iPartType = CRover::PARTTYPE::TYPE_END; // 추후 애니메이션에 따른. 분기문 필요.

    // 6. 무기에 Bone 붙이기. + Offset 추가.
    _string strBoneName = "";
  
	m_fSpeed = 2.f;

	m_pRover->Set_Gravity(true);
}

void CRoverAirAttack::OnUpdate(_float fTimeDelta)
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

void CRoverAirAttack::OnExit()
{
    CAirState::OnExit();

	if (m_iPartType != CRover::PARTTYPE::TYPE_END)
	{
		m_pRover->PartActivate(m_iPartType, false);
	}

	if (m_iSubPartType != CRover::PARTTYPE::TYPE_END)
	{
		m_pRover->PartActivate(m_iSubPartType, false);
	}

    m_pRover->Set_Gravity(true);
    m_fSpeed = 0.f;
}

void CRoverAirAttack::Handle_Input()
{
    m_States[ATTACK] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[DOUBLE_JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
    
	// 이전 E스킬이 그리폰이였다면 까지 조건이 있어야함.
}

void CRoverAirAttack::Update_AttackAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선) // 거리 계산에 따른 Animation Scale 조절.
	m_fRootMotionScale = m_pRover->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations[m_iCurrentAnimIdx].fRootMotionRate * m_fRootMotionScale;


	// 1. 특정 애니메이션에서는 비율 조정
	Handle_Animation_SpecialState();
	

	// 2. 애니메이션 실행.
    CCharacterState::Play_Animation(m_pRover, fTimeDelta, m_fAnimationScale);


	ERoverAirAttackType eAirAttackType = static_cast<ERoverAirAttackType>(m_iCurrentAnimIdx);
    _vector vLook = m_pRover->Get_LookVector();

    if (eAirAttackType == ERoverAirAttackType::AIRATTACK_LOOP)
    {
        m_pRover->Move_Fall(fTimeDelta, m_fSpeed);
    }
    
}

void CRoverAirAttack::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
}

void CRoverAirAttack::Check_StateTransition(_float fTimeDelta)
{
    ERoverAirAttackType eAirAttackType = static_cast<ERoverAirAttackType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();
    _float fOffsetY = 0.1f;
    //_float fDistanceToGround = m_pRover->Get_DistanceFromGround(fOffsetY);


    if (IsEscapePossible)
    {
        if (eAirAttackType == ERoverAirAttackType::AIRATTACK_START)
        {
            if (m_States[DOUBLE_JUMP])
            {
                m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_SECOND_F; // 애니메이션 상태 => 블랙보드에 기입.        
                m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP)); // 상위, 하위 상태
                return;
            }

        }

        if (m_States[LAND])
        {
			// 기본 공중 공격 
            if (eAirAttackType == ERoverAirAttackType::AIRATTACK_LOOP)
            {
                m_iCurrentAnimIdx = ENUM_CLASS(ERoverAirAttackType::AIRATTACK_END);
                return;
            }

            if (eAirAttackType == ERoverAirAttackType::AIRATTACK_END)
            {
                if (m_States[MOVE])
                {
                    m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
                    m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
                    return;
                }
            }
        }
    }

	// 애니메이션이 끝나면?
    if (m_IsAnimationEnd)
    {
		// 기본 공중 공격
        if (eAirAttackType == ERoverAirAttackType::AIRATTACK_START)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(ERoverAirAttackType::AIRATTACK_LOOP); // 떨어지게.
            m_fSpeed = 2.f;
            return;
        }
          

        // 땅에 안닿으면? => AirAttack Loop가 아닌 경우에는 Fall로 변경.
        if (!m_States[LAND])
        {
			// Loop면? 재진입.
			if (eAirAttackType == ERoverAirAttackType::AIRATTACK_LOOP)
			{
				m_pRover->GetStateContextForWrite().m_eAirAttackType = ERoverAirAttackType::AIRATTACK_LOOP;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::AIR_ATTACK)); // 상위, 하위 상태
				return;
			}
			else
			{
				m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL)); // 상위, 하위 상태
				return;
			}
			
        }

        // 땅에 닿으면.
        if (m_States[LAND])
        {
			// Loop 상태일때 땅에 닿으면 END 애니메이션 실행.
			if (eAirAttackType == ERoverAirAttackType::AIRATTACK_LOOP)
			{
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverAirAttackType::AIRATTACK_END);
				return;
			}

            if (eAirAttackType == ERoverAirAttackType::AIRATTACK_END)
            {
                if (m_States[MOVE])
                {
                    m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
                    m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
                    return;
                }
                else
                {
                    m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1;
                    m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
                    return;
                }
                
            }
        }
        else
        {
            // 모든 조건이 아닌 경우 Idle로
            m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1;
            m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
            return;
        }
        
    }

}

void CRoverAirAttack::SetUp_Animations()
{
    
    
    CState::Add_Animations(ENUM_CLASS(ERoverAirAttackType::AIRATTACK_START),"AirAttack_Start", 1.3f, 16.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(ERoverAirAttackType::AIRATTACK_LOOP),"AirAttack_Loop", 1.3f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(ERoverAirAttackType::AIRATTACK_END),"AirAttack_End", 1.3f, 55.f, 1.f);

	// Griffon 전용 애니메이션 맵 등록.
	m_PartsAnimations.emplace("AirAttack_HackDown_Start", "SA1Shouwangjiu_AirAttack_Start");
	m_PartsAnimations.emplace("AirAttack_HackDown_Loop", "SA1Shouwangjiu_AirAttack_Loop");
	m_PartsAnimations.emplace("AirAttack_HackDown_Sp_End", "SA1Shouwangjiu_AirAttack_End");
}

void CRoverAirAttack::State_Reset()
{
    for (_uint i = 0; i < AIRATTACKSTATE::END; ++i)
        m_States[i] = false;
}

// 예외적인 애니메이션에 관련해서는 RootMotion Scale을 조절합니다. => 최소 수치를 보장한다?
void CRoverAirAttack::Handle_Animation_SpecialState()
{
	ERoverAirAttackType eAirAttackType = static_cast<ERoverAirAttackType>(m_iCurrentAnimIdx);
	
	
}



CRoverAirAttack* CRoverAirAttack::Create(class CGameObject* pOwner)
{
    CRoverAirAttack* pInstance = new CRoverAirAttack();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverAirAttack");
    }

    return pInstance;
}

void CRoverAirAttack::Free()
{
    CAirState::Free();
}
