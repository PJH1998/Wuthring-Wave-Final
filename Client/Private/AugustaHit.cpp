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
    EAugustaHitType eHitType = context.m_eHitType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eHitType);

    State_Reset();

    // 4. 현재 때린 객체를 바라보게.? 임시로 Target
    m_pAugusta->Rotate_HitTarget();


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
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
}

void CAugustaHit::Check_Physics(_float fTimeDelta)
{
    m_States[LAND] = m_pAugusta->Get_DistanceFromGround(0.1f) <= 0.2f;
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
}

void CAugustaHit::Check_StateTransition(_float fTimeDelta)
{
    EAugustaHitType eHitType = static_cast<EAugustaHitType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();
    // 땅이 아닌 경우/
    if (m_States[!LAND])
    {
        if (IsEscapePossible)
        {
            if (m_States[JUMP])
            {
                if (eHitType == EAugustaHitType::BEHIT_FLY_FALL || eHitType == EAugustaHitType::BEHIT_B_L
                    || eHitType == EAugustaHitType::BEHIT_B_R || eHitType == EAugustaHitType::BEHIT_S_L
                    || eHitType == EAugustaHitType::BEHIT_S_R || eHitType == EAugustaHitType::BEHIT_FLY_START
                    || eHitType == EAugustaHitType::BEHIT_FLY_LOOP)
                {
                    m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_WALK_LF;
                    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
                    return;
                }
            }
        }

        // 애니메이션이 끝났음에도 땅이 아니라면?
        if (m_IsAnimationEnd)
        {
            if (eHitType == EAugustaHitType::BEHIT_FLY_START)
            {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaHitType::BEHIT_FLY_LOOP);
                return;
            }


            if (eHitType == EAugustaHitType::BEHIT_PUSH_START)
            {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaHitType::BEHIT_PUSH_LOOP);
                return;
            }
        }
    }

    // 땅인 경우.
    if (m_States[LAND])
    {
        // 탈출 가능할때 키입력 확인.
        if (IsEscapePossible)
        {
            if (m_States[JUMP])
            {
                if (eHitType == EAugustaHitType::BEHIT_FLY_FALL || eHitType == EAugustaHitType::BEHIT_B_L
                    || eHitType == EAugustaHitType::BEHIT_B_R || eHitType == EAugustaHitType::BEHIT_S_L
                    || eHitType == EAugustaHitType::BEHIT_S_R)
                {
                    m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_WALK_LF;
                    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
                    return;
                }
            }

            if (m_States[MOVE])
            {
                if (eHitType == EAugustaHitType::BEHIT_FLY_FALL || eHitType == EAugustaHitType::BEHIT_B_L
                    || eHitType == EAugustaHitType::BEHIT_B_R || eHitType == EAugustaHitType::BEHIT_S_L
                    || eHitType == EAugustaHitType::BEHIT_S_R)
                {
                    m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
                    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
                    return;
                }
            }
           
        }

        if (m_IsAnimationEnd)
        {
            if (eHitType == EAugustaHitType::BEHIT_FLY_START || eHitType == EAugustaHitType::BEHIT_FLY_LOOP)
            {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaHitType::BEHIT_FLY_FALL);
                return;
            }
            else 
            {
                m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
                return;
            }
            
        }
    }
    
}


void CAugustaHit::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_B_L), "Behit_B_L", 1.f, 40.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_B_R), "Behit_B_R", 1.f, 40.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_FLY_FALL), "Behit_Fly_Fall", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_FLY_LOOP), "Behit_Fly_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_FLY_START), "Behit_Fly_Start", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_HOVER), "Behit_Hover", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_PRESS), "Behit_Press", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_PUSH_FALL), "Behit_Push_Fall", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_PUSH_LOOP), "Behit_Push_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_PUSH_START), "Behit_Push_Start", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_S_L), "Behit_S_L", 1.f, 40.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaHitType::BEHIT_S_R), "Behit_S_R", 1.f, 40.f);

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
