#include "ClientPch.h"
#include "AugustaHit.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaHit::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CHitState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaHit::OnEnter()
{
    CHitState::OnEnter();

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EHitType eHitType = context.m_eHitType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eHitType);

    State_Reset();

    m_pAugusta->Set_Gravity(true);
}

void CAugustaHit::OnUpdate(_float fTimeDelta)
{
    CHitState::OnUpdate(fTimeDelta);

    // 0. 키입력 체크
    Handle_Input();

    // 1. 애니메이션 갱신
    Update_HitAnimation(fTimeDelta);

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    // 3. 전환 체크
    Check_StateTransition(fTimeDelta);

    // 상태 리셋;
    State_Reset();
}

void CAugustaHit::OnExit()
{
    CHitState::OnExit();
    m_pAugusta->Set_Gravity(false);
}

void CAugustaHit::Handle_Input()
{
    m_eDir = m_pAugusta->Calculate_Direction(); // 방향 계산.
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    
}

void CAugustaHit::Update_HitAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 플레이.
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
}

void CAugustaHit::Check_Physics(_float fTimeDelta)
{
    m_States[LAND] = m_pAugusta->Get_DistanceToGround(0.1f) <= 0.2f;
}

void CAugustaHit::Check_StateTransition(_float fTimeDelta)
{
    _float fDistanceToGround = m_pAugusta->Get_DistanceToGround(0.2f);
}


void CAugustaHit::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EHitType::BEHIT_B_L), "BeHit_B_L", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EHitType::BEHIT_B_R), "BeHit_B_R", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EHitType::BEHIT_FLY_FALL), "BeHit_B_L", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EHitType::BEHIT_FLY_LOOP), "BeHit_Fly_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EHitType::BEHIT_FLY_START), "BeHit_Fly_Start", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EHitType::BEHIT_HOVER), "BeHit_Hover", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EHitType::BEHIT_PRESS), "BeHit_Hover", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EHitType::BEHIT_PUSH_FALL), "BeHit_Push_Fall", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EHitType::BEHIT_PUSH_LOOP), "BeHit_Push_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EHitType::BEHIT_PUSH_START), "BeHit_Push_Start", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EHitType::BEHIT_S_L), "BeHit_S_L", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EHitType::BEHIT_S_R), "BeHit_S_R", 1.f, 0.f);

}

void CAugustaHit::State_Reset()
{
    for (_uint i = 0; i < HITSTATE::END; ++i)
        m_States[i] = false;
}



CAugustaHit* CAugustaHit::Create(class CGameObject* pOwner)
{
    CAugustaHit* pInstance = new CAugustaHit();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaHit");
        return nullptr;
    }

    return pInstance;
}

void CAugustaHit::Free()
{
    CHitState::Free();
}
