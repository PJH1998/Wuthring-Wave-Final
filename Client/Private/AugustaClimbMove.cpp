#include "ClientPch.h"
#include "AugustaClimbMove.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaClimbMove::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CClimbState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaClimbMove::OnEnter(void* pArg)
{
    CClimbState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaClimbMoveType eClimbMoveType = context.m_eClimbMoveType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eClimbMoveType);

    // 4. 벽 Normal 저장
    m_pAugusta->Check_ClimbableWall(&m_vWallNormal);

    // 0 . 중력 끄기.
    m_pAugusta->Set_Gravity(false);

    // 5. 상태 초기화.
    State_Reset();
}

void CAugustaClimbMove::OnUpdate(_float fTimeDelta)
{
    CClimbState::OnUpdate(fTimeDelta);

    // 0. 입력 체크
    Handle_Input();

    // 1. 애니메이션 플레이.
    Update_ClimbAnimation(fTimeDelta);
    
    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    // 3. 상태 전환 체크
    Check_StateTransition(fTimeDelta);

    // 4. 상태 초기화
    State_Reset();
}

void CAugustaClimbMove::OnExit()
{
    CClimbState::OnExit();
    m_IsSecondStep = false;

    //m_pAugusta->Set_Gravity(true);
}

void CAugustaClimbMove::Handle_Input()
{
    EAugustaClimbMoveType eClimbMoveType = static_cast<EAugustaClimbMoveType>(m_iCurrentAnimIdx);

    // 1. WASD 입력에 따라 등반 방향 결정
    m_States[U] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
    m_States[D] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
    m_States[R] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::A)); // 방향 반대.
    m_States[L] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::D)); // 방향 반대
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);

    if (m_States[MOVE]) // 입력이 있었다면?
        m_States[IS_CLIMBED] = true;

    m_States[BACKJUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
}



// Climing Update
void CAugustaClimbMove::Update_ClimbAnimation(_float fTimeDelta)
{
    if (m_States[IS_CLIMBED])
    {
        CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
    }
        
}

void CAugustaClimbMove::Check_Physics(_float fTimeDelta)
{
    m_States[WALL] = m_pAugusta->Check_ClimbableWall(&m_vWallNormal);
	m_States[LAND] = m_pAugusta->Is_Land();

    // 머리에서 쐈는데 안 맞으면?
    m_States[ONTOP] = m_States[WALL] && (!m_pAugusta->Check_ClimbableWall_Above(1.f, &m_vHeadWallNormal));
}


