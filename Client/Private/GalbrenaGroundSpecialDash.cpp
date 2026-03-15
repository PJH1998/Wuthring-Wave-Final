#include "ClientPch.h"
#include "GalbrenaGroundSpecialDash.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"


HRESULT CGalbrenaGroundSpecialDash::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pCharacter);
    ASSERT_CRASH(m_pGalbrena);

    Setup_Animations();

    return S_OK;
}

void CGalbrenaGroundSpecialDash::OnEnter(void* pArg)
{
    // 상위 객체 수행 작업.
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
	EGalbrenaSpecialDashType eSpecialDashType = context.m_eSpecialDashType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eSpecialDashType);

    State_Reset();
	m_pGalbrena->Set_Gravity(true);
}

void CGalbrenaGroundSpecialDash::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키 입력
    Handle_Input();

    // 1. 애니메이션 실행
    Update_SprintAnimation(fTimeDelta);
    
    // 2. 상태 제어.
    Check_StateTransition(fTimeDelta);

    // 3. 상태 초기화.
    State_Reset();
}

void CGalbrenaGroundSpecialDash::OnExit()
{
    CGroundState::OnExit();
}

void CGalbrenaGroundSpecialDash::Handle_Input()
{

	// Dash 키입력 체크.
	m_States[DASH] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));
	m_States[HIT] = Hit_Judge();

	m_States[DODGE] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));
	//m_States[DODGEABLE] = m_pGalbrena->HasAbilityFlag(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));
	//m_States[DODGE] = m_States[DODGEABLE] && m_States[DASH]; // Dodge 가능하면서 Dash 키 누르면?

	if (m_States[DODGE] || m_States[HIT]) // 모든 조건 상위 조건
		return;

    m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
	//m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal, 0.2f);;
}





void CGalbrenaGroundSpecialDash::Update_SprintAnimation(_float fTimeDelta)
{
    // 1. 누른키에 따른 방향 계산
    m_eDir = m_pGalbrena->Calculate_Direction();

	_vector vMoveDir = m_pGalbrena->Calculate_Move_Direction(m_eDir);
	m_pGalbrena->Rotate_Direction(vMoveDir);

    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);
}

void CGalbrenaGroundSpecialDash::Check_StateTransition(_float fTimeDelta)
{
    EGalbrenaSpecialDashType eSpecialDashType = static_cast<EGalbrenaSpecialDashType>(m_iCurrentAnimIdx);
	_bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. 우선순위
	if (m_States[DODGE])
	{
		// Dodge 이전에 누른 방향으로 회전.
		_vector vMoveDir = m_pGalbrena->Calculate_Move_Direction(m_eDir);
		m_pGalbrena->Rotate_Direction(vMoveDir);

		m_pGalbrena->GetStateContextForWrite().m_eDodgeType = EGalbrenaDodgeType::MOVE_LIMIT_F;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::DODGE)); // 상위, 하위 상태
		return;
	}

	// 2. Hit 단위.
	if (m_States[HIT])
	{
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EGalbrenaHitState::HIT));
		return;
	}
  
  

	if (IsEscapePossible)
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

	// 애니메이션 끝나면?
	if (m_IsAnimationEnd)
	{
		if (!m_States[LAND])
		{
			m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL)); // 상위, 하위 상태
			return;
		}
		if (m_States[LAND])
		{
			m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STANDCHANGE;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE)); // 상위, 하위 상태
			return;
		}
	}
}

void CGalbrenaGroundSpecialDash::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaSpecialDashType::ATTACK_CHARGE), "Attack_charge", 1.5f, 20.f);
}

void CGalbrenaGroundSpecialDash::State_Reset()
{
    for (_uint i = 0; i < DASHSTATE::END; ++i)
        m_States[i] = false;
}

_bool CGalbrenaGroundSpecialDash::Hit_Judge()
{
	_bool IsHit = false;
	const HIT_DESC* pDesc = m_pGalbrena->GetPendingHitDesc();

	if (nullptr == pDesc)
		return false;

	// Hit 상태이면서 Enemy Skill을 받았을때만 캔슬하고 Hit로
	if (!m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT)))
		return false;


	COLLISIONLAYER eLayer = static_cast<COLLISIONLAYER>(m_pGalbrena->GetPendingHitDesc()->iLayer);
	if (eLayer == COLLISIONLAYER::ENEMY_SKILL)
		IsHit = true;

	return IsHit;
}

CGalbrenaGroundSpecialDash* CGalbrenaGroundSpecialDash::Create(CCharacter* pOwner)
{
    CGalbrenaGroundSpecialDash* pInstance = new CGalbrenaGroundSpecialDash();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaGroundSpecialDash");
    }

    return pInstance;
}

void CGalbrenaGroundSpecialDash::Free()
{
    CGroundState::Free();
}
