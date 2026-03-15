#include "ClientPch.h"
#include "GalbrenaGroundHeavyAttack.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaShotGun.h"
#include "Ability.h"

HRESULT CGalbrenaGroundHeavyAttack::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pCharacter);
    ASSERT_CRASH(m_pGalbrena);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

	// 미리 사용할 공간 선언.
	m_ActivePartTypes.reserve(CGalbrena::PARTTYPE::TYPE_END);

    return S_OK;
}


void CGalbrenaGroundHeavyAttack::OnEnter(void* pArg)
{
	CGroundState::OnEnter(pArg);

	// 1. 복사본 Context 받아오기
	const auto context = m_pGalbrena->TakeStateContext();

	// 2. 복사본에서 필요한 값 읽기
	EGalbrenaHeavyAttackType eHeavyAttackType = context.m_eHeavyAttackType;

	// 3. 애니메이션 세팅.
	m_iCurrentAnimIdx = ENUM_CLASS(eHeavyAttackType);

	// 4. Attack 상태 초기화
	State_Reset();

	
	m_ActivePartTypes.clear(); // 파츠 목록 초기화

	// 5. 무기 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음(Attack은)
	switch (eHeavyAttackType)
	{
	case EGalbrenaHeavyAttackType::ATTACK_H_0201:
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_FIRSTGUN);
		//m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_SECONDGUN);
		break;
	case EGalbrenaHeavyAttackType::ATTACK_H_0202:
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_FIRSTGUN);
		//m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_SECONDGUN);
		break;
	case EGalbrenaHeavyAttackType::ATTACK_H_0203:
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_FIRSTGUN);
		//m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_SECONDGUN);
		break;
	}

	// 6. 사용하는 PartType이 있다면?
	for (auto& PartType : m_ActivePartTypes)
		m_pGalbrena->PartActivate(PartType, true);

	// 7. 중력 활성화 및 Target 회전.
	m_pGalbrena->Set_Gravity(true);
	m_pGalbrena->Rotate_Target();
}

void CGalbrenaGroundHeavyAttack::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 입력 확인
    Handle_Input();

    // 1. Attack 업데이트
    Update_AttackAnimations(fTimeDelta);

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

	// 3. 전환조건 체크.
    Check_StateTransition(fTimeDelta);

	// 4. 상태 초기화
    State_Reset();
}

void CGalbrenaGroundHeavyAttack::OnExit()
{
    CGroundState::OnExit();

	// Part Activate 비활성화
	for (auto& PartType : m_ActivePartTypes)
		m_pGalbrena->PartActivate(PartType, false);

    // 콤보 카운트 초기화
    m_iComboCount = 0;
	m_iCurrentAnimIdx = 0;
    m_fAttackPressTime = 0.f; // 시간 초기화
    m_pGalbrena->PartActivate(m_iPartType, false); 
    m_pGalbrena->PartActivate(m_iSubPartType, false); 

	m_pGalbrena->Collider_Active(TEXT("Main|X|X"), false);
}

_bool CGalbrenaGroundHeavyAttack::Hit_Judge()
{
	_bool IsHit = false;
	const HIT_DESC* pDesc = m_pGalbrena->GetPendingHitDesc();

	if (nullptr == pDesc)
		return false;

	COLLISIONLAYER eLayer = static_cast<COLLISIONLAYER>(m_pGalbrena->GetPendingHitDesc()->iLayer);
	if (eLayer == COLLISIONLAYER::ENEMY_SKILL)
		IsHit = true;

	return IsHit;
}

void CGalbrenaGroundHeavyAttack::Handle_Input()
{
    EGalbrenaHeavyAttackType eHeavyAttackType = static_cast<EGalbrenaHeavyAttackType>(m_iCurrentAnimIdx);

	m_States[HIT_PENDING] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT));

    // HEAVY_ATTACK_PENDING(강공 발생 조건)
    // Attack이 01이고 키를 애니메이션 탈출 가능 상태까지 계속 누르고 있다면?
	m_States[HEAVY_ATTACK_PENDING] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB), KEYSTATE::PRESS);

    // 입력키 체크
    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[DASH] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));

    // 스킬 체크
    m_States[SKILL_Q] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::Q));
    m_States[SKILL_E] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));
    m_States[SKILL_R] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::R));


	// Cost 계속
	m_States[BURST] = (m_pGalbrena->Get_Cost(COST_TYPE::COST1) >= m_pGalbrena->Get_MaxCost());
	


	// 공격시에 Hit 받았을때는 좀더 판단을 빡빡하게
	if (m_pGalbrena->Is_Hit())
		m_States[HIT] = Hit_Judge();
	
    
}

void CGalbrenaGroundHeavyAttack::Update_AttackAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pGalbrena->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; // 거리 계산에 따른 Animation Scale 조절.

    // 1. 현재 애니메이션 재생
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta, m_fAnimationScale);

    // Target이 존재한다면? => Auto Target
   

    // 1타 모션일때 누르고 있다면?
    if (m_States[HEAVY_ATTACK_PENDING])
        m_fAttackPressTime += fTimeDelta;

    // Attack State에 해당하는 경우 모두 Animation이 존재.

	 // 2. 파츠 실행.
	for (auto& iPartType : m_ActivePartTypes)
	{
		m_pGalbrena->Play_PartAnimation(
			iPartType,
			m_Animations.at(m_iCurrentAnimIdx).strAnimName,
			m_Animations.at(m_iCurrentAnimIdx).fSpeed * fTimeDelta, nullptr
		);
	}
}

