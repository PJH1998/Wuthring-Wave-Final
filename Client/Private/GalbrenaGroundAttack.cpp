#include "ClientPch.h"
#include "GalbrenaGroundAttack.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaShotGun.h"
#include "Ability.h"

HRESULT CGalbrenaGroundAttack::Initialize(CCharacter* pCharacter)
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


void CGalbrenaGroundAttack::OnEnter(void* pArg)
{
	CGroundState::OnEnter(pArg);

	// 1. 복사본 Context 받아오기
	const auto context = m_pGalbrena->TakeStateContext();

	// 2. 복사본에서 필요한 값 읽기
	EGalbrenaAttackType eAttackType = context.m_eAttackType;

	// 3. 애니메이션 세팅.
	m_iCurrentAnimIdx = ENUM_CLASS(eAttackType);

	// 4. Attack 상태 초기화
	State_Reset();

	
	m_ActivePartTypes.clear(); // 파츠 목록 초기화

	// 5. 무기 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음(Attack은)
	switch (eAttackType)
	{
	case EGalbrenaAttackType::ATTACK01:
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_FIRSTGUN);
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_SECONDGUN);
		break;
	case EGalbrenaAttackType::ATTACK02:
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_FIRSTGUN);
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_SECONDGUN);
		break;
	case EGalbrenaAttackType::ATTACK03:
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_FIRSTGUN);
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_SECONDGUN);
		break;
	case EGalbrenaAttackType::ATTACK04:
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_FIRSTGUN);
		break;
	}

	// 6. 사용하는 PartType이 있다면?
	for (auto& PartType : m_ActivePartTypes)
		m_pGalbrena->PartActivate(PartType, true);

	// 7. 중력 활성화 및 Target 회전.
	m_pGalbrena->Set_Gravity(true);
	m_pGalbrena->Rotate_Target();
}

void CGalbrenaGroundAttack::OnUpdate(_float fTimeDelta)
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

void CGalbrenaGroundAttack::OnExit()
{
    CGroundState::OnExit();

	// Part Activate 비활성화
	for (auto& PartType : m_ActivePartTypes)
		m_pGalbrena->PartActivate(PartType, false);

    // 콤보 카운트 초기화
    m_iComboCount = 0;
    m_fAttackPressTime = 0.f; // 시간 초기화
    m_pGalbrena->PartActivate(m_iPartType, false); 
    m_pGalbrena->PartActivate(m_iSubPartType, false); 

	m_pGalbrena->Collider_Active(TEXT("Main|X|X"), false);

	m_pGalbrena->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::HIT));
}

_bool CGalbrenaGroundAttack::Hit_Judge()
{
	_bool IsHit = false;
	const CCharacter::HIT_DESC* pDesc = m_pGalbrena->GetPendingHitDesc();

	if (nullptr == pDesc)
		return false;

	// Hit 상태이면서 Enemy Skill을 받았을때만 캔슬하고 Hit로
	if (!m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT)))
		return false;


	COLLISIONLAYER eLayer = static_cast<COLLISIONLAYER>(m_pGalbrena->GetPendingHitDesc()->iLayer);
	if (eLayer == COLLISIONLAYER::ENEMY_SKILL)
		IsHit = true;

	return IsHit;
}

void CGalbrenaGroundAttack::Handle_Input()
{
    EGalbrenaAttackType eAttackType = static_cast<EGalbrenaAttackType>(m_iCurrentAnimIdx);

	m_States[HIT_PENDING] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT));

    // HEAVY_ATTACK_PENDING(강공 발생 조건)
    // Attack이 01이고 키를 애니메이션 탈출 가능 상태까지 계속 누르고 있다면?
	m_States[HEAVY_ATTACK_PENDING] = (eAttackType == EGalbrenaAttackType::ATTACK01)
		&& (m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB), KEYSTATE::PRESS));

	m_States[HEAVY_ATTACK] = m_States[HEAVY_ATTACK_PENDING] && (m_fAttackPressTime >= m_fAttackPressMaxTime);

    // 입력키 체크
    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
	m_States[DASH] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));
    m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));

    // 스킬 체크
    m_States[SKILL_Q] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::Q));
    m_States[SKILL_E] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));
    m_States[SKILL_R] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::R));

	m_States[ATTACK] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

	// Cost 계속
	m_States[BURST] = (m_pGalbrena->Get_Cost(COST_TYPE::COST1) >= m_pGalbrena->Get_MaxCost());
	

    if (eAttackType >= EGalbrenaAttackType::ATTACK01 && eAttackType < EGalbrenaAttackType::ATTACK04)
    {
        if (m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB), KEYSTATE::PRESS))
            m_IsNextAttackInput = true;
    }

	// 공격시에 Hit 받았을때는 좀더 판단을 빡빡하게
	if (m_pGalbrena->Is_Hit())
		m_States[HIT] = Hit_Judge();
	
    
}

