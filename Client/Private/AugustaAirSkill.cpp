#include "ClientPch.h"
#include "AugustaAirSkill.h"
#include "Augusta.h"
#include "StateMachine.h"

HRESULT CAugustaAirSkill::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CAirState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    return S_OK;
}


void CAugustaAirSkill::OnEnter()
{
    CAirState::OnEnter();

    // 1. 복사본 Context 받아오기
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaAirAttackType eAirAttackType = context.m_eAirAttackType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eAirAttackType);

    // 4. Attack 상태 초기화
    State_Reset();

    // 5. 무기 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음(Attack은)
    m_iPartType = CAugusta::PARTTYPE::PART_BAYONET; // 추후 애니메이션에 따른. 분기문 필요.

    // 6. 무기에 Bone 붙이기. + Offset 추가.
    _string strBoneName = "WeaponProp05";
    m_pAugusta->PartActivate(m_iPartType, true);
    m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);

    // 7. 다른 애니메이션당 필요한 상태 재정의
    switch (eAirAttackType)
    {
    case EAugustaAirAttackType::AIRATTACK_HACKDOWN_START:
    {
        //m_pAugusta->Set_ColliderReferenceBone("Bip001", { 0.f, 0.5f, 0.f });
        m_fSpeed = 0.5f;
        m_pAugusta->Set_Gravity(true);
    }
        break;
    case EAugustaAirAttackType::AIRATTACK_HACKDOWN_SP_END:
    {
        //m_pAugusta->Set_ColliderReferenceBone("Bip001", { 0.f, 0.5f, 0.f });
        m_fSpeed = 0.f;
        m_pAugusta->Set_Gravity(true);
    }
        break;
    }

    // 8. Enter에 들어오면 한번 회전.
    m_pAugusta->Rotate_Target(); 

}

void CAugustaAirSkill::OnUpdate(_float fTimeDelta)
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

void CAugustaAirSkill::OnExit()
{
    CAirState::OnExit();

    // 콤보 카운트 초기화
    m_pAugusta->PartActivate(m_iPartType, false); 
    m_pAugusta->Set_Gravity(true);

    // Sync Bone 초기화
    //m_pAugusta->Set_ColliderReferenceBone("");
}

void CAugustaAirSkill::Handle_Input()
{
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
        
}

void CAugustaAirSkill::Update_AttackAnimations(_float fTimeDelta)
{
    // 1. 애니메이션 실행.
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // 2.
    _vector vLook = m_pAugusta->Get_LookVector();

    EAugustaAirAttackType eAirAttackType = static_cast<EAugustaAirAttackType>(m_iCurrentAnimIdx);
    if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_HACKDOWN_START)
    {
        m_pAugusta->Move_Direction(vLook, fTimeDelta, m_fSpeed); // 조금씩 앞으로 이동?
    }

    // Attack State에 해당하는 경우 모두 Animation이 존재.
    m_pAugusta->Play_PartAnimation(
        m_iPartType,
        m_Animations[m_iCurrentAnimIdx].strAnimName,
        fTimeDelta, nullptr
    );
}

void CAugustaAirSkill::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pAugusta->Is_Land();
    //m_pAugusta->Set_ColliderReferenceBone("Bip001", { 0.f, 0.5f, 0.f }); // 실시간 Offset 수정.
}

void CAugustaAirSkill::LockOn_StateTransition(_float fTimeDelta)
{
}

void CAugustaAirSkill::Check_StateTransition(_float fTimeDelta)
{
    // 1. 스킬 입력 (E, R 등) 들어오면 Skill로 => 우선순위 별.
    // ... 추후 구현
    // 2. Normal Attack의 경우 콤보 공격이 가능하게.
    
    EAugustaAirAttackType eAirAttackType = static_cast<EAugustaAirAttackType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();
    _float fOffsetY = 0.1f;
    _float fDistanceToGround = m_pAugusta->Get_DistanceFromGround(fOffsetY);

    if (IsEscapePossible)
    {
        if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_HACKDOWN_SP_END)
        {
            if (m_States[MOVE])
            {
                if (fDistanceToGround <= 0.2f)
                {
                    m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        
                    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN)); // 상위, 하위 상태
                    return;
                }
                if (fDistanceToGround > 0.2f)
                {
                    m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
                    m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
                    return;
                }
            }
        }
    }

    // 우선순위 순서대로
    if (m_IsAnimationEnd)
    {
        if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_HACKDOWN_START)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_HACKDOWN_SP_END);
            return;
        }

        if (eAirAttackType == EAugustaAirAttackType::AIRATTACK_HACKDOWN_SP_END)
        {
            if (fDistanceToGround <= 0.2f)
            {
                m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
                return;
            }
            if (fDistanceToGround > 0.2f)
            {
                m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
                return;
            }
            
        }
    }

}

void CAugustaAirSkill::SetUp_Animations()
{
    
    CState::Add_Animations(ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_HACKDOWN_START),"AirAttack_HackDown_Start", 1.3f, 20.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_HACKDOWN_LOOP),"AirAttack_HackDown_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_HACKDOWN_SP_END),"AirAttack_HackDown_Sp_End", 1.3f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_START),"AirAttack_Start", 1.f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_LOOP),"AirAttack_Loop", 1.f, 0.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAirAttackType::AIRATTACK_END),"AirAttack_End", 1.f, 0.f, 1.f);
}

void CAugustaAirSkill::State_Reset()
{
    for (_uint i = 0; i < AIRSKILLSTATE::END; ++i)
        m_States[i] = false;
}

CAugustaAirSkill* CAugustaAirSkill::Create(class CGameObject* pOwner)
{
    CAugustaAirSkill* pInstance = new CAugustaAirSkill();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaAirSkill");
    }

    return pInstance;
}

void CAugustaAirSkill::Free()
{
    CAirState::Free();
}
