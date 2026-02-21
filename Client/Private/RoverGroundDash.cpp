#include "ClientPch.h"
#include "RoverGroundDash.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"


HRESULT CRoverGroundDash::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pCharacter);
    ASSERT_CRASH(m_pRover);

    Setup_Animations();

    return S_OK;
}

void CRoverGroundDash::OnEnter(void* pArg)
{
    // 상위 객체 수행 작업.
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverDashType EDashType = context.m_eDashType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eDashType);

    State_Reset();
	m_pRover->Set_Gravity(true);
}

void CRoverGroundDash::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키 입력
    Handle_Input();

    // 1. 애니메이션 실행
	Update_DashAnimation(fTimeDelta);
    
    // 2. 상태 제어.
    Check_StateTransition(fTimeDelta);

    // 3. 상태 초기화.
    State_Reset();
}

void CRoverGroundDash::OnExit()
{
    CGroundState::OnExit();
}

void CRoverGroundDash::Handle_Input()
{

	// Dash 키입력 체크.
	m_States[DASH] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));
	m_States[HIT] = m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT)); // HIT 상태인가?
	m_States[DODGE] = m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));
	//m_States[DODGEABLE] = m_pRover->HasAbilityFlag(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));
	//m_States[DODGE] = m_States[DODGEABLE] && m_States[DASH]; // Dodge 가능하면서 Dash 키 누르면?

	if (m_States[DODGE] || m_States[HIT]) // 모든 조건 상위 조건
		return;

    m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
	//m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal, 0.2f);;
}





void CRoverGroundDash::Update_DashAnimation(_float fTimeDelta)
{
    // 1. 누른키에 따른 방향 계산
    m_eDir = m_pRover->Calculate_Direction();

    // 2. LockOn 상태일때는 현재 방향에서 누른 방향을 바라보게 수정.
    if (m_pRover->Is_LockOn())
    {
        _vector vMoveDir = m_pRover->Calculate_Move_Direction(m_eDir);
        m_pRover->Rotate_Direction(vMoveDir);
    }

    CCharacterState::Play_Animation(m_pRover, fTimeDelta);
}

void CRoverGroundDash::Check_StateTransition(_float fTimeDelta)
{
    ERoverDashType eDashType = static_cast<ERoverDashType>(m_iCurrentAnimIdx);
	_bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. 우선순위
	if (m_States[DODGE])
	{
		const CCharacter::HIT_DESC* pDesc = m_pRover->GetPendingHitDesc();
		if (nullptr == pDesc)
			return;

		/*if (pDesc->IsBack)
			m_pRover->GetStateContextForWrite().m_eDodgeType = ERoverDodgeType::MOVE_LIMIT_F;
		else
			m_pRover->GetStateContextForWrite().m_eDodgeType = ERoverDodgeType::MOVE_LIMIT_B;*/


		m_pRover->GetStateContextForWrite().m_eDodgeType = ERoverDodgeType::MOVE_LIMIT_F;

		// Dodge 이전에 누른 방향으로 회전.
		_vector vMoveDir = m_pRover->Calculate_Move_Direction(m_eDir);
		m_pRover->Rotate_Direction(vMoveDir);


		m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::DODGE)); // 상위, 하위 상태
		return;
	}

	// 2.
	if (m_States[HIT])
	{
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(ERoverHitState::HIT));
		return;
	}

	if (!m_States[LAND])
	{
		m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL)); // 상위, 하위 상태
		return;
	}

    // 애니메이션 끝나면?
    if (m_IsAnimationEnd)
    {
        m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1;
        m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE)); // 상위, 하위 상태
        return;
    }
  

	if (IsEscapePossible)
	{
		if (eDashType == ERoverDashType::MOVE_F)
		{
			if (m_States[JUMP])
			{
				m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_WALK_LF;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP)); // 상위, 하위 상태
				return;
			}

			if (m_States[MOVE])
			{
				m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN)); // 상위, 하위 상태
				return;
			}
		}
	}
}

void CRoverGroundDash::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERoverDashType::MOVE_F), "Move_F", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(ERoverDashType::MOVE_B), "Move_B", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(ERoverDashType::MOVE_LIMIT_B), "Move_Limit_B", 30.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ERoverDashType::MOVE_LIMIT_F), "Move_Limit_F", 30.f, 0.f);
}

void CRoverGroundDash::State_Reset()
{
    for (_uint i = 0; i < DASHSTATE::END; ++i)
        m_States[i] = false;
}

CRoverGroundDash* CRoverGroundDash::Create(CCharacter* pOwner)
{
    CRoverGroundDash* pInstance = new CRoverGroundDash();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverGroundDash");
    }

    return pInstance;
}

void CRoverGroundDash::Free()
{
    CGroundState::Free();
}
