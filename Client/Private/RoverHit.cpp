#include "ClientPch.h"
#include "RoverHit.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"

HRESULT CRoverHit::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CHitState::Initialize(pCharacter)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pCharacter);
    ASSERT_CRASH(m_pRover);

    Setup_Animations();
    return S_OK;
}

void CRoverHit::OnEnter(void* pArg)
{
    CHitState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverHitType eHitType = context.m_eHitType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eHitType);

	// 4. 상태 리셋.
    State_Reset();

	m_strPrevInfo = context.m_strPrevInfo;

	// 5. Hit Description을 이용하여 시작 초기 작업을 정의합니다.
	if (context.m_strPrevInfo.empty())
		Enter_Hit();
	
	// 6. 중력 적용
    m_pRover->Set_Gravity(true);
}

void CRoverHit::OnUpdate(_float fTimeDelta)
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

void CRoverHit::OnExit()
{
    CHitState::OnExit();
    m_pRover->Set_Gravity(false);

	// Hit 판정 끝났으므로 정보 초기화
	m_pRover->ClearPendingHit();
	m_pRover->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::HIT));
}



void CRoverHit::Enter_Hit()
{
	// 0. Hit 정보 가져오기.
	const CCharacter::HIT_DESC* pDesc = m_pRover->GetPendingHitDesc();

	// 1. 현재 레이어
	COLLISIONLAYER eLayer = static_cast<COLLISIONLAYER>(pDesc->iLayer);

	// 2. 스킬 판정.
	_bool IsSkill = (eLayer == COLLISIONLAYER::ENEMY_SKILL);

	// 3. 바로 회전.
	m_pRover->Rotate_HitTarget(pDesc->pTransform);

	// 4. 땅 판정.
	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);

	// 5. 애니메이션 선정.
	if (!m_States[LAND])
	{
		/*m_iCurrentAnimIdx = ENUM_CLASS(ERoverHitType::BEHIT_FLY_FALL);*/
		m_iCurrentAnimIdx = ENUM_CLASS(ERoverHitType::BEHIT_FLY_START);

		// 내 위치 - 공격자 위치 = 밀려날 방향
		_vector vMyPos = m_pRover->Get_Position();
		_vector vAttackerPos = pDesc->pTransform->Get_State(STATE::POSITION);
		_vector vHitDir = vMyPos - vAttackerPos;
		vHitDir = XMVectorSetY(vHitDir, 0.f);
		if (XMVectorGetX(XMVector3Length(vHitDir)) < 0.01f)
		{
			vHitDir = m_pRover->Get_LookVector_NoPitch() * -1.f;
		}
		else
		{
			vHitDir = XMVector3Normalize(vHitDir);
		}


		_float fKnockbackPower = 2.0f;
		_float fUpForce = 2.0f;
		m_vKnockbackVelocity = vHitDir * fKnockbackPower; // 뒤로 밀리는 힘
		m_vKnockbackVelocity = XMVectorSetY(m_vKnockbackVelocity, fUpForce); // 위로 솟구치는 힘
	}
	else
	{
		switch (eLayer)
		{
		case COLLISIONLAYER::ENEMY_ATTACK:
			m_iCurrentAnimIdx = ENUM_CLASS(ERoverHitType::BEHIT_S_L);
			break;
		case COLLISIONLAYER::ENEMY_HARDATTACK:
			m_iCurrentAnimIdx = ENUM_CLASS(ERoverHitType::BEHIT_B_L);
			break;
		case COLLISIONLAYER::ENEMY_SKILL:
			m_iCurrentAnimIdx = ENUM_CLASS(ERoverHitType::BEHIT_FLY_FALL);
			break;
		}
	}
}

void CRoverHit::Handle_Input()
{
    m_eDir = m_pRover->Calculate_Direction(); // 방향 계산.
    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    
}

void CRoverHit::Update_HitAnimation(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pRover, fTimeDelta);
	ERoverHitType eHitType = static_cast<ERoverHitType>(m_iCurrentAnimIdx);
	if (eHitType == ERoverHitType::BEHIT_FLY_FALL)
	{
		m_pRover->Move_Fall(fTimeDelta, 0.1f); // 미세하게 떨어지게
	}

	if (eHitType == ERoverHitType::BEHIT_FLY_START ||
		eHitType == ERoverHitType::BEHIT_FLY_LOOP)
	{
		_vector vDir = XMVector3Normalize(m_vKnockbackVelocity);
		_float fSpeed = XMVectorGetX(XMVector3Length(m_vKnockbackVelocity));

		m_pRover->Move_Direction(vDir, fTimeDelta, fSpeed);

		_float fGravity = 5.f;
		_vector vVelocityY = XMVectorSet(0.f, XMVectorGetY(m_vKnockbackVelocity), 0.f, 0.f);
		vVelocityY = XMVectorSetY(vVelocityY, XMVectorGetY(vVelocityY) - fGravity * fTimeDelta);

		_float fDrag = 2.0f; // 마찰 계수
		_vector vVelocityXZ = XMVectorSetY(m_vKnockbackVelocity, 0.f);
		vVelocityXZ = vVelocityXZ * (1.0f - fDrag * fTimeDelta); // 점점 느려지게

		m_vKnockbackVelocity = vVelocityXZ + vVelocityY;
	}
}

