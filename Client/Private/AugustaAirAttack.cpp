#include "ClientPch.h"
#include "AugustaAirAttack.h"
#include "Augusta.h"
#include "StateMachine.h"

HRESULT CAugustaAirAttack::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    return S_OK;
}


void CAugustaAirAttack::OnEnter()
{
    CAirState::OnEnter();

    // 1. 복사본 Context 받아오기
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAirAttackType eAirAttackType = context.m_eAirAttackType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eAirAttackType);

    // 4. Attack 상태 초기화
    State_Reset();

    // 5. 무기 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음(Attack은)
    m_iPartType = CAugusta::PARTTYPE::PART_BAYONET; // 추후 애니메이션에 따른. 분기문 필요.

    _string strBoneName = "WeaponProp02";
    m_pAugusta->PartAcitvate(m_iPartType, true);
    m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);

    // 점공이니까 한번만? => 카메라 락온상태일때 뭔가 문제가 있다.
    m_pAugusta->Rotate_Target(); 
    m_pAugusta->Set_Gravity(false);
}

void CAugustaAirAttack::OnUpdate(_float fTimeDelta)
{
    CAirState::OnUpdate(fTimeDelta);

    // 0. 입력 확인
    Handle_Input();

    // 1. Attack 업데이트
    Update_AttackAnimations(fTimeDelta);

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    // 3. 전환 조건 체크
    Check_StateTransition(fTimeDelta);

    State_Reset();
}

void CAugustaAirAttack::OnExit()
{
    CAirState::OnExit();

    // 콤보 카운트 초기화
    m_pAugusta->PartAcitvate(m_iPartType, false); 
    m_pAugusta->Set_Gravity(true);
}

void CAugustaAirAttack::Handle_Input()
{
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
        
}

void CAugustaAirAttack::Update_AttackAnimations(_float fTimeDelta)
{
    // 1. 현재 애니메이션 재생
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // Target이 존재한다면? => Auto Target
    
    // Attack State에 해당하는 경우 모두 Animation이 존재.
    m_pAugusta->Play_PartAnimation(
        m_iPartType,
        m_Animations[m_iCurrentAnimIdx].strAnimName,
        fTimeDelta, nullptr
    );
}

void CAugustaAirAttack::Check_Physics(_float fTimeDelta)
{
    m_States[LAND] = m_pAugusta->Is_Land(&m_vLandNormal);
    m_pAugusta->Sync_Collider();
}

void CAugustaAirAttack::LockOn_StateTransition(_float fTimeDelta)
{
}

void CAugustaAirAttack::Check_StateTransition(_float fTimeDelta)
{
    // 1. 스킬 입력 (E, R 등) 들어오면 Skill로 => 우선순위 별.
    // ... 추후 구현
    // 2. Normal Attack의 경우 콤보 공격이 가능하게.
    
    EAirAttackType eAirAttackType = static_cast<EAirAttackType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();
    _float fOffsetY = 0.1f;
    _float fDistanceToGround = m_pAugusta->Get_DistanceToGround(fOffsetY);

    if (IsEscapePossible)
    {
        if (eAirAttackType == EAirAttackType::AIRATTACK_HACKDOWN_SP_END)
        {
            if (m_States[MOVE] && m_States[LAND])
            {
                m_pAugusta->GetStateContextForWrite().m_eRunType = ERunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN)); // 상위, 하위 상태
                return;
            }
        }
    }

    // 우선순위 순서대로
    if (m_IsAnimationEnd)
    {
        if (eAirAttackType == EAirAttackType::AIRATTACK_HACKDOWN_START)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(EAirAttackType::AIRATTACK_HACKDOWN_SP_END);
            return;
        }

        if (eAirAttackType == EAirAttackType::AIRATTACK_HACKDOWN_SP_END)
        {
            if (m_pAugusta->Is_Land(&m_vLandNormal))
            {
                m_pAugusta->GetStateContextForWrite().m_eIdleType = EIdleType::STAND1_ACTION01;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
                return;
            }
            else
            {
                m_pAugusta->GetStateContextForWrite().m_eFallType = EFallType::FALL_LOOP;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
                return;
            }
            
        }
    }

}

void CAugustaAirAttack::SetUp_Animations()
{
    
    CState::Add_Animations(ENUM_CLASS(EAirAttackType::AIRATTACK_HACKDOWN_START),"AirAttack_HackDown_Start", 1.3f, 20.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAirAttackType::AIRATTACK_HACKDOWN_LOOP),"AirAttack_HackDown_Loop", 1.f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAirAttackType::AIRATTACK_HACKDOWN_SP_END),"AirAttack_HackDown_Sp_End", 1.3f, 20.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAirAttackType::AIRATTACK_START),"AirAttack_Start", 1.f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAirAttackType::AIRATTACK_LOOP),"AirAttack_Loop", 1.f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAirAttackType::AIRATTACK_END),"AirAttack_End", 1.f, 0.f, 1.f);
}

void CAugustaAirAttack::State_Reset()
{
    for (_uint i = 0; i < AIRATTACKSTATE::END; ++i)
        m_States[i] = false;
}

CAugustaAirAttack* CAugustaAirAttack::Create(class CGameObject* pOwner)
{
    CAugustaAirAttack* pInstance = new CAugustaAirAttack();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaAirAttack");
    }

    return pInstance;
}

void CAugustaAirAttack::Free()
{
    CAirState::Free();
}
