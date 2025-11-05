#include "ClientPch.h"
#include "AugustaAirAttack.h"
#include "Augusta.h"
#include "StateMachine.h"

HRESULT CAugustaAirAttack::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

	// 매핑.

    return S_OK;
}


void CAugustaAirAttack::OnEnter()
{
    CAirState::OnEnter();

    // 1. 복사본 Context 받아오기
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaAirAttackType eAirAttackType = context.m_eAirAttackType;
	m_strPrevInfo = context.m_strPrevInfo; // 복사본에서 받은 정보.

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eAirAttackType);

    // 4. Attack 상태 초기화
    State_Reset();

    // 5. 무기 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음(Attack은)
    m_iPartType = CAugusta::PARTTYPE::PART_BAYONET; // 추후 애니메이션에 따른. 분기문 필요.

    // 6. 무기에 Bone 붙이기. + Offset 추가.
    _string strBoneName = "";


    // 7. 다른 애니메이션당 필요한 상태 재정의
    switch (eAirAttackType)
    {
        case EAugustaAirAttackType::AIRATTACK_HACKDOWN_START:
        {
            strBoneName = "WeaponProp05";
            m_fSpeed = 0.5f;
            m_pAugusta->Set_Gravity(true);

			
			if (m_strPrevInfo == "Griffon")
			{
				m_iSubPartType = CAugusta::PARTTYPE::PART_GRIFFON;
				m_pAugusta->Set_SocketMatrixToParts(m_iPartType, "Root");
				m_pAugusta->Clear_PartAnimation(m_iSubPartType, m_PartsAnimations[m_Animations[m_iCurrentAnimIdx].strAnimName]);
				m_pAugusta->PartActivate(m_iSubPartType, true);
			}
			// Enter에 들어오면 한번 회전. => 애니메이션 따라 다르게?
			m_pAugusta->Rotate_Target();

            break;
        }

        case EAugustaAirAttackType::AIRATTACK_HACKDOWN_SP_END:
        {
            strBoneName = "WeaponProp05";
            m_fSpeed = 0.f;
            m_pAugusta->Set_Gravity(true);
			if (m_strPrevInfo == "Griffon")
			{
				m_iSubPartType = CAugusta::PARTTYPE::PART_GRIFFON;
				m_pAugusta->Set_SocketMatrixToParts(m_iPartType, "Root");
				m_pAugusta->Clear_PartAnimation(m_iSubPartType, m_PartsAnimations[m_Animations[m_iCurrentAnimIdx].strAnimName]);
				m_pAugusta->PartActivate(m_iSubPartType, true);
			}

			// Enter에 들어오면 한번 회전. => 애니메이션 따라 다르게?
			m_pAugusta->Rotate_Target();
            break;
        }
        case EAugustaAirAttackType::AIRATTACK_START:
        {
			// Enter에 들어오면 한번 회전. => 애니메이션 따라 다르게?
			m_pAugusta->Rotate_Target();

            strBoneName = "WeaponProp02";
            m_pAugusta->Set_Gravity(false);
            break;
        }
        case EAugustaAirAttackType::AIRATTACK_END:
        {
			// Enter에 들어오면 한번 회전. => 애니메이션 따라 다르게?
			m_pAugusta->Rotate_Target();

            strBoneName = "WeaponProp02";
            m_pAugusta->Set_Gravity(false);
            break;
        }
    }
	
	// 공통으로 무기는 다 나옴.
    m_pAugusta->PartActivate(m_iPartType, true);
	m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
    m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
}

void CAugustaAirAttack::OnUpdate(_float fTimeDelta)
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

void CAugustaAirAttack::OnExit()
{
    CAirState::OnExit();

	if (m_iPartType != CAugusta::PARTTYPE::TYPE_END)
	{
		m_pAugusta->PartActivate(m_iPartType, false);
	}

	if (m_iSubPartType != CAugusta::PARTTYPE::TYPE_END)
	{
		m_pAugusta->PartActivate(m_iSubPartType, false);
	}

    m_pAugusta->Set_Gravity(true);
    m_fSpeed = 0.f;
    
    m_iPartType = CAugusta::PARTTYPE::TYPE_END;
	m_iSubPartType = CAugusta::PARTTYPE::TYPE_END;
}

void CAugustaAirAttack::Handle_Input()
{
    m_States[ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[DOUBLE_JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
    
	// 이전 E스킬이 그리폰이였다면 까지 조건이 있어야함.
}

void CAugustaAirAttack::Update_AttackAnimations(_float fTimeDelta)
{
    // 1. 애니메이션 실행.
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // 2.
    _vector vLook = m_pAugusta->Get_LookVector();

    EAugustaAirAttackType eAirAttackType = static_cast<EAugustaAirAttackType>(m_iCurrentAnimIdx);
    if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_HACKDOWN_START)
    {
        m_pAugusta->Move_Direction(vLook, fTimeDelta, m_fSpeed); // 조금씩 앞으로 이동?
    }

    if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_LOOP)
    {
        m_pAugusta->Move_Fall(fTimeDelta, m_fSpeed);
    }

    // Attack State에 해당하는 경우 모두 Animation이 존재.
    if (m_iPartType != CAugusta::PARTTYPE::TYPE_END)
    {
		// 애니메이션 속도 서로 Sync 맞추기.
        m_pAugusta->Play_PartAnimation(
            m_iPartType,
            m_Animations[m_iCurrentAnimIdx].strAnimName,
			m_Animations[m_iCurrentAnimIdx].fSpeed * fTimeDelta, nullptr
        );
    }

	// Griffon
	if (m_iSubPartType != CAugusta::PARTTYPE::TYPE_END)
	{
		
		m_pAugusta->Play_PartAnimation(
			m_iSubPartType,
			m_PartsAnimations[m_Animations[m_iCurrentAnimIdx].strAnimName],
			m_Animations[m_iCurrentAnimIdx].fSpeed * fTimeDelta, nullptr, 1.f, true, true, true, false
		);
	}
    
}

