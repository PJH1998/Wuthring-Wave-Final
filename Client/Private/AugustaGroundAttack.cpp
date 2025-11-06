#include "ClientPch.h"
#include "AugustaGroundAttack.h"
#include "Augusta.h"
#include "StateMachine.h"

HRESULT CAugustaGroundAttack::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    

    return S_OK;
}


void CAugustaGroundAttack::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaAttackType eAttackType = context.m_eAttackType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eAttackType);

    // 4. Attack 상태 초기화
    State_Reset();


    // 5. 무기 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음(Attack은)
    m_iPartType = CAugusta::PARTTYPE::PART_BAYONET; // 추후 애니메이션에 따른. 분기문 필요.

    _string strBoneName = "WeaponProp02";
    m_pAugusta->PartActivate(m_iPartType, true);
    m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
    m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
	m_pAugusta->Set_Gravity(true);


}

void CAugustaGroundAttack::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 입력 확인
    Handle_Input();

    // 1. Attack 업데이트
    Update_AttackAnimations(fTimeDelta);

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    // 3. LockOn 여부 확인 및 상태 전환

    Check_StateTransition(fTimeDelta);

    State_Reset();
}

void CAugustaGroundAttack::OnExit()
{
    CGroundState::OnExit();

    // 콤보 카운트 초기화
    m_iComboCount = 0;
    m_fAttackPressTime = 0.f; // 시간 초기화
    m_pAugusta->PartActivate(m_iPartType, false); 
}

void CAugustaGroundAttack::Handle_Input()
{
    
    EAugustaAttackType eAttackType = static_cast<EAugustaAttackType>(m_iCurrentAnimIdx);

    // HEAVY_ATTACK_PENDING(강공 발생 조건)
    // Attack이 01이고 키를 애니메이션 탈출 가능 상태까지 계속 누르고 있다면?

	m_States[HEAVY_ATTACK_PENDING] = (eAttackType == EAugustaAttackType::ATTACK01)
		&& (m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB), KEYSTATE::PRESS));
		



    // 입력키 체크
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));

    // 스킬 체크
    m_States[SKILL_Q] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::Q));
    m_States[SKILL_E] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));
    m_States[SKILL_R] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::R));

    

	m_States[ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

    if (eAttackType >= EAugustaAttackType::ATTACK01 && eAttackType < EAugustaAttackType::ATTACK04)
    {
        if (m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB), KEYSTATE::PRESS))
            m_IsNextAttackInput = true;
    }
    
}

void CAugustaGroundAttack::Update_AttackAnimations(_float fTimeDelta)
{
    // 1. 현재 애니메이션 재생
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // Target이 존재한다면? => Auto Target
    m_pAugusta->Rotate_Target();

    // 1타 모션일때 누르고 있다면?
    if (m_States[HEAVY_ATTACK_PENDING])
        m_fAttackPressTime += fTimeDelta;

    // Attack State에 해당하는 경우 모두 Animation이 존재.
    m_pAugusta->Play_PartAnimation(
        m_iPartType,
        m_Animations[m_iCurrentAnimIdx].strAnimName,
        fTimeDelta, nullptr
    );
}

void CAugustaGroundAttack::Check_Physics(_float fTimeDelta)
{

}

void CAugustaGroundAttack::LockOn_StateTransition(_float fTimeDelta)
{
}

void CAugustaGroundAttack::Check_StateTransition(_float fTimeDelta)
{
    // 1. 스킬 입력 (E, R 등) 들어오면 Skill로 => 우선순위 별.
    // ... 추후 구현
    // 2. Normal Attack의 경우 콤보 공격이 가능하게.
    
    EAugustaAttackType eAttackType = static_cast<EAugustaAttackType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();
    // 우선순위 순서대로
    
    // 0. 1타모션에서 계속 누르고 임계시간을 넘으면?
    if (m_States[HEAVY_ATTACK_PENDING] && (m_fAttackPressTime >= m_fAttackPressMaxTime) && IsEscapePossible)
    {
		if (SKILL_STATE::READY == m_pAugusta->Use_Skill("Attack_HeavyHack"))
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAttackType::ATTACK_HEAVYHACK);
			m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
			m_fAttackPressTime = 0.f;
			return;
		}
    }

    /*if (m_States[HEAVY_ATTACK_PENDING] && IsEscapePossible)
    {
        m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAttackType::ATTACK_HEAVYHACK);
        m_fAttackPressTime = 0.f;
        return;
    }*/
    
    // 1. 기본 공상태에서 Heavy_Attack_Pending이 아닌 경우?
    if (eAttackType >= EAugustaAttackType::ATTACK01 && eAttackType < EAugustaAttackType::ATTACK04)
    {
        // 키 누르고 있다면 다른 상태전환하지 말고 계속 Attack01 실행. => 강공을 위해.
        if (eAttackType == EAugustaAttackType::ATTACK01 && m_States[HEAVY_ATTACK_PENDING])
        {
            return;
        }
        
        if (m_IsNextAttackInput && IsEscapePossible)
        {
            m_iComboCount++;
			m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName); // 애니메이션 초기화
            m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAttackType::ATTACK01) + m_iComboCount;
            m_IsNextAttackInput = false;
            m_fAttackPressTime = 0.f; // Attack02나 03으로 전환되므로 PressTime 초기화
            return;
        }
        
    }
    
    // 위 상태에서 안걸렸으면 무조건 초기화
    m_IsNextAttackInput = false;

    // 2. 애니메이션 탈출 조건인 경우.
    if (IsEscapePossible)
    {
        // 점프키 => 내부 우선순위 높음. (입력 보다) ex) w space 동시에 눌렀으면? => space 먼저 판별.
        if (m_States[JUMP])
        {
            m_pAugusta->GetStateContextForWrite().m_eJumpType = EAugustaJumpType::JUMP_WALK_LF;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::JUMP));
            return;
        }

        // 입력키
		
        if (m_States[MOVE])
        {
			// Escape가 너무 빠르게 동작함. => 공격 전환 TrackPoistion과 이동 전환 TrackPosition이 달라야할듯?
			if ((eAttackType == EAugustaAttackType::ATTACK01) && m_fTrackPosition < 30.f)
			{
				return;
			}

            m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
            return;
        }
        
    }

 
    //공격 애니메이션 끝나고 추가 입력 없으면 Idle로 => 가장 우선순위 낮음.
    if (m_IsAnimationEnd)
    {
        m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
        m_IsNextAttackInput = false;
        return;
    }
    
    

    
}

void CAugustaGroundAttack::SetUp_Animations()
{
    
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK01),"Attack01", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK02),"Attack02", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK03),"Attack03", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK04),"Attack04", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK_HEAVYHACK),"Attack_HeavyHack", 1.f, 80.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK_PULL), "Attack_Pull", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK_SPEEDDRIVE),"Attack_SpeedDrive", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK_SPSKILL),"Attack_SpSkill", 1.f, 0.f);
}

void CAugustaGroundAttack::State_Reset()
{
    for (_uint i = 0; i < ATTACKSTATE::END; ++i)
        m_States[i] = false;
}

CAugustaGroundAttack* CAugustaGroundAttack::Create(class CGameObject* pOwner)
{
    CAugustaGroundAttack* pInstance = new CAugustaGroundAttack();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundAttack");
    }

    return pInstance;
}

void CAugustaGroundAttack::Free()
{
    CGroundState::Free();
}
