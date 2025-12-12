#include "ClientPch.h"
#include "GalbrenaGroundDash.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"


HRESULT CGalbrenaGroundDash::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pCharacter);
    ASSERT_CRASH(m_pGalbrena);

    Setup_Animations();

    return S_OK;
}

void CGalbrenaGroundDash::OnEnter(void* pArg)
{
    // 상위 객체 수행 작업.
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaDashType EDashType = context.m_eDashType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eDashType);

    State_Reset();
	m_pGalbrena->Set_Gravity(true);
}

void CGalbrenaGroundDash::OnUpdate(_float fTimeDelta)
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

void CGalbrenaGroundDash::OnExit()
{
    CGroundState::OnExit();
}

void CGalbrenaGroundDash::Handle_Input()
{

	// Dash 키입력 체크.
	m_States[DASH] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));
	m_States[HIT] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT)); // HIT 상태인가?

	m_States[DODGE] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));
	//m_States[DODGEABLE] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));
	//m_States[DODGE] = m_States[DODGEABLE] && m_States[DASH]; // Dodge 가능하면서 Dash 키 누르면?

	if (m_States[DODGE] || m_States[HIT]) // 모든 조건 상위 조건
		return;

    m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
	//m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal, 0.2f);;
}





void CGalbrenaGroundDash::Update_DodgeAnimation(_float fTimeDelta)
{
    // 1. 누른키에 따른 방향 계산
    m_eDir = m_pGalbrena->Calculate_Direction();

    // 2. LockOn 상태일때는 현재 방향에서 누른 방향을 바라보게 수정.
    if (m_pGalbrena->Is_LockOn())
    {
        _vector vMoveDir = m_pGalbrena->Calculate_Move_Direction(m_eDir);
        m_pGalbrena->Rotate_Direction(vMoveDir);
    }
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);
}

void CGalbrenaGroundDash::Check_StateTransition(_float fTimeDelta)
{
    EGalbrenaDashType eDashType = static_cast<EGalbrenaDashType>(m_iCurrentAnimIdx);
	_bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. 우선순위
	if (m_States[DODGE])
	{
		const CCharacter::HIT_DESC* pDesc = m_pGalbrena->GetPendingHitDesc();
		if (nullptr == pDesc)
			return;

		/*if (pDesc->IsBack)
			m_pGalbrena->GetStateContextForWrite().m_eDodgeType = EGalbrenaDodgeType::MOVE_LIMIT_F;
		else
			m_pGalbrena->GetStateContextForWrite().m_eDodgeType = EGalbrenaDodgeType::MOVE_LIMIT_B;*/

		m_pGalbrena->GetStateContextForWrite().m_eDodgeType = EGalbrenaDodgeType::MOVE_LIMIT_F;

		// Dodge 이전에 누른 방향으로 회전.
		_vector vMoveDir = m_pGalbrena->Calculate_Move_Direction(m_eDir);
		m_pGalbrena->Rotate_Direction(vMoveDir);

		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::DODGE)); // 상위, 하위 상태
		return;
	}

	// 2. Hit 단위.
	if (m_States[HIT])
	{
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EGalbrenaHitState::HIT));
		return;
	}

	if (!m_States[LAND])
	{
		m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL)); // 상위, 하위 상태
		return;
	}

    // 애니메이션 끝나면?
    if (m_IsAnimationEnd)
    {
        m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND1_ACTION02;
        m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE)); // 상위, 하위 상태
        return;
    }
  

	if (IsEscapePossible)
	{
		if (eDashType == EGalbrenaDashType::MOVE_F)
		{
			if (m_States[JUMP])
			{
				m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_WALK_LF;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP)); // 상위, 하위 상태
				return;
			}

			if (m_States[MOVE])
			{
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN)); // 상위, 하위 상태
				return;
			}
		}
	}
}

void CGalbrenaGroundDash::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaDashType::MOVE_F), "Move_F", 1.f, 20.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaDashType::MOVE_B), "Move_B", 1.f, 20.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaDashType::MOVE_LIMIT_B), "Move_Limit_B", 30.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaDashType::MOVE_LIMIT_F), "Move_Limit_F", 30.f, 0.f);
}

void CGalbrenaGroundDash::State_Reset()
{
    for (_uint i = 0; i < DASHSTATE::END; ++i)
        m_States[i] = false;
}

CGalbrenaGroundDash* CGalbrenaGroundDash::Create(CCharacter* pOwner)
{
    CGalbrenaGroundDash* pInstance = new CGalbrenaGroundDash();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaGroundDash");
    }

    return pInstance;
}

void CGalbrenaGroundDash::Free()
{
    CGroundState::Free();
}