void CGalbrenaGroundAttack::Update_AttackAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pGalbrena->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; // 거리 계산에 따른 Animation Scale 조절.

    // 1. 현재 애니메이션 재생
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta, m_fAnimationScale);

    // Target이 존재한다면? => Auto Target
    m_pGalbrena->Rotate_Target();

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

void CGalbrenaGroundAttack::Check_Physics(_float fTimeDelta)
{

}

void CGalbrenaGroundAttack::LockOn_StateTransition(_float fTimeDelta)
{
}

void CGalbrenaGroundAttack::Check_StateTransition(_float fTimeDelta)
{
    // 1. 스킬 입력 (E, R 등) 들어오면 Skill로 => 우선순위 별.
    // ... 추후 구현
    // 2. Normal Attack의 경우 콤보 공격이 가능하게.
    
    EGalbrenaAttackType eAttackType = static_cast<EGalbrenaAttackType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();
    // 우선순위 순서대로
	if (m_States[HIT])
	{
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EGalbrenaHitState::HIT));
		return;
	}

	//if (m_States[DASH]) 
	//{
	//	m_pGalbrena->GetStateContextForWrite().m_eDashType = EGalbrenaDashType::MOVE_F;
	//	m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::DASH)); // 상위, 하위 상태
	//	return;
	//}

	if (m_States[DASH])
	{
		m_pGalbrena->GetStateContextForWrite().m_eSpecialDashType = EGalbrenaSpecialDashType::ATTACK_CHARGE;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::SPECIALDASH));
		return;
	}

	// 강공 조건이 되었다면?
	if (m_States[HEAVY_ATTACK] && IsEscapePossible)
	{
		m_pGalbrena->GetStateContextForWrite().m_eHeavyAttackType = EGalbrenaHeavyAttackType::ATTACK_H_0201;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::HEAVYATTACK));
		return;
	}

	// 1. 기본 공상태에서 Heavy_Attack_Pending이 아닌 경우?
	if (eAttackType >= EGalbrenaAttackType::ATTACK01 && eAttackType < EGalbrenaAttackType::ATTACK04)
	{
		// 키 누르고 있다면 다른 상태전환하지 말고 계속 Attack01 실행. => 강공을 위해.
		if (eAttackType == EGalbrenaAttackType::ATTACK01 && m_States[HEAVY_ATTACK_PENDING])
		{
			return;
		}

		if (m_IsNextAttackInput && IsEscapePossible)
		{
			switch (eAttackType)
			{
			case EGalbrenaAttackType::ATTACK01:
				m_pGalbrena->GetStateContextForWrite().m_eAttackType = EGalbrenaAttackType::ATTACK02;
				break;
			case EGalbrenaAttackType::ATTACK02:
				m_pGalbrena->GetStateContextForWrite().m_eAttackType = EGalbrenaAttackType::ATTACK03;
				break;
			case EGalbrenaAttackType::ATTACK03:
				m_pGalbrena->GetStateContextForWrite().m_eAttackType = EGalbrenaAttackType::ATTACK04;
				break;
			}

			m_IsNextAttackInput = false;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::ATTACK));
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
			if ((eAttackType == EGalbrenaAttackType::ATTACK01) && m_fTrackPosition < 30.f)
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
			if ((eAttackType == EGalbrenaAttackType::ATTACK01) && m_fTrackPosition < 30.f)
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
		m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND2;
		m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
		m_IsNextAttackInput = false;
		return;
	}

    
}

void CGalbrenaGroundAttack::SetUp_Animations()
{
    
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAttackType::ATTACK01),"Attack01", 1.f, 9.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAttackType::ATTACK02),"Attack02", 1.f, 25.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAttackType::ATTACK03),"Attack03", 1.f, 25.f); // 이때 공격하면 전환
    CState::Add_Animations(ENUM_CLASS(EGalbrenaAttackType::ATTACK04),"Attack04", 1.f, 10.f);
}

void CGalbrenaGroundAttack::State_Reset()
{
    for (_uint i = 0; i < ATTACKSTATE::END; ++i)
        m_States[i] = false;
}

CGalbrenaGroundAttack* CGalbrenaGroundAttack::Create(CCharacter* pOwner)
{
    CGalbrenaGroundAttack* pInstance = new CGalbrenaGroundAttack();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaGroundAttack");
    }

    return pInstance;
}

void CGalbrenaGroundAttack::Free()
{
    CGroundState::Free();
}
