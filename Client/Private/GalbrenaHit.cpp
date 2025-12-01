#include "ClientPch.h"
#include "GalbrenaHit.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"

HRESULT CGalbrenaHit::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CHitState::Initialize(pCharacter)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pCharacter);
    ASSERT_CRASH(m_pGalbrena);

    Setup_Animations();
    return S_OK;
}

void CGalbrenaHit::OnEnter(void* pArg)
{
    CHitState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaHitType eHitType = context.m_eHitType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eHitType);

	// 4. 상태 리셋.
    State_Reset();

	// 5. Hit Description을 이용하여 시작 초기 작업을 정의합니다.
	if (context.m_strPrevInfo.empty())
		Enter_Hit();
	
	// 6. 중력 적용
    m_pGalbrena->Set_Gravity(true);
}

void CGalbrenaHit::OnUpdate(_float fTimeDelta)
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

void CGalbrenaHit::OnExit()
{
    CHitState::OnExit();
    m_pGalbrena->Set_Gravity(false);

	// Hit 판정 끝났으므로 정보 초기화
	m_pGalbrena->ClearPendingHit();
	m_pGalbrena->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::HIT));
}



void CGalbrenaHit::Enter_Hit()
{
	// 0. Hit 정보 가져오기.
	const CCharacter::HIT_DESC* pDesc = m_pGalbrena->GetPendingHitDesc();

	// 1. 현재 레이어
	COLLISIONLAYER eLayer = static_cast<COLLISIONLAYER>(pDesc->iLayer);

	// 2. 스킬 판정.
	_bool IsSkill = (eLayer == COLLISIONLAYER::ENEMY_SKILL);

	// 3. 바로 회전.
	m_pGalbrena->Rotate_HitTarget(pDesc->pTransform);

	// 4. 땅 판정.
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);

	// 5. 애니메이션 선정.
	if (!m_States[LAND])
	{
		m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaHitType::BEHIT_FLY_FALL);
	}
	else
	{
		switch (eLayer)
		{
		case COLLISIONLAYER::ENEMY_ATTACK:
			m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaHitType::BEHIT_S_L);
			break;
		case COLLISIONLAYER::ENEMY_HARDATTACK:
			m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaHitType::BEHIT_B_L);
			break;
		case COLLISIONLAYER::ENEMY_SKILL:
			m_iCurrentAnimIdx = ENUM_CLASS(EGalbrenaHitType::BEHIT_FLY_FALL);
			break;
		}
	}
}

void CGalbrenaHit::Handle_Input()
{
    m_eDir = m_pGalbrena->Calculate_Direction(); // 방향 계산.
    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    
}

void CGalbrenaHit::Update_HitAnimation(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);
	EGalbrenaHitType eHitType = static_cast<EGalbrenaHitType>(m_iCurrentAnimIdx);
	if (eHitType == EGalbrenaHitType::BEHIT_FLY_FALL)
	{
		m_pGalbrena->Move_Fall(fTimeDelta, 0.1f); // 미세하게 떨어지게
	}
}

void CGalbrenaHit::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
    m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
}

void CGalbrenaHit::Check_StateTransition(_float fTimeDelta)
{
    EGalbrenaHitType eHitType = static_cast<EGalbrenaHitType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

	if (IsEscapePossible)
	{
		if (m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
		        m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
				return;
			}

			if (m_States[JUMP])
			{
				m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
				return;
			}
		}

		if (!m_States[LAND])
		{
			if (m_States[JUMP])
			{
				m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
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
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
				return;
			}

			if (m_States[JUMP])
			{
				m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
				return;
			}
			else
			{
				if (eHitType == EGalbrenaHitType::BEHIT_FLY_FALL)
				{
					m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STANDUP; // Idle 전용 일어나는 모션.
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
					return;
				}
				else
				{
					m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND2; // Idle 전용 일어나는 모션.
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
					return;
				}
			}
		}

		if (!m_States[LAND])
		{
			if (m_States[JUMP])
			{
				m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_SECOND_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
				return;
			}
			else
			{
				m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL));
				return;
			}
		}
	}

    
}


void CGalbrenaHit::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHitType::BEHIT_B_L), "Behit_B_L", 1.f, 40.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHitType::BEHIT_B_R), "Behit_B_R", 1.f, 40.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHitType::BEHIT_FLY_FALL), "Behit_Fly_Fall", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHitType::BEHIT_FLY_LOOP), "Behit_Fly_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHitType::BEHIT_FLY_START), "Behit_Fly_Start", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHitType::BEHIT_HOVER), "Behit_Hover", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHitType::BEHIT_PRESS), "Behit_Press", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHitType::BEHIT_PUSH_FALL), "Behit_Push_Fall", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHitType::BEHIT_PUSH_LOOP), "Behit_Push_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHitType::BEHIT_PUSH_START), "Behit_Push_Start", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHitType::BEHIT_S_L), "Behit_S_L", 1.f, 40.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHitType::BEHIT_S_R), "Behit_S_R", 1.f, 40.f);

}

void CGalbrenaHit::State_Reset()
{
    for (_uint i = 0; i < HITSTATE::END; ++i)
        m_States[i] = false;
}



CGalbrenaHit* CGalbrenaHit::Create(CCharacter* pOwner)
{
    CGalbrenaHit* pInstance = new CGalbrenaHit();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaHit");
        return nullptr;
    }

    return pInstance;
}

void CGalbrenaHit::Free()
{
    CHitState::Free();
}
