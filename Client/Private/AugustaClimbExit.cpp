#include "ClientPch.h"
#include "AugustaClimbExit.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaClimbExit::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CClimbState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaClimbExit::OnEnter()
{
    CClimbState::OnEnter();

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EClimbExitType eClimbExitType = context.m_eClimbExitType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eClimbExitType);

    // 두번째 상태값이였는지 체크.
    m_IsSecondStep = context.m_IsClimbSecondStep;

    // 예외 처리.
    _bool IsGravity = { false };
    if (eClimbExitType == EClimbExitType::CLIMB_MOVE)
        IsGravity = true;
    
    m_pAugusta->Set_Gravity(IsGravity);
}

void CAugustaClimbExit::OnUpdate(_float fTimeDelta)
{
    CClimbState::OnUpdate(fTimeDelta);

    // 0. 애니메이션 플레이.
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);


    Update_ClimbAnimation(fTimeDelta);

    if (m_IsClimbExit)
        Check_StateTransition(fTimeDelta);
}

void CAugustaClimbExit::OnExit()
{
    CClimbState::OnExit();
    m_pAugusta->Set_Gravity(false);
}

void CAugustaClimbExit::Setup_Animations()
{
    // 올라가는 것부터?
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_D1_STOP), "Climb_D1_Stop", 1.f, 0.f, 1.f, false);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_D2_STOP), "Climb_D2_Stop", 1.f, 0.f, 1.f, false);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_DL1_STOP), "Climb_DL1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_DL2_STOP), "Climb_DL1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_DR1_STOP), "Climb_DR1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_DR2_STOP), "Climb_DR1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_L1_STOP), "Climb_L1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_L2_STOP), "Climb_L2_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_R1_STOP), "Climb_R1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_R2_STOP), "Climb_R2_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_U1_STOP), "Climb_U1_Stop", 1.f, 0.f, 1.f, false);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_U2_STOP), "Climb_U2_Stop", 1.f, 0.f, 1.f, false);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_UL1_STOP), "Climb_UL1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_UL2_STOP), "Climb_UL2_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_UR1_STOP), "Climb_UR1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_UR2_STOP), "Climb_UR2_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_ONTOP), "Climb_OnTop", 1.5f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EClimbExitType::CLIMB_MOVE), "Climb_Move", 1.f, 0.f, 1.f, true, false); // 루트모션 회전 끄니까 됨.
}

// Climb에 관련된 Update
void CAugustaClimbExit::Update_ClimbAnimation(_float fTimeDelta)
{
    
    EClimbExitType eClimbExitType = static_cast<EClimbExitType>(m_iCurrentAnimIdx);
    _float3 vNormal = {};

    // Space 키를 눌렀다면?
    

    // 0. Move, OnTop상태일때만 예외처리.
    if (eClimbExitType == EClimbExitType::CLIMB_MOVE || eClimbExitType == EClimbExitType::CLIMB_ONTOP)
    {
        m_IsClimbExit = true;
        return;
    }

    // Move상태가 아닌데 Space키를 눌렀다면?
    if (m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE)))
    {
        m_IsClimbExit = true;
        m_iCurrentAnimIdx = ENUM_CLASS(EClimbExitType::CLIMB_MOVE);
        return;
    }

    // 1. WASD 입력에 따라 등반 방향 결정
    _bool bW = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
    _bool bS = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
    _bool bA = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
    _bool bD = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));


    EClimbMoveType eClimbMoveType = { EClimbMoveType::CLIMB_U_1 };

    // 입력 있을때만.
    if (m_pAugusta->Check_AnyInput(m_iMoveKey))
    {
        if (m_IsSecondStep)
        {
            // 교체할 Move Type
            if (bW)
                eClimbMoveType = EClimbMoveType::CLIMB_U_2;
            else if (bS)
                eClimbMoveType = EClimbMoveType::CLIMB_D_2;
            else if (bA)
                eClimbMoveType = EClimbMoveType::CLIMB_L_2;
            else if (bD)
                eClimbMoveType = EClimbMoveType::CLIMB_R_2;

            m_pAugusta->GetStateContextForWrite().m_eClimbMoveType = eClimbMoveType;
            //m_pAugusta->GetStateContextForWrite().m_IsClimbSecondStep = !m_IsSecondStep; // SecondStep이였으면 SecondStep이 아닌 것으로 교체. (발 바꿈)
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_MOVE));
            return;
        }
        if (!m_IsSecondStep)
        {
            if (bW)
                eClimbMoveType = EClimbMoveType::CLIMB_U_1;
            else if (bS)
                eClimbMoveType = EClimbMoveType::CLIMB_D_1;
            else if (bA)
                eClimbMoveType = EClimbMoveType::CLIMB_L_1;
            else if (bD)
                eClimbMoveType = EClimbMoveType::CLIMB_R_1;

            m_pAugusta->GetStateContextForWrite().m_eClimbMoveType = eClimbMoveType;
            //m_pAugusta->GetStateContextForWrite().m_IsClimbSecondStep = m_IsSecondStep; // SecondStep이였으면 SecondStep이 아닌 것으로 교체. (발 바꿈)
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_MOVE));
            return;
        }
    }
}

void CAugustaClimbExit::Check_StateTransition(_float fTimeDelta)
{
    // 땅에 닿았다면?
    _float3 vNormal = {};
    _bool IsLand = m_pAugusta->Is_Land(&vNormal);

    if (IsLand)
    {
        // 위를 바라본다면?
        if (vNormal.y > 0.f)
        {
            m_pAugusta->GetStateContextForWrite().m_eLandType = ELandType::LAND_LIGHT;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
            return;
        }
        
    }
        

   
}






CAugustaClimbExit* CAugustaClimbExit::Create(class CGameObject* pOwner)
{
    CAugustaClimbExit* pInstance = new CAugustaClimbExit();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaClimbExit");
        return nullptr;
    }

    return pInstance;
}

void CAugustaClimbExit::Free()
{
    CClimbState::Free();
}