void CGalbrenaGroundHeavyAttack::Check_Physics(_float fTimeDelta)
{

}

void CGalbrenaGroundHeavyAttack::LockOn_StateTransition(_float fTimeDelta)
{
}

void CGalbrenaGroundHeavyAttack::Check_StateTransition(_float fTimeDelta)
{
    // 1. 스킬 입력 (E, R 등) 들어오면 Skill로 => 우선순위 별.
    // ... 추후 구현
    // 2. Normal Attack의 경우 콤보 공격이 가능하게.
    
	EGalbrenaHeavyAttackType eAttackType = static_cast<EGalbrenaHeavyAttackType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();
    // 우선순위 순서대로
    
	if (m_States[HIT])
	{
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EGalbrenaHitState::HIT));
		return;
	}

	if (m_States[DASH])
	{
		m_pGalbrena->GetStateContextForWrite().m_eSpecialDashType = EGalbrenaSpecialDashType::ATTACK_CHARGE;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::SPECIALDASH));
		return;
	}

	// 1. 기본 공상태에서 Heavy_Attack_Pending이 아닌 경우?
	if (m_States[HEAVY_ATTACK_PENDING])
	{
		if (IsEscapePossible)
		{
			switch (eAttackType)
			{
			case EGalbrenaHeavyAttackType::ATTACK_H_0201:
				m_pGalbrena->GetStateContextForWrite().m_eHeavyAttackType = EGalbrenaHeavyAttackType::ATTACK_H_0202;
				break;
			case EGalbrenaHeavyAttackType::ATTACK_H_0202:
				m_pGalbrena->GetStateContextForWrite().m_eHeavyAttackType = EGalbrenaHeavyAttackType::ATTACK_H_0203;
				break;
			case EGalbrenaHeavyAttackType::ATTACK_H_0203: // 203인 경우에는 바꾸지 않도록.
				return;
				break;
			}
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::HEAVYATTACK));
			return;
		}
	}

	// 위 상태에서 안걸렸으면 무조건 초기화
	m_IsNextAttackInput = false;

	if (IsEscapePossible)
	{
		// 점프키 => 내부 우선순위 높음. (입력 보다) ex) w space 동시에 눌렀으면? => space 먼저 판별.
		if (m_States[JUMP])
		{
			m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_WALK_LF;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
			return;
		}

		// 입력키
		if (m_States[MOVE])
		{
			// Escape가 너무 빠르게 동작함. => 공격 전환 TrackPoistion과 이동 전환 TrackPosition이 달라야할듯?
			if ((eAttackType == EGalbrenaHeavyAttackType::ATTACK_H_0201) && m_fTrackPosition < 30.f)
				return;

			m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
			return;
		}

	}
    

	// 2. 애니메이션 탈출 조건인 경우.
	if (IsEscapePossible)
	{
		// 점프키 => 내부 우선순위 높음. (입력 보다) ex) w space 동시에 눌렀으면? => space 먼저 판별.
		if (m_States[JUMP])
		{
			m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_WALK_LF;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP));
			return;
		}

		// 입력키

		if (m_States[MOVE])
		{
			// Escape가 너무 빠르게 동작함. => 공격 전환 TrackPoistion과 이동 전환 TrackPosition이 달라야할듯?
			if ((eAttackType == EGalbrenaHeavyAttackType::ATTACK_H_0201) && m_fTrackPosition < 30.f)
			{
				return;
			}

			m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
			return;
		}

	}

	//공격 애니메이션 끝나고 추가 입력 없으면 Idle로 => 가장 우선순위 낮음.
	if (m_IsAnimationEnd)
	{
		m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STANDCHANGE;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
		m_IsNextAttackInput = false;
		return;
	}

    
}

void CGalbrenaGroundHeavyAttack::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHeavyAttackType::ATTACK_H_0201),"Attack_H_0201", 1.5f, 15.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHeavyAttackType::ATTACK_H_0202),"Attack_H_0202", 1.5f, 8.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaHeavyAttackType::ATTACK_H_0203),"Attack_H_0203", 1.5f, 30.f); // 이때 공격하면 전환

}

void CGalbrenaGroundHeavyAttack::State_Reset()
{
    for (_uint i = 0; i < ATTACKSTATE::END; ++i)
        m_States[i] = false;
}

CGalbrenaGroundHeavyAttack* CGalbrenaGroundHeavyAttack::Create(CCharacter* pOwner)
{
    CGalbrenaGroundHeavyAttack* pInstance = new CGalbrenaGroundHeavyAttack();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaGroundHeavyAttack");
    }

    return pInstance;
}

void CGalbrenaGroundHeavyAttack::Free()
{
    CGroundState::Free();
}
