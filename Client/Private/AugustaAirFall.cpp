#include "ClientPch.h"
#include "AugustaAirFall.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaAirFall::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaAirFall::OnEnter()
{
    CAirState::OnEnter();

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EFallType eFallType = context.m_eFallType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eFallType);

}

void CAugustaAirFall::OnUpdate(_float fTimeDelta)
{
    CAirState::OnUpdate(fTimeDelta);

    // 0. 애니메이션 플레이.
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    Update_FallAnimation(fTimeDelta);
    Check_StateTransition(fTimeDelta);
}

void CAugustaAirFall::OnExit()
{
    CAirState::OnExit();
}

void CAugustaAirFall::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EFallType::FALL_LOOP), "Fall_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EFallType::FALL_LOOP_FAST), "Fall_Loop_Fast", 1.f, 0.f);
}


void CAugustaAirFall::Update_FallAnimation(_float fTimeDelta)
{
    // 조금 더 가속 주기?
    m_pAugusta->Move_Fall(fTimeDelta, 5.f);

    // 1. 조작키에 따른 이동?
    m_eDir = m_pAugusta->Calculate_Direction(); // 여기서 이미 키체크를 완료하고 방향 계산.
    if (m_pAugusta->Check_AnyInput(m_iMoveKey))
    {
        m_pAugusta->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, 1.f);
        return;
    }
}

void CAugustaAirFall::Check_StateTransition(_float fTimeDelta)
{
    EFallType eFallType = static_cast<EFallType>(m_iCurrentAnimIdx);

    _float3 vNormal = {};
    _bool IsLand = m_pAugusta->Is_Land(&vNormal);

    if (IsLand)
    {
        m_pAugusta->GetStateContextForWrite().m_eLandType = ELandType::LAND_LIGHT;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
        return;
    }

}






CAugustaAirFall* CAugustaAirFall::Create(class CGameObject* pOwner)
{
    CAugustaAirFall* pInstance = new CAugustaAirFall();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaAirFall");
        return nullptr;
    }

    return pInstance;
}

void CAugustaAirFall::Free()
{
    CAirState::Free();
}
