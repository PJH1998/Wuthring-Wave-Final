#include "ClientPch.h"
#include "GalbrenaGroundLand.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"

HRESULT CGalbrenaGroundLand::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pCharacter);
    ASSERT_CRASH(m_pGalbrena);

    Setup_Animations();
    return S_OK;
}



void CGalbrenaGroundLand::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaLandType eLandType = context.m_eLandType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eLandType);

    // 4. 상태 초기화
    State_Reset();

}

void CGalbrenaGroundLand::OnUpdate(_float fTimeDelta)
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

void CGalbrenaGroundLand::OnExit()
{
    CGroundState::OnExit();
}



void CGalbrenaGroundLand::Handle_Input()
{
	m_States[HIT] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT));
	// Hit면 모든 상태 제거
	if (m_States[HIT])
		return;

    m_States[RUN] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
}

void CGalbrenaGroundLand::Update_LandAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);
}

void CGalbrenaGroundLand::Check_StateTransition(_float fTimeDelta)
{
    _bool IsEscapePossible = CState::Is_EscapePossible();

	// Hit는 무조건 전환
	if (m_States[HIT])
	{
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EGalbrenaHitState::HIT));
		return;
	}

    if (IsEscapePossible && m_States[RUN])
    {
		if (m_States[LAND])
		{
			m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
			return;
		}
		else
		{
			m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL));
			return;
		}
        
        
    }

    if (m_IsAnimationEnd)
    {
        m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND1_ACTION01;
        m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
        return;
    }
}


void CGalbrenaGroundLand::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaLandType::LAND_LIGHT), "Land_Light", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaLandType::LAND_HEAVY), "Land_Heavy", 1.5f, 32.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaLandType::LAND_ROLL), "Land_Roll", 1.f, 22.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaLandType::LANDSLIDE_F), "Landslide_F", 1.f, 0.f);
}

void CGalbrenaGroundLand::State_Reset()
{
    for (_uint i = 0; i < LANDSTATE::END; ++i)
        m_States[i] = false;
}



CGalbrenaGroundLand* CGalbrenaGroundLand::Create(CCharacter* pOwner)
{
    CGalbrenaGroundLand* pInstance = new CGalbrenaGroundLand();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaGroundLand");
        return nullptr;
    }

    return pInstance;
}

void CGalbrenaGroundLand::Free()
{
    CGroundState::Free();
}
