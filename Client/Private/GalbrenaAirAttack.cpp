#include "ClientPch.h"
#include "GalbrenaAirAttack.h"
#include "Galbrena.h"
#include "StateMachine.h"

HRESULT CGalbrenaAirAttack::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pOwner);
    ASSERT_CRASH(m_pGalbrena);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

	// 매핑.

    return S_OK;
}


void CGalbrenaAirAttack::OnEnter(void* pArg)
{
    CAirState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaAirAttackType eAirAttackType = context.m_eAirAttackType;
	m_strPrevInfo = context.m_strPrevInfo; // 복사본에서 받은 정보.

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eAirAttackType);

    // 4. Attack 상태 초기화
    State_Reset();

	// 5. 무기 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음(Attack은)
	m_iPartType = CGalbrena::PARTTYPE::TYPE_END; // 추후 애니메이션에 따른. 분기문 필요.


	// 6. 무기에 Bone 붙이기. + Offset 추가.
	_string strMainBoneName = "";
	_string strSubBoneName = "";

	m_pGalbrena->Set_Gravity(true);

	switch (eAirAttackType)
	{
	case EGalbrenaAirAttackType::AIRATTACK_START:
		m_pGalbrena->Set_Gravity(false);
		break;
	case EGalbrenaAirAttackType::AIRATTACK_LOOP_1:
		strMainBoneName = "WeaponProp01";
		strSubBoneName = "WeaponProp02";
		m_iPartType = CGalbrena::PARTTYPE::PART_FIRSTGUN;
		m_iSubPartType = CGalbrena::PARTTYPE::PART_SECONDGUN;
		m_pGalbrena->Set_Gravity(false);
		m_pGalbrena->PartActivate(m_iPartType, true); // 파츠 변경. 
		m_pGalbrena->Clear_PartAnimation(m_iPartType, m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName));
		m_pGalbrena->Set_SocketMatrixToParts(m_iPartType, strMainBoneName);
		m_pGalbrena->PartActivate(m_iSubPartType, true);
		m_pGalbrena->Clear_PartAnimation(m_iSubPartType, m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName));
		m_pGalbrena->Set_SocketMatrixToParts(m_iSubPartType, strSubBoneName);
		break;
	}

   

  
	m_fSpeed = 2.f;

	

	m_pGalbrena->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
}

void CGalbrenaAirAttack::OnUpdate(_float fTimeDelta)
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

void CGalbrenaAirAttack::OnExit()
{
    CAirState::OnExit();

	if (m_iPartType != CGalbrena::PARTTYPE::TYPE_END)
	{
		m_pGalbrena->PartActivate(m_iPartType, false);
	}

	if (m_iSubPartType != CGalbrena::PARTTYPE::TYPE_END)
	{
		m_pGalbrena->PartActivate(m_iSubPartType, false);
	}

    m_pGalbrena->Set_Gravity(true);
    m_fSpeed = 0.f;

	m_iPartType = CGalbrena::PARTTYPE::TYPE_END;
	m_iSubPartType = CGalbrena::PARTTYPE::TYPE_END;
	m_pGalbrena->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));

	m_pGalbrena->Collider_Active(TEXT("Main|X|X"), false);
}

void CGalbrenaAirAttack::Handle_Input()
{
    m_States[ATTACK] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[DOUBLE_JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
    
	// 이전 E스킬이 그리폰이였다면 까지 조건이 있어야함.
}

void CGalbrenaAirAttack::Update_AttackAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선) // 거리 계산에 따른 Animation Scale 조절.
	m_fRootMotionScale = m_pGalbrena->Calculate_RootMotionScale();
	//m_fAnimationScale = m_Animations[m_iCurrentAnimIdx].fRootMotionRate * m_fRootMotionScale;
	m_fAnimationScale = m_Animations[m_iCurrentAnimIdx].fRootMotionRate;


	// 1. 특정 애니메이션에서는 비율 조정
	Handle_Animation_SpecialState();
	

	// 2. 애니메이션 실행.
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta, m_fAnimationScale);


	EGalbrenaAirAttackType eAirAttackType = static_cast<EGalbrenaAirAttackType>(m_iCurrentAnimIdx);
    _vector vLook = m_pGalbrena->Get_LookVector();

    if (eAirAttackType == EGalbrenaAirAttackType::AIRATTACK_LOOP_2)
    {
        m_pGalbrena->Move_Fall(fTimeDelta, m_fSpeed);
    }

	
	if (m_iPartType != CGalbrena::PARTTYPE::TYPE_END)
	{
		m_pGalbrena->Play_PartAnimation(
			m_iPartType,
			m_Animations.at(m_iCurrentAnimIdx).strAnimName,
			fTimeDelta, nullptr
		);
	}

	if (m_iSubPartType != CGalbrena::PARTTYPE::TYPE_END)
	{
		m_pGalbrena->Play_PartAnimation(
			m_iSubPartType,
			m_Animations.at(m_iCurrentAnimIdx).strAnimName,
			fTimeDelta, nullptr
		);
	}
    
}

void CGalbrenaAirAttack::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
}