void CAugustaAirAttack::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pAugusta->Is_Land();
}

void CAugustaAirAttack::Check_StateTransition(_float fTimeDelta)
{
    EAugustaAirAttackType eAirAttackType = static_cast<EAugustaAirAttackType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();
    _float fOffsetY = 0.1f;
    _float fDistanceToGround = m_pAugusta->Get_DistanceFromGround(fOffsetY);


    if (IsEscapePossible)
    {
		//// 모션 끝날때까지 기달리기.
		//if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_HACKDOWN_START)
		//{
		//	m_pAugusta->GetStateContextForWrite().m_eAirAttackType = EAugustaAirAttackType::AIRATTACK_HACKDOWN_SP_END;
		//	m_pAugusta->GetStateContextForWrite().m_strPrevInfo = "Griffon";
		//	m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::AIR_ATTACK));
		//	return;
		//}

        if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_START)
        {
            if (m_States[DOUBLE_JUMP])
            {
                m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F; // 애니메이션 상태 => 블랙보드에 기입.        
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP)); // 상위, 하위 상태
                return;
            }
        }

        if (m_States[LAND])
        {
            if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_LOOP)
            {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_END);
                return;
            }

            if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_HACKDOWN_SP_END)
            {
                if (m_States[MOVE])
                {
                    m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
                    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
                    return;
                }

				if (!m_States[MOVE])
				{
					m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
					return;
				}
            }

            if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_END)
            {
                if (m_States[MOVE])
                {
                    m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
                    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
                    return;
                }
            }
        }
    }

    if (m_IsAnimationEnd)
    {
        if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_HACKDOWN_START)
        {
			m_pAugusta->GetStateContextForWrite().m_eAirAttackType = EAugustaAirAttackType::AIRATTACK_HACKDOWN_SP_END;
			m_pAugusta->GetStateContextForWrite().m_strPrevInfo = "Griffon";
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::AIR_ATTACK));
            return;
        }

        if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_START)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_LOOP); // 떨어지게.
            m_fSpeed = 2.f;
            return;
        }
          

        // 땅에 안닿으면? => AirAttack Loop가 아닌 경우에는 Fall로 변경.
        if (!m_States[LAND])
        {
            if (eAirAttackType != EAugustaAirAttackType::AIRATTACK_LOOP)
            {
                m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
                return;
            }
        }

        // 땅에 닿으면.
        if (m_States[LAND])
        {
            if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_HACKDOWN_LOOP)
            {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_HACKDOWN_SP_END);
                return;
            }

            // Loop 상태일때 땅에 닿으면 END 애니메이션 실행.
            if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_LOOP)
            {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_END);
                return;
            }

            if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_HACKDOWN_SP_END)
            {
                if (m_States[MOVE])
                {
                    m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
                    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
                    return;
                }
                else
                {
                    m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
                    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
                    return;
                }
            }

            if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_END)
            {
                if (m_States[MOVE])
                {
                    m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
                    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
                    return;
                }
                else
                {
                    m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
                    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
                    return;
                }
                
            }
        }
        else
        {
            // 모든 조건이 아닌 경우 Idle로
            m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
            return;
        }
        
    }

}

void CAugustaAirAttack::SetUp_Animations()
{
    
    CState::Add_Animations(ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_HACKDOWN_START),"AirAttack_HackDown_Start", 1.5f, 20.f, 3.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_HACKDOWN_LOOP),"AirAttack_HackDown_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_HACKDOWN_SP_END),"AirAttack_HackDown_Sp_End", 1.5f, 65.f, 3.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_START),"AirAttack_Start", 1.f, 10.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_LOOP),"AirAttack_Loop", 1.f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_END),"AirAttack_End", 1.3f, 50.f, 1.f);

	// Griffon 전용 애니메이션 맵 등록.
	m_PartsAnimations.emplace("AirAttack_HackDown_Start", "SA1Shouwangjiu_AirAttack_Start");
	m_PartsAnimations.emplace("AirAttack_HackDown_Sp_End", "SA1Shouwangjiu_AirAttack_End");
}

void CAugustaAirAttack::State_Reset()
{
    for (_uint i = 0; i < AIRATTACKSTATE::END; ++i)
        m_States[i] = false;
}

CAugustaAirAttack* CAugustaAirAttack::Create(class CGameObject* pOwner)
{
    CAugustaAirAttack* pInstance = new CAugustaAirAttack();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaAirAttack");
    }

    return pInstance;
}

void CAugustaAirAttack::Free()
{
    CAirState::Free();
}