void CRoverHit::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
    m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
}

void CRoverHit::Check_StateTransition(_float fTimeDelta)
{
    ERoverHitType eHitType = static_cast<ERoverHitType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

	if (IsEscapePossible)
	{
		if (m_States[LAND])
		{
			if (eHitType == ERoverHitType::BEHIT_FLY_LOOP ||
				eHitType == ERoverHitType::BEHIT_FLY_START) // Loop 라면?
			{
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverHitType::BEHIT_FLY_FALL); // Fall로 변경.
				return;
			}

			if (m_States[MOVE])
			{
				m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
		        m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
				return;
			}

			if (m_States[JUMP])
			{
				m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_SECOND_F;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP));
				return;
			}
		}

		if (!m_States[LAND])
		{
			if (m_States[JUMP])
			{
				m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_SECOND_F;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP));
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
				m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
				return;
			}

			if (m_States[JUMP])
			{
				m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_SECOND_F;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP));
				return;
			}

			if (eHitType == ERoverHitType::BEHIT_FLY_LOOP) // Loop 라면?
			{
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverHitType::BEHIT_FLY_FALL); // Fall로 변경.
				return;
			}
			else if (eHitType == ERoverHitType::BEHIT_FLY_START)
			{
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverHitType::BEHIT_FLY_FALL); // Fall로 변경.
				return;
			}
			else if (eHitType == ERoverHitType::BEHIT_FLY_FALL)
			{
				m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STANDUP; // Idle 전용 일어나는 모션.
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
				return;
			}
			else
			{
				m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1_ACTION02; // Idle 전용 일어나는 모션.
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
				return;
			}
			
		}

		if (!m_States[LAND])
		{
			if (m_States[JUMP])
			{
				m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_SECOND_F;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP));
				return;
			}
			else if (ERoverHitType::BEHIT_FLY_START == eHitType) // Fly Start라면?
			{
				m_iCurrentAnimIdx = ENUM_CLASS(ERoverHitType::BEHIT_FLY_LOOP); // Fly Loop로 전환.
				return;
			}
			else
			{
				m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL));
				return;
			}
		}
	}

    
}


void CRoverHit::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERoverHitType::BEHIT_B_L), "Behit_B_L", 1.f, 40.f);
    CState::Add_Animations(ENUM_CLASS(ERoverHitType::BEHIT_B_R), "Behit_B_R", 1.f, 40.f);
    CState::Add_Animations(ENUM_CLASS(ERoverHitType::BEHIT_FLY_FALL), "Behit_Fly_Fall", 1.5f, 30.f, 2.f);
    CState::Add_Animations(ENUM_CLASS(ERoverHitType::BEHIT_FLY_LOOP), "Behit_Fly_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverHitType::BEHIT_FLY_START), "Behit_Fly_Start", 1.5f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverHitType::BEHIT_HOVER), "Behit_Hover", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverHitType::BEHIT_PRESS), "Behit_Press", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverHitType::BEHIT_PUSH_FALL), "Behit_Push_Fall", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(ERoverHitType::BEHIT_PUSH_LOOP), "Behit_Push_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverHitType::BEHIT_PUSH_START), "Behit_Push_Start", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(ERoverHitType::BEHIT_S_L), "Behit_S_L", 1.f, 40.f);
    CState::Add_Animations(ENUM_CLASS(ERoverHitType::BEHIT_S_R), "Behit_S_R", 1.f, 40.f);

}

void CRoverHit::State_Reset()
{
    for (_uint i = 0; i < HITSTATE::END; ++i)
        m_States[i] = false;
}



CRoverHit* CRoverHit::Create(CCharacter* pOwner)
{
    CRoverHit* pInstance = new CRoverHit();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverHit");
        return nullptr;
    }

    return pInstance;
}

void CRoverHit::Free()
{
    CHitState::Free();
}
