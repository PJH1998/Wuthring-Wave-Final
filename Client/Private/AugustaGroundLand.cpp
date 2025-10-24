#include "ClientPch.h"
#include "AugustaGroundLand.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaGroundLand::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaGroundLand::OnEnter()
{
    CGroundState::OnEnter();

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ELandType eLandType = context.m_eLandType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eLandType);

}

void CAugustaGroundLand::OnUpdate(_float fTimeDelta)
{
    
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    //  상태 제어.
    Update_LandAnimation(fTimeDelta);
    Check_StateTransition(fTimeDelta);
   

    
}

void CAugustaGroundLand::OnExit()
{
    CGroundState::OnExit();
}

void CAugustaGroundLand::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ELandType::LAND_LIGHT), "Land_Light", 1.f, 20.f);
    CState::Add_Animations(ENUM_CLASS(ELandType::LAND_HEAVY), "Land_Heavy", 1.f, 32.f);
    CState::Add_Animations(ENUM_CLASS(ELandType::LAND_ROLL), "Land_Roll",   1.f, 22.f);
    CState::Add_Animations(ENUM_CLASS(ELandType::LANDSLIDE_F), "Landslide_F", 1.f, 0.f);
}


void CAugustaGroundLand::Update_LandAnimation(_float fTimeDelta)
{
 
}

void CAugustaGroundLand::Check_StateTransition(_float fTimeDelta)
{
 
    ELandType eLandType = static_cast<ELandType>(m_iCurrentAnimIdx);
    _float3 vNormal = {}; // 벽타기 전환 용도 Normal

    _bool IsEscapePossible = CState::Is_EscapePossible();
    // Land 상태라는 것 자체가 땅에 닿았다는 의미.


    if (IsEscapePossible && m_pAugusta->Check_AnyInput(m_iMoveKey))
    {
        m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_BASEPOSE;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
        return;
    }

    if (m_IsAnimationEnd)
    {
        m_pAugusta->GetStateContextForWrite().m_eIdleType = EIdleType::STAND1_ACTION01;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
        return;
    }
  
}






CAugustaGroundLand* CAugustaGroundLand::Create(class CGameObject* pOwner)
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
