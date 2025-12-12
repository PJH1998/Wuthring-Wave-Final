#include "ClientPch.h"
#include "RoverGroundDodge.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"
#include "GameInstance.h"


HRESULT CRoverGroundDodge::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pCharacter);
    ASSERT_CRASH(m_pRover);

    Setup_Animations();

    return S_OK;
}

void CRoverGroundDodge::OnEnter(void* pArg)
{
    // 상위 객체 수행 작업.
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
	ERoverDodgeType EDodgeType = context.m_eDodgeType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eDodgeType);

    State_Reset();

	// 4. 락온 중이였다면? => 한번만 입력방향에 따른 회전.
	//if (m_pRover->Is_LockOn())
	//{
	//	// 5. 누른 키에 따른 입력 방향 받아오기.
	//	m_eDir = m_pRover->Calculate_Direction();
	//	_vector vMoveDir = m_pRover->Calculate_Move_Direction(m_eDir);
	//	m_pRover->Rotate_Direction(vMoveDir);
	//}

	m_pRover->Set_Gravity(true);

	// 6. 플레이어 상태 제어 => 무적 추가 및 Hit 상태 제거
	// 회피 가능 창을 닫습니다. => Timer 실행 방지.

	m_pRover->Resolve_PerfectDodge();

	m_pRover->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE)); 
	m_pRover->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::HIT));

	m_pRover->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::DODGE));
	m_pRover->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));

	// 7. Hit Stop
	
}

void CRoverGroundDodge::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키 입력
    Handle_Input();

    // 1. 애니메이션 실행
    Update_DodgeAnimation(fTimeDelta);
    
    // 2. 상태 제어.
    Check_StateTransition(fTimeDelta);

    // 3. 상태 초기화.
    State_Reset();
}

void CRoverGroundDodge::OnExit()
{
    CGroundState::OnExit();
	m_pRover->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::DODGE));
	m_pRover->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
}

void CRoverGroundDodge::Handle_Input()
{
    m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);

	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
	m_States[LOCKON] = m_pRover->Is_LockOn();

}



void CRoverGroundDodge::Update_DodgeAnimation(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pRover->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; // 거리 계산에 따른 Animation Scale 조절.

	// 2. Animation 실행.
    CCharacterState::Play_Animation(m_pRover, fTimeDelta, 1.f);

	
    
}

void CRoverGroundDodge::Check_StateTransition(_float fTimeDelta)
{
	ERoverDodgeType eDodgeType = static_cast<ERoverDodgeType>(m_iCurrentAnimIdx);

	_bool IsEscapePossible =CState::Is_EscapePossible();
  

	if (IsEscapePossible)
	{
		if (m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN)); // 상위, 하위 상태
				return;
			}
		}

		if (!m_States[LAND])
		{
			m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL)); // 상위, 하위 상태
			return;
		}

	}


	// 애니메이션 끝나면?
	if (m_IsAnimationEnd)
	{
		m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1_ACTION01;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE)); // 상위, 하위 상태
		return;
	}

}

void CRoverGroundDodge::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ERoverDodgeType::MOVE_LIMIT_F), "Move_Limit_F", 1.5f, 20.f, 2.f);
    CState::Add_Animations(ENUM_CLASS(ERoverDodgeType::MOVE_LIMIT_B), "Move_Limit_B", 1.5f, 20.f, 2.f);
}

void CRoverGroundDodge::State_Reset()
{
    for (_uint i = 0; i < DASHSTATE::END; ++i)
        m_States[i] = false;
}

CRoverGroundDodge* CRoverGroundDodge::Create(CCharacter* pOwner)
{
    CRoverGroundDodge* pInstance = new CRoverGroundDodge();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverGroundDodge");
    }

    return pInstance;
}

void CRoverGroundDodge::Free()
{
    CGroundState::Free();
}