void CAugustaClimbMove::Check_StateTransition(_float fTimeDelta)
{

    EAugustaClimbMoveType eClimbMoveType = static_cast<EAugustaClimbMoveType>(m_iCurrentAnimIdx);

    // 0. 땅에 닿았다면?
    if (m_States[LAND])
    {
        // 아래 내려가면서 아래 상태라면?
        if (m_vLandNormal.y > 1.f && (eClimbMoveType == EAugustaClimbMoveType::CLIMB_D_1 || eClimbMoveType == EAugustaClimbMoveType::CLIMB_D_2))
        {
            m_pAugusta->GetStateContextForWrite().m_eLandType = EAugustaLandType::LAND_LIGHT;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
            return;
        }
    }

    // 1. 벽에서 떨어졌는지 체크
    if (!m_States[WALL])
    {
        m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL));
        return;
    }

    // 2. 위로 올라가는 중 벽 상단 도달 체크
    if (eClimbMoveType == EAugustaClimbMoveType::CLIMB_U_1 || eClimbMoveType == EAugustaClimbMoveType::CLIMB_U_2)
    {
        if (m_fTrackPosition > 30.f && m_States[ONTOP])
        {
            m_pAugusta->GetStateContextForWrite().m_eClimbExitType = EAugustaClimbExitType::CLIMB_ONTOP; // 점프.
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_EXIT));
        }
    }

    // 2. Space 입력 → 벽에서 점프 (Exit)
    if (m_States[BACKJUMP])
    {
        m_pAugusta->GetStateContextForWrite().m_eClimbExitType = EAugustaClimbExitType::CLIMB_MOVE; // 점프.
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_EXIT));
        return;
    }

    // 입력이 있다면?
    // 2. 두번째 재생 애니메이션이 아니고 애니메이션 실행이 종료되었다면? => 키입력이 있다면.
    if (m_IsAnimationEnd && m_States[MOVE])
    {
        if (!m_IsSecondStep)
        {
            // 3. 두번째 재생 애니메이션이 아니라면..
            if (m_States[U]) {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaClimbMoveType::CLIMB_U_2);
            }
            else if (m_States[D]) {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaClimbMoveType::CLIMB_D_2);
            }
            else if (m_States[L]) {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaClimbMoveType::CLIMB_L_2);
            }
            else if (m_States[R]) {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaClimbMoveType::CLIMB_R_2);
            }
            m_IsSecondStep = true;
            return;
        }
        if (m_IsSecondStep)
        {
            // 3. 두번째 재생 애니메이션이라면..
            if (m_States[U]) {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaClimbMoveType::CLIMB_U_1);
            }
            else if (m_States[D]) {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaClimbMoveType::CLIMB_D_1);
            }
            else if (m_States[L]) {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaClimbMoveType::CLIMB_L_1);
            }
            else if (m_States[R]) {
                // 오른쪽으로 등반
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaClimbMoveType::CLIMB_R_1);
            }

            m_IsSecondStep = false;
            return;
        }
    }

    //if (!m_States[MOVE] && m_IsAnimationEnd)
    //{
    //    // 교체할 Exit Type
    //    EAugustaClimbExitType eClimbExitType = { EAugustaClimbExitType::CLIMB_D1_STOP };
    //    
    //    // 현재 MoveType
    //    switch (eClimbMoveType)
    //    {
    //    case EAugustaClimbMoveType::CLIMB_D_1:
    //        eClimbExitType = EAugustaClimbExitType::CLIMB_D1_STOP;
    //        break;
    //    case EAugustaClimbMoveType::CLIMB_D_2:
    //        eClimbExitType = EAugustaClimbExitType::CLIMB_D2_STOP;
    //        break;
    //    case EAugustaClimbMoveType::CLIMB_L_1:
    //        eClimbExitType = EAugustaClimbExitType::CLIMB_L1_STOP;
    //        break;
    //    case EAugustaClimbMoveType::CLIMB_L_2:
    //        eClimbExitType = EAugustaClimbExitType::CLIMB_L2_STOP;
    //        break;
    //    case EAugustaClimbMoveType::CLIMB_R_1:
    //        eClimbExitType = EAugustaClimbExitType::CLIMB_R1_STOP;
    //        break;
    //    case EAugustaClimbMoveType::CLIMB_R_2:
    //        eClimbExitType = EAugustaClimbExitType::CLIMB_R2_STOP;
    //        break;
    //    case EAugustaClimbMoveType::CLIMB_U_1:
    //        eClimbExitType = EAugustaClimbExitType::CLIMB_U1_STOP;
    //        break;
    //    case EAugustaClimbMoveType::CLIMB_U_2:
    //        eClimbExitType = EAugustaClimbExitType::CLIMB_U2_STOP;
    //        break;
    //    }

    //    m_pAugusta->GetStateContextForWrite().m_eClimbExitType = eClimbExitType;
    //    m_pAugusta->GetStateContextForWrite().m_IsClimbSecondStep = m_IsSecondStep; // 두번째 상태였으면?
    //    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_EXIT));
    //    return;
    //}
}

// 벽타기 조절.
void CAugustaClimbMove::Adjust_To_Wall(_float fTimeDelta)
{
    
}

void CAugustaClimbMove::Setup_Animations()
{
    // 올라가는 것부터?
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_D_1), "Climb_D_1",   2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_D_2), "Climb_D_2",   2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_DL_1), "Climb_DL_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_DL_2), "Climb_DL_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_DR_1), "Climb_DR_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_DR_2), "Climb_DR_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_L_1), "Climb_L_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_L_2), "Climb_L_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_R_1), "Climb_R_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_R_2), "Climb_R_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_U_1), "Climb_U_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_U_2), "Climb_U_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_UL_1), "Climb_UL_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_UL_2), "Climb_UL_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_UR_1), "Climb_UR_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbMoveType::CLIMB_UR_2), "Climb_UR_2", 2.f, 0.f);
}

void CAugustaClimbMove::State_Reset()
{
    for (_uint i = 0; i < CLIMBSTATE::END; ++i)
        m_States[i] = false;
}





CAugustaClimbMove* CAugustaClimbMove::Create(class CGameObject* pOwner)
{
    CAugustaClimbMove* pInstance = new CAugustaClimbMove();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaClimbMove");
        return nullptr;
    }

    return pInstance;
}

void CAugustaClimbMove::Free()
{
    CClimbState::Free();
}