void CGalbrenaAirAttack::Check_StateTransition(_float fTimeDelta)
{
    EGalbrenaAirAttackType eAirAttackType = static_cast<EGalbrenaAirAttackType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();

    if (IsEscapePossible)
    {
		if (eAirAttackType == EGalbrenaAirAttackType::AIRATTACK_LOOP_1)
		{
			if (m_States[ATTACK])
			{
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaAirAttackType::AIRATTACK_LOOP_1);
				//m_pGalbrena->GetStateContextForWrite().m_eAirAttackType = EGalbrenaAirAttackType::AIRATTACK_LOOP_1; // 애니메이션 상태 => 블랙보드에 기입.        
				//m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::AIR_ATTACK)); // 상위, 하위 상태
				return;
			}
		}

		// 0. Loop 도중에 공격키 한번 더누르면?
		if (eAirAttackType == EGalbrenaAirAttackType::AIRATTACK_LOOP_2)
		{
			if (m_States[ATTACK])
			{
				m_pGalbrena->GetStateContextForWrite().m_eAirAttackType = EGalbrenaAirAttackType::AIRATTACK_LOOP_1; // 애니메이션 상태 => 블랙보드에 기입.        
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::AIR_ATTACK)); // 상위, 하위 상태
				return;
			}
		}

		// 0. End면 Move로 전환 가능.
		if (eAirAttackType == EGalbrenaAirAttackType::AIRATTACK_END)
		{
			if (m_States[MOVE])
			{
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN)); // 상위, 하위 상태
				return;
			}
		}

		// 1. Jump로 생략 가능.
		if (eAirAttackType != EGalbrenaAirAttackType::AIRATTACK_END)
		{
			if(m_States[JUMP])
			{
				m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F; // 애니메이션 상태 => 블랙보드에 기입.        
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP)); // 상위, 하위 상태
				return;
			}
		}

		// 2. 땅에 닿으면 End로 자동 전환.
		if (m_States[LAND])
		{
			if (eAirAttackType != EGalbrenaAirAttackType::AIRATTACK_END)
			{
				m_pGalbrena->GetStateContextForWrite().m_eAirAttackType = EGalbrenaAirAttackType::AIRATTACK_END;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::AIR_ATTACK)); // 상위, 하위 상태
				return;
			}
		}


		
    }

	// 애니메이션이 끝나면?
    if (m_IsAnimationEnd)
    {
		// 기본 공중 공격 2번째..
		if (eAirAttackType == EGalbrenaAirAttackType::AIRATTACK_START02)
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaAirAttackType::AIRATTACK_LOOP_2); // 떨어지게.
			m_fSpeed = 2.f;
			return;
		}

		// 기본 공중 공격
        if (eAirAttackType == EGalbrenaAirAttackType::AIRATTACK_START)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaAirAttackType::AIRATTACK_START02); // 떨어지게.
            m_fSpeed = 2.f;
            return;
        }
          

        // 땅에 안닿으면? => AirAttack Loop가 아닌 경우에는 Fall로 변경.
        if (!m_States[LAND])
        {
			// Loop면? 재진입.
			if (eAirAttackType == EGalbrenaAirAttackType::AIRATTACK_LOOP_2)
			{
				m_pGalbrena->GetStateContextForWrite().m_eAirAttackType = EGalbrenaAirAttackType::AIRATTACK_LOOP_2;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::AIR_ATTACK)); // 상위, 하위 상태
				return;
			}
			else
			{
				m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL)); // 상위, 하위 상태
				return;
			}
			
        }

        // 땅에 닿으면.
        if (m_States[LAND])
        {
			

			if (eAirAttackType == EGalbrenaAirAttackType::AIRATTACK_END)
			{
				if (m_States[MOVE])
				{
					m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
					return;
				}
				else
				{
					m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND2;
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
					return;
				}

			}

			// Loop 상태일때 땅에 닿으면 END 애니메이션 실행.
			if (eAirAttackType == EGalbrenaAirAttackType::AIRATTACK_LOOP_2 || eAirAttackType == EGalbrenaAirAttackType::AIRATTACK_START02 || eAirAttackType == EGalbrenaAirAttackType::AIRATTACK_START)
			{
				m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaAirAttackType::AIRATTACK_END);
				return;
			}
        }
        else
        {
            // 모든 조건이 아닌 경우 Idle로
            m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND1;
            m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
            return;
        }
        
    }

}

void CGalbrenaAirAttack::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAirAttackType::AIRATTACK_START),"AirAttack_Start", 1.0f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAirAttackType::AIRATTACK_START02),"AirAttack_Start02", 1.0f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAirAttackType::AIRATTACK_LOOP_1),"AirAttack_Loop_1", 1.5f, 5.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAirAttackType::AIRATTACK_LOOP_2),"AirAttack_Loop_2", 1.0f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAirAttackType::AIRATTACK_END),"AirAttack_End", 1.3f, 45.f, 1.f);

	// Griffon 전용 애니메이션 맵 등록.
	m_PartsAnimations.emplace("AirAttack_Start", "Gun01");
	m_PartsAnimations.emplace("AirAttack_Start02", "Gun01");
	m_PartsAnimations.emplace("AirAttack_Loop_1", "Gun01");
	m_PartsAnimations.emplace("AirAttack_Loop_2", "Gun01");
	m_PartsAnimations.emplace("AirAttack_End", "Gun01");
}

void CGalbrenaAirAttack::State_Reset()
{
    for (_uint i = 0; i < AIRATTACKSTATE::END; ++i)
        m_States[i] = false;
}

// 예외적인 애니메이션에 관련해서는 RootMotion Scale을 조절합니다. => 최소 수치를 보장한다?
void CGalbrenaAirAttack::Handle_Animation_SpecialState()
{
	EGalbrenaAirAttackType eAirAttackType = static_cast<EGalbrenaAirAttackType>(m_iCurrentAnimIdx);
}



CGalbrenaAirAttack* CGalbrenaAirAttack::Create(class CGameObject* pOwner)
{
    CGalbrenaAirAttack* pInstance = new CGalbrenaAirAttack();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaAirAttack");
    }

    return pInstance;
}

void CGalbrenaAirAttack::Free()
{
    CAirState::Free();
}
