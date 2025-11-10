#include "ClientPch.h"
#include "AugustaHit.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaHit::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CHitState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}

void CAugustaHit::OnEnter(void* pArg)
{
    CHitState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaHitType eHitType = context.m_eHitType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eHitType);

	// 4. 상태 리셋.
    State_Reset();

	// 5. Hit Description을 이용하여 시작 초기 작업을 정의합니다.
	Enter_Hit();
	
	// 6. 중력 적용
    m_pAugusta->Set_Gravity(true);
}

void CAugustaHit::OnUpdate(_float fTimeDelta)
{
    CHitState::OnUpdate(fTimeDelta);

    // 0. 키입력 체크
    Handle_Input();

    // 1. 애니메이션 갱신
    Update_HitAnimation(fTimeDelta);

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    // 3. 전환 체크
    Check_StateTransition(fTimeDelta);

    // 상태 리셋;
    State_Reset();
}

void CAugustaHit::OnExit()
{
    CHitState::OnExit();
    m_pAugusta->Set_Gravity(false);

	// Hit 판정 끝났으므로 정보 초기화
	m_pAugusta->ClearPendingHit();
	//m_pAugusta->Set_Hit(false);
	
}



void CAugustaHit::Enter_Hit()
{
	// 0. Hit 정보 가져오기.
	const CCharacter::HIT_DESC* pDesc = m_pAugusta->GetPendingHitDesc();

	// 1. 현재 레이어
	COLLISIONLAYER eLayer = static_cast<COLLISIONLAYER>(pDesc->iLayer);

	// 2. 스킬 판정.
	_bool IsSkill = (eLayer == COLLISIONLAYER::ENEMY_SKILL);

	// 3. 바로 회전.
	m_pAugusta->Rotate_HitTarget(pDesc->pTransform);

	// 4. 땅 판정.
	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);

	// 5. 애니메이션 선정.
	if (!m_States[LAND])
	{
		m_iCurrentAnimIdx = ENUM_CLASS(EAugustaHitType::BEHIT_FLY_FALL);
	}
	else
	{
		switch (eLayer)
		{
		case COLLISIONLAYER::ENEMY_ATTACK:
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaHitType::BEHIT_S_L);
			break;
		case COLLISIONLAYER::ENEMY_HARDATTACK:
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaHitType::BEHIT_B_L);
			break;
		case COLLISIONLAYER::ENEMY_SKILL:
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaHitType::BEHIT_FLY_FALL);
			break;
		}
	}
}

void CAugustaHit::Handle_Input()
{
    m_eDir = m_pAugusta->Calculate_Direction(); // 방향 계산.
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    
}

void CAugustaHit::Update_HitAnimation(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
	EAugustaHitType eHitType = static_cast<EAugustaHitType>(m_iCurrentAnimIdx);

	// Hit Type이 뒷점프면?
	if (eHitType == EAugustaHitType::BEHIT_FLY_FALL)
	{
		m_pAugusta->Move_Fall(fTimeDelta, 0.1f); // 미세하게 떨어지게
	}
}

void CAugustaHit::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
}

void CAugustaHit::Check_StateTransition(_float fTimeDelta)
{
    EAugustaHitType eHitType = static_cast<EAugustaHitType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

	if (IsEscapePossible)
	{
		if (m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
		        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
				return;
			}

			if (m_States[JUMP])
			{
				m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
				return;
			}
			
		}

		if (!m_States[LAND])
		{
			if (m_States[JUMP])
			{
				m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
				return;
			}
		}
	}

	if (m_IsAnimationEnd)
	{
		if (m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
				return;
			}

			if (m_States[JUMP])
			{
				m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
				return;
			}
			else
			{
				if (eHitType == EAugustaHitType::BEHIT_FLY_FALL) 
				{
					m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STANDUP; // Idle 전용 일어나는 모션.
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
					return;
				}
				else
				{
					m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1; // Idle 전용 일어나는 모션.
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
					return;
				}
			}
		}

		if (!m_States[LAND])
		{
			if (m_States[JUMP])
			{
				m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_SECOND_F;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
				return;
			}
			else
			{
				m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL));
				return;
			}
		}
	}

    
}


void CAugustaHit::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_B_L), "Behit_B_L", 1.f, 40.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_B_R), "Behit_B_R", 1.f, 40.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_FLY_FALL), "Behit_Fly_Fall", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_FLY_LOOP), "Behit_Fly_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_FLY_START), "Behit_Fly_Start", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_HOVER), "Behit_Hover", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_PRESS), "Behit_Press", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_PUSH_FALL), "Behit_Push_Fall", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_PUSH_LOOP), "Behit_Push_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_PUSH_START), "Behit_Push_Start", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_S_L), "Behit_S_L", 1.f, 40.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_S_R), "Behit_S_R", 1.f, 40.f);

}

void CAugustaHit::State_Reset()
{
    for (_uint i = 0; i < HITSTATE::END; ++i)
        m_States[i] = false;
}



CAugustaHit* CAugustaHit::Create(class CGameObject* pOwner)
{
    CAugustaHit* pInstance = new CAugustaHit();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaHit");
        return nullptr;
    }

    return pInstance;
}

void CAugustaHit::Free()
{
    CHitState::Free();
}
