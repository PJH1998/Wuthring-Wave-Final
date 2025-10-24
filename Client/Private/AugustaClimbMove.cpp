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



void CAugustaClimbMove::OnEnter()
{
    CClimbState::OnEnter();

    // 0 .
    m_pAugusta->Set_Gravity(false);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EClimbMoveType eClimbMoveType = context.m_eClimbMoveType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eClimbMoveType);

    // 벽 Normal 저장
    m_pAugusta->Check_ClimbableWall(&m_vWallNormal);

    
}

void CAugustaClimbMove::OnUpdate(_float fTimeDelta)
{
    CClimbState::OnUpdate(fTimeDelta);

    m_IsClimbed = false;

    // 0. 애니메이션 플레이.
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // 1. 벽 Normal 체크
    m_pAugusta->Check_ClimbableWall(&m_vWallNormal);

    // 2. ClimbAnimation의 전환.
    Update_ClimbAnimation(fTimeDelta);

    // 3. 상태 전환 체크
    if (!m_IsClimbed)
        Check_StateTransition(fTimeDelta);
}

void CAugustaClimbMove::OnExit()
{
    CClimbState::OnExit();
    m_IsSecondStep = false;

    m_pAugusta->Set_Gravity(true);
}

void CAugustaClimbMove::Setup_Animations()
{
    // 올라가는 것부터?
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_D_1), "Climb_D_1",  2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_D_2),  "Climb_D_2", 2.f, 0.f); 
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_DL_1), "Climb_DL_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_DL_2), "Climb_DL_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_DR_1), "Climb_DR_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_DR_2), "Climb_DR_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_L_1),  "Climb_L_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_L_2),  "Climb_L_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_R_1),  "Climb_R_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_R_2),  "Climb_R_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_U_1),  "Climb_U_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_U_2),  "Climb_U_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_UL_1),  "Climb_UL_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_UL_2),  "Climb_UL_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_UR_1),  "Climb_UR_1", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_UR_2),  "Climb_UR_2", 2.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbMoveType::CLIMB_VAULT),  "Climb_Vault", 2.f, 0.f);
}

// Climing Update
void CAugustaClimbMove::Update_ClimbAnimation(_float fTimeDelta)
{

    EClimbMoveType eClimbMoveType = static_cast<EClimbMoveType>(m_iCurrentAnimIdx);

    // 1. WASD 입력에 따라 등반 방향 결정
    _bool bW = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
    _bool bS = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
    _bool bA = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
    _bool bD = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));

    // 2. 두번째 재생 애니메이션이 아니고 애니메이션 실행이 종료되었다면? => 키입력이 있다면.
    if (m_IsAnimationEnd && m_pAugusta->Check_AnyInput(m_iMoveKey))
    {
        if (!m_IsSecondStep)
        {
            // 3. 두번째 재생 애니메이션이 아니라면..
            if (bW) {
                m_iCurrentAnimIdx = ENUM_CLASS(EClimbMoveType::CLIMB_U_2);
            }
            else if (bS) {
                m_iCurrentAnimIdx = ENUM_CLASS(EClimbMoveType::CLIMB_D_2);
            }
            else if (bA) {
                m_iCurrentAnimIdx = ENUM_CLASS(EClimbMoveType::CLIMB_L_2);
            }
            else if (bD) {
                // 오른쪽으로 등반
                m_iCurrentAnimIdx = ENUM_CLASS(EClimbMoveType::CLIMB_R_2);
            }
            m_IsClimbed = true;
            m_IsSecondStep = true;
            return;
        }
        if (m_IsSecondStep)
        {
            // 3. 두번째 재생 애니메이션이라면..
            if (bW) {
                m_iCurrentAnimIdx = ENUM_CLASS(EClimbMoveType::CLIMB_U_1);
            }
            else if (bS) {
                m_iCurrentAnimIdx = ENUM_CLASS(EClimbMoveType::CLIMB_D_1);
            }
            else if (bA) {
                m_iCurrentAnimIdx = ENUM_CLASS(EClimbMoveType::CLIMB_L_1);
            }
            else if (bD) {
                // 오른쪽으로 등반
                m_iCurrentAnimIdx = ENUM_CLASS(EClimbMoveType::CLIMB_R_1);
            }

            m_IsClimbed = true;
            m_IsSecondStep = false;
            return;
        }
    }
    
}

void CAugustaClimbMove::Check_StateTransition(_float fTimeDelta)
{

    EClimbMoveType eClimbMoveType = static_cast<EClimbMoveType>(m_iCurrentAnimIdx);

    // 0. 땅에 닿았다면?
    _float3 vNormal = {};
    _bool IsLand = m_pAugusta->Is_Land(&vNormal);

    if (IsLand)
    {
        // 위를 바라보고 아래키 누르고있으면?
        if (vNormal.y > 0.f && (eClimbMoveType == EClimbMoveType::CLIMB_D_1 || eClimbMoveType == EClimbMoveType::CLIMB_D_2))
        {
            m_pAugusta->GetStateContextForWrite().m_eLandType = ELandType::LAND_LIGHT;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
            return;
        }

    }

    // 1. 벽에서 떨어졌는지 체크
    if (!m_pAugusta->Check_ClimbableWall())
    {
        m_pAugusta->GetStateContextForWrite().m_eFallType = EFallType::FALL_LOOP;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL));
        return;
    }

    // 2. Space 입력 → 벽에서 점프 (Exit)
    if (m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE)))
    {
        m_pAugusta->GetStateContextForWrite().m_eClimbExitType = EClimbExitType::CLIMB_MOVE; // 점프.
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_EXIT));
        return;
    }

    // 3. 입력이 없고 애니메이션이 종료되었다면?
    if (!m_pAugusta->Check_AnyInput(m_iMoveKey) && m_IsAnimationEnd)
    {
        // 교체할 Exit Type
        EClimbExitType eClimbExitType = { EClimbExitType::CLIMB_D1_STOP };
        
        // 현재 MoveType
        switch (eClimbMoveType)
        {
        case EClimbMoveType::CLIMB_D_1:
            eClimbExitType = EClimbExitType::CLIMB_D1_STOP;
            break;
        case EClimbMoveType::CLIMB_D_2:
            eClimbExitType = EClimbExitType::CLIMB_D2_STOP;
            break;
        case EClimbMoveType::CLIMB_L_1:
            eClimbExitType = EClimbExitType::CLIMB_L1_STOP;
            break;
        case EClimbMoveType::CLIMB_L_2:
            eClimbExitType = EClimbExitType::CLIMB_L2_STOP;
            break;
        case EClimbMoveType::CLIMB_R_1:
            eClimbExitType = EClimbExitType::CLIMB_R1_STOP;
            break;
        case EClimbMoveType::CLIMB_R_2:
            eClimbExitType = EClimbExitType::CLIMB_R2_STOP;
            break;
        case EClimbMoveType::CLIMB_U_1:
            eClimbExitType = EClimbExitType::CLIMB_U1_STOP;
            break;
        case EClimbMoveType::CLIMB_U_2:
            eClimbExitType = EClimbExitType::CLIMB_U2_STOP;
            break;
        }

        m_pAugusta->GetStateContextForWrite().m_eClimbExitType = eClimbExitType;
        m_pAugusta->GetStateContextForWrite().m_IsClimbSecondStep = m_IsSecondStep; // 두번째 상태였으면?
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_EXIT));
        return;
    }
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
