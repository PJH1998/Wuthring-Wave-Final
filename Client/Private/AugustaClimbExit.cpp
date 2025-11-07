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



void CAugustaClimbExit::OnEnter(void* pArg)
{
    CClimbState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaClimbExitType eClimbExitType = context.m_eClimbExitType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eClimbExitType);

    // 두번째 상태값이였는지 체크.
    m_IsSecondStep = context.m_IsClimbSecondStep;

    // 예외 처리.
    _bool IsGravity = { false };
    if (eClimbExitType == EAugustaClimbExitType::CLIMB_MOVE)
        IsGravity = true;
    
    m_pAugusta->Set_Gravity(IsGravity);

    State_Reset();
}

void CAugustaClimbExit::OnUpdate(_float fTimeDelta)
{
    CClimbState::OnUpdate(fTimeDelta);

    // 0. Key Input
    Handle_Input();

    // 1. 애니메이션 재생.
    Update_ClimbAnimation(fTimeDelta);

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    // 3. 전환 조건 체크
    Check_StateTransition(fTimeDelta);

    State_Reset();
}

void CAugustaClimbExit::OnExit()
{
    CClimbState::OnExit();
    m_pAugusta->Set_Gravity(false);
}



void CAugustaClimbExit::Handle_Input()
{
    EAugustaClimbExitType eClimbExitType = static_cast<EAugustaClimbExitType>(m_iCurrentAnimIdx);

    if ((eClimbExitType != EAugustaClimbExitType::CLIMB_ONTOP) && (eClimbExitType != EAugustaClimbExitType::CLIMB_MOVE))
        m_States[BACKJUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));

    m_States[U] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
    m_States[D] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
    m_States[R] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::A)); // 방향 반대.
    m_States[L] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::D)); // 방향 반대
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);

}

// Climb에 관련된 Update
void CAugustaClimbExit::Update_ClimbAnimation(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    _float3 vNormal = {};
    // Space 키를 눌렀다면?

    // 0. Move, OnTop상태일때만 예외처리.
    if (m_States[BACKJUMP])
    {
        m_States[IS_CLIMBEXIT] = true;
        m_iCurrentAnimIdx = ENUM_CLASS(EAugustaClimbExitType::CLIMB_MOVE);
        return;
    }

}

void CAugustaClimbExit::Check_Physics(_float fTimeDelta)
{
    m_States[WALL] = m_pAugusta->Check_ClimbableWall(&m_vWallNormal);
	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);
}

void CAugustaClimbExit::Check_StateTransition(_float fTimeDelta)
{

    EAugustaClimbExitType eClimbExitType = static_cast<EAugustaClimbExitType>(m_iCurrentAnimIdx);

    // ONTOP 애니메이션 끝나면 벽 위에 착지
    

    // 땅에 닿았다면?
    if (m_States[LAND])
    {
        if (eClimbExitType == EAugustaClimbExitType::CLIMB_ONTOP && m_IsAnimationEnd)
        {
            m_pAugusta->GetStateContextForWrite().m_eLandType = EAugustaLandType::LAND_LIGHT;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
            return;
        }
        // 위를 바라본다면?
        if (m_vLandNormal.y > 0.f)
        {
            m_pAugusta->GetStateContextForWrite().m_eLandType = EAugustaLandType::LAND_LIGHT;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LAND));
            return;
        }
    }

    // 입력 있을때만.
    EAugustaClimbMoveType eClimbMoveType = { EAugustaClimbMoveType::CLIMB_U_1 };
    if (m_States[MOVE])
    {
        if (m_IsSecondStep)
        {
            // 교체할 Move Type
            if (m_States[U])
                eClimbMoveType = EAugustaClimbMoveType::CLIMB_U_2;
            else if (m_States[D])
                eClimbMoveType = EAugustaClimbMoveType::CLIMB_D_2;
            else if (m_States[L])
                eClimbMoveType = EAugustaClimbMoveType::CLIMB_L_2;
            else if (m_States[R])
                eClimbMoveType = EAugustaClimbMoveType::CLIMB_R_2;

            m_pAugusta->GetStateContextForWrite().m_eClimbMoveType = eClimbMoveType;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_MOVE));
            return;
        }
        if (!m_IsSecondStep)
        {
            if (m_States[U])
                eClimbMoveType = EAugustaClimbMoveType::CLIMB_U_1;
            else if (m_States[D])
                eClimbMoveType = EAugustaClimbMoveType::CLIMB_D_1;
            else if (m_States[L])
                eClimbMoveType = EAugustaClimbMoveType::CLIMB_L_1;
            else if (m_States[R])
                eClimbMoveType = EAugustaClimbMoveType::CLIMB_R_1;

            m_pAugusta->GetStateContextForWrite().m_eClimbMoveType = eClimbMoveType;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(EAugustaClimbState::CLIMB_MOVE));
            return;
        }
    }

    
}

void CAugustaClimbExit::Setup_Animations()
{
    // 올라가는 것부터?
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_D1_STOP), "Climb_D1_Stop", 1.f, 0.f, 1.f, false);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_D2_STOP), "Climb_D2_Stop", 1.f, 0.f, 1.f, false);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_DL1_STOP), "Climb_DL1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_DL2_STOP), "Climb_DL1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_DR1_STOP), "Climb_DR1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_DR2_STOP), "Climb_DR1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_L1_STOP), "Climb_L1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_L2_STOP), "Climb_L2_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_R1_STOP), "Climb_R1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_R2_STOP), "Climb_R2_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_U1_STOP), "Climb_U1_Stop", 1.f, 0.f, 1.f, false);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_U2_STOP), "Climb_U2_Stop", 1.f, 0.f, 1.f, false);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_UL1_STOP), "Climb_UL1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_UL2_STOP), "Climb_UL2_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_UR1_STOP), "Climb_UR1_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_UR2_STOP), "Climb_UR2_Stop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_ONTOP), "Climb_OnTop", 1.5f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_MOVE), "Climb_Move", 1.f, 0.f, 3.f, true, false); // 루트모션 회전 끄니까 됨.
    CState::Add_Animations(ENUM_CLASS(EAugustaClimbExitType::CLIMB_VAULT), "Climb_Vault", 2.f, 0.f); // 어찌보면 이것도 달출인데?
}

void CAugustaClimbExit::State_Reset()
{
    for (_uint i = 0; i < CLIMBSTATE::END; ++i)
        m_States[i] = false;
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
