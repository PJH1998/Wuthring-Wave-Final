#include "ClientPch.h"
#include "GalbrenaGroundDodge.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"
#include "GameInstance.h"


HRESULT CGalbrenaGroundDodge::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pCharacter);
    ASSERT_CRASH(m_pGalbrena);

    Setup_Animations();

    return S_OK;
}

void CGalbrenaGroundDodge::OnEnter(void* pArg)
{
    // 상위 객체 수행 작업.
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
	EGalbrenaDodgeType EDodgeType = context.m_eDodgeType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eDodgeType);

    State_Reset();

	m_pGalbrena->Set_Gravity(true);

	

	m_pGalbrena->Resolve_PerfectDodge();

	m_pGalbrena->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::DODGE));
	m_pGalbrena->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));

	m_pGalbrena->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));
	m_pGalbrena->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::HIT));
}

void CGalbrenaGroundDodge::OnUpdate(_float fTimeDelta)
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

void CGalbrenaGroundDodge::OnExit()
{
    CGroundState::OnExit();
	m_pGalbrena->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::DODGE));
	m_pGalbrena->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
}

void CGalbrenaGroundDodge::Handle_Input()
{
    m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);

	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
	m_States[LOCKON] = m_pGalbrena->Is_LockOn();

}



void CGalbrenaGroundDodge::Update_SprintAnimation(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pGalbrena->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; // 거리 계산에 따른 Animation Scale 조절.

    // 1. 누른키에 따른 방향 계산
    m_eDir = m_pGalbrena->Calculate_Direction();

	// 2. Animation 실행.
    //CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta, m_fAnimationScale);
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta, 1.f); // Dodge는 제외.

	
    
}

void CGalbrenaGroundDodge::Check_StateTransition(_float fTimeDelta)
{
	EGalbrenaDodgeType eDodgeType = static_cast<EGalbrenaDodgeType>(m_iCurrentAnimIdx);

	_bool IsEscapePossible =CState::Is_EscapePossible();
  

	if (IsEscapePossible)
	{
		if (m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN)); // 상위, 하위 상태
				return;
			}
		}

		if (!m_States[LAND])
		{
			m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL)); // 상위, 하위 상태
			return;
		}

	}


	// 애니메이션 끝나면?
	if (m_IsAnimationEnd)
	{
		m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND1_ACTION01;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE)); // 상위, 하위 상태
		return;
	}

}

void CGalbrenaGroundDodge::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EGalbrenaDodgeType::MOVE_LIMIT_F), "Move_Limit_F", 1.5f, 20.f, 2.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaDodgeType::MOVE_LIMIT_B), "Move_Limit_B", 1.5f, 20.f, 2.f);
}

void CGalbrenaGroundDodge::State_Reset()
{
    for (_uint i = 0; i < DASHSTATE::END; ++i)
        m_States[i] = false;
}

CGalbrenaGroundDodge* CGalbrenaGroundDodge::Create(CCharacter* pOwner)
{
    CGalbrenaGroundDodge* pInstance = new CGalbrenaGroundDodge();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaGroundDodge");
    }

    return pInstance;
}

void CGalbrenaGroundDodge::Free()
{
    CGroundState::Free();
}
