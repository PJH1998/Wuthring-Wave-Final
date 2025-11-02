#include "ClientPch.h"
#include "RoverGroundLand.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"

HRESULT CRoverGroundLand::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pOwner);
    ASSERT_CRASH(m_pRover);

    Setup_Animations();
    return S_OK;
}



void CRoverGroundLand::OnEnter()
{
    CGroundState::OnEnter();

    // 1. 복사본 context 받아오기.
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverLandType eLandType = context.m_eLandType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eLandType);

    // 4. 상태 초기화
    State_Reset();

}

void CRoverGroundLand::OnUpdate(_float fTimeDelta)
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

void CRoverGroundLand::OnExit()
{
    CGroundState::OnExit();
}



void CRoverGroundLand::Handle_Input()
{
    m_States[RUN] = m_pRover->Check_AnyInput(m_iMoveKey);
}

void CRoverGroundLand::Update_LandAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pRover, fTimeDelta);
}

void CRoverGroundLand::Check_StateTransition(_float fTimeDelta)
{
    _bool IsEscapePossible = CState::Is_EscapePossible();
    if (IsEscapePossible && m_States[RUN])
    {
        m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
        m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
        return;
    }

    if (m_IsAnimationEnd)
    {
        m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1;
        m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
        return;
    }
}


void CRoverGroundLand::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERoverLandType::LAND_LIGHT), "Land_Light", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(ERoverLandType::LAND_HEAVY), "Land_Heavy", 1.f, 32.f);
    CState::Add_Animations(ENUM_CLASS(ERoverLandType::LAND_ROLL), "Land_Roll", 1.f, 22.f);
    CState::Add_Animations(ENUM_CLASS(ERoverLandType::LANDSLIDE_F), "Landslide_F", 1.f, 0.f);
}

void CRoverGroundLand::State_Reset()
{
    for (_uint i = 0; i < LANDSTATE::END; ++i)
        m_States[i] = false;
}



CRoverGroundLand* CRoverGroundLand::Create(class CGameObject* pOwner)
{
    CRoverGroundLand* pInstance = new CRoverGroundLand();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverGroundLand");
        return nullptr;
    }

    return pInstance;
}

void CRoverGroundLand::Free()
{
    CGroundState::Free();
}
