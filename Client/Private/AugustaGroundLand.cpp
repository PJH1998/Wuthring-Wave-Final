#include "ClientPch.h"
#include "AugustaGroundLand.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaGroundLand::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pCharacter);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaGroundLand::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaLandType eLandType = context.m_eLandType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eLandType);

    // 4. 상태 초기화
    State_Reset();

	// 5. 중력 켰다.
	m_pAugusta->Set_Gravity(true);

}

void CAugustaGroundLand::OnUpdate(_float fTimeDelta)
{
    
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키입력 제어
    Handle_Input();

    // 1. 애니메이션 제어.
    Update_LandAnimation(fTimeDelta);

    // 2. 상태 제어.
    Check_StateTransition(fTimeDelta);
   
    // 3. 상태 초기화
    State_Reset();
}

void CAugustaGroundLand::OnExit()
{
    CGroundState::OnExit();
	m_pAugusta->Set_Gravity(true);
}



void CAugustaGroundLand::Handle_Input()
{
	// 이벤트 상태 => 
	m_States[LANDSLIDE] = m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::LANDSLIDE));
	m_States[HIT] = m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT)); // HIT 상태인가?
	// Hit면 모든 상태 제거
	if (m_States[HIT])
		return;

	m_States[RUN] = m_pAugusta->Check_AnyInput(m_iMoveKey);
}

void CAugustaGroundLand::Update_LandAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
}

void CAugustaGroundLand::Check_StateTransition(_float fTimeDelta)
{
    _bool IsEscapePossible = CState::Is_EscapePossible();

	//// 0. LandSlide 이벤트 시. 
	//if (m_States[LANDSLIDE])
	//{
	//	m_pAugusta->GetStateContextForWrite().m_eLandSlideType = EAugustaLandSlideType::LANDSLIDE_SPRINT_START;
	//	m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LANDSLIDE));
	//	return;
	//}
	

	// 1. Hit는 무조건 전환
	if (m_States[HIT])
	{
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EAugustaHitState::HIT));
		return;
	}

    if (IsEscapePossible)
    {
		if (m_States[RUN])
		{
			m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
			return;
		}
    }

    if (m_IsAnimationEnd)
    {
        m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION02;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
        return;
    }
}


void CAugustaGroundLand::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaLandType::LAND_LIGHT), "Land_Light", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaLandType::LAND_HEAVY), "Land_Heavy", 1.f, 22.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaLandType::LAND_ROLL), "Land_Roll", 1.f, 22.f);
}

void CAugustaGroundLand::State_Reset()
{
    for (_uint i = 0; i < LANDSTATE::END; ++i)
        m_States[i] = false;
}



CAugustaGroundLand* CAugustaGroundLand::Create(CCharacter* pOwner)
{
    CAugustaGroundLand* pInstance = new CAugustaGroundLand();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundLand");
        return nullptr;
    }

    return pInstance;
}

void CAugustaGroundLand::Free()
{
    CGroundState::Free();
}
