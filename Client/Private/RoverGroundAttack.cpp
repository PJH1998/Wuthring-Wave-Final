#include "ClientPch.h"
#include "RoverGroundAttack.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverSword.h"
#include "Ability.h"

HRESULT CRoverGroundAttack::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pOwner);
    ASSERT_CRASH(m_pRover);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    return S_OK;
}


void CRoverGroundAttack::OnEnter(void* pArg)
{
	CGroundState::OnEnter(pArg);

	// 1. 복사본 Context 받아오기
	const auto context = m_pRover->TakeStateContext();

	// 2. 복사본에서 필요한 값 읽기
	ERoverAttackType eAttackType = context.m_eAttackType;

	// 3. 애니메이션 세팅.
	m_iCurrentAnimIdx = ENUM_CLASS(eAttackType);

	// 4. Attack 상태 초기화
	State_Reset();


	// 5. 무기 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음(Attack은)
	m_iPartType = CRover::PARTTYPE::PART_SWORD; // 추후 애니메이션에 따른. 분기문 필요.

	_string strBoneName = "WeaponProp02";
	m_pRover->PartActivate(m_iPartType, true); // 파츠 변경. // Volume Activate는 Notify로..
	m_pRover->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
	m_pRover->Set_SocketMatrixToParts(m_iPartType, strBoneName);
	m_pRover->Set_Gravity(true);

	m_pRover->Rotate_Target();
}

void CRoverGroundAttack::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 입력 확인
    Handle_Input();

    // 1. Attack 업데이트
    Update_AttackAnimations(fTimeDelta);

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    Check_StateTransition(fTimeDelta);

    State_Reset();
}

void CRoverGroundAttack::OnExit()
{
    CGroundState::OnExit();

    // 콤보 카운트 초기화
    m_iComboCount = 0;
    m_fAttackPressTime = 0.f; // 시간 초기화
    m_pRover->PartActivate(m_iPartType, false); 
}

_bool CRoverGroundAttack::Hit_Judge()
{
	_bool IsHit = false;
	const CCharacter::HIT_DESC* pDesc = m_pRover->GetPendingHitDesc();

	if (nullptr == pDesc)
		return false;

	COLLISIONLAYER eLayer = static_cast<COLLISIONLAYER>(m_pRover->GetPendingHitDesc()->iLayer);
	if (eLayer == COLLISIONLAYER::ENEMY_SKILL)
		IsHit = true;

	return IsHit;
}

void CRoverGroundAttack::Handle_Input()
{
    ERoverAttackType eAttackType = static_cast<ERoverAttackType>(m_iCurrentAnimIdx);

	m_States[HIT_PENDING] = m_pRover->Check_AnyCondition(CHARACTER_CONDITION::HIT);

    // HEAVY_ATTACK_PENDING(강공 발생 조건)
    // Attack이 01이고 키를 애니메이션 탈출 가능 상태까지 계속 누르고 있다면?
	m_States[HEAVY_ATTACK_PENDING] = (eAttackType == ERoverAttackType::ATTACK01)
		&& (m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB), KEYSTATE::PRESS));

    // 입력키 체크
    m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
    m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));

    // 스킬 체크
    m_States[SKILL_Q] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::Q));
    m_States[SKILL_E] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));
    m_States[SKILL_R] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::R));

	m_States[ATTACK] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));

	// Cost 계속
	m_States[BURST] = (m_pRover->Get_Cost(COST_TYPE::COST1) >= m_pRover->Get_MaxCost());
	

    if (eAttackType >= ERoverAttackType::ATTACK01 && eAttackType < ERoverAttackType::ATTACK04) // 05가 마지막이 아니라 04가 마지막
    {
        if (m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB), KEYSTATE::PRESS))
            m_IsNextAttackInput = true;
    }

	// 공격시에 Hit 받았을때는 좀더 판단을 빡빡하게
	if (m_pRover->Is_Hit())
	{
		m_States[HIT] = Hit_Judge();
	}
	
    
}

void CRoverGroundAttack::Update_AttackAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pRover->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations[m_iCurrentAnimIdx].fRootMotionRate * m_fRootMotionScale; // 거리 계산에 따른 Animation Scale 조절.

    // 1. 현재 애니메이션 재생
    CCharacterState::Play_Animation(m_pRover, fTimeDelta, m_fAnimationScale);

    // Target이 존재한다면? => Auto Target
    m_pRover->Rotate_Target();

    // 1타 모션일때 누르고 있다면?
    if (m_States[HEAVY_ATTACK_PENDING])
        m_fAttackPressTime += fTimeDelta;

    // Attack State에 해당하는 경우 모두 Animation이 존재.
    m_pRover->Play_PartAnimation(
        m_iPartType,
        m_Animations[m_iCurrentAnimIdx].strAnimName,
        fTimeDelta, nullptr
    );
}

void CRoverGroundAttack::Check_Physics(_float fTimeDelta)
{

}

void CRoverGroundAttack::LockOn_StateTransition(_float fTimeDelta)
{
}

void CRoverGroundAttack::Check_StateTransition(_float fTimeDelta)
{
    // 1. 스킬 입력 (E, R 등) 들어오면 Skill로 => 우선순위 별.
    // ... 추후 구현
    // 2. Normal Attack의 경우 콤보 공격이 가능하게.
    
    ERoverAttackType eAttackType = static_cast<ERoverAttackType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();
    // 우선순위 순서대로
    
	if (m_States[HIT])
	{

	}

	
	if (m_States[HEAVY_ATTACK_PENDING])
	{
		if ((m_fAttackPressTime >= m_fAttackPressMaxTime) && IsEscapePossible)
		{
			if (m_States[BURST]) // 강공 게이지 가득 차있으면? 
			{
				// Ability 동기화
				m_pRover->GetStateContextForWrite().m_eBurstType = ERoverBurstType::BURST01;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::BURST));
				return;
			}
			else // 아니면? => 기본 강공.
			{

			}
		}

	}

	// 1. 기본 공상태에서 Heavy_Attack_Pending이 아닌 경우?
	if (eAttackType >= ERoverAttackType::ATTACK01 && eAttackType < ERoverAttackType::ATTACK04)
	{
		// 키 누르고 있다면 다른 상태전환하지 말고 계속 Attack01 실행. => 강공을 위해.
		if (eAttackType == ERoverAttackType::ATTACK01 && m_States[HEAVY_ATTACK_PENDING])
		{
			return;
		}

		if (m_IsNextAttackInput && IsEscapePossible)
		{
			m_iComboCount++;
			m_pRover->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName); // 애니메이션 초기화
			m_iCurrentAnimIdx = ENUM_CLASS(ERoverAttackType::ATTACK01) + m_iComboCount;

			
			m_IsNextAttackInput = false;
			m_fAttackPressTime = 0.f; // Attack02나 03으로 전환되므로 PressTime 초기화
			m_pRover->Rotate_Target();
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
			m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_WALK_LF;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP));
			return;
		}

		// 입력키
		if (m_States[MOVE])
		{
			// Escape가 너무 빠르게 동작함. => 공격 전환 TrackPoistion과 이동 전환 TrackPosition이 달라야할듯?
			if ((eAttackType == ERoverAttackType::ATTACK01) && m_fTrackPosition < 30.f)
				return;
			m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
			return;
		}

	}
    

	// 2. 애니메이션 탈출 조건인 경우.
	if (IsEscapePossible)
	{
		// 점프키 => 내부 우선순위 높음. (입력 보다) ex) w space 동시에 눌렀으면? => space 먼저 판별.
		if (m_States[JUMP])
		{
			m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_WALK_LF;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::JUMP));
			return;
		}

		// 입력키

		if (m_States[MOVE])
		{
			// Escape가 너무 빠르게 동작함. => 공격 전환 TrackPoistion과 이동 전환 TrackPosition이 달라야할듯?
			if ((eAttackType == ERoverAttackType::ATTACK01) && m_fTrackPosition < 30.f)
			{
				return;
			}

			m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
			m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
			return;
		}

	}

	//공격 애니메이션 끝나고 추가 입력 없으면 Idle로 => 가장 우선순위 낮음.
	if (m_IsAnimationEnd)
	{
		m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
		m_IsNextAttackInput = false;
		return;
	}

    
}

void CRoverGroundAttack::SetUp_Animations()
{
    
    CState::Add_Animations(ENUM_CLASS(ERoverAttackType::ATTACK01),"Attack01", 1.3f, 10.f);
    CState::Add_Animations(ENUM_CLASS(ERoverAttackType::ATTACK02),"Attack02", 1.3f, 30.f);
    CState::Add_Animations(ENUM_CLASS(ERoverAttackType::ATTACK03),"Attack03", 1.3f, 20.f); // 이때 공격하면 전환
    CState::Add_Animations(ENUM_CLASS(ERoverAttackType::ATTACK04),"Attack04", 1.3f, 10.f);
	CState::Add_Animations(ENUM_CLASS(ERoverAttackType::ATTACK05), "Attack05", 1.f, 19.f); // 이때 공격하면 전환.
    
}

void CRoverGroundAttack::State_Reset()
{
    for (_uint i = 0; i < ATTACKSTATE::END; ++i)
        m_States[i] = false;
}

CRoverGroundAttack* CRoverGroundAttack::Create(class CGameObject* pOwner)
{
    CRoverGroundAttack* pInstance = new CRoverGroundAttack();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverGroundAttack");
    }

    return pInstance;
}

void CRoverGroundAttack::Free()
{
    CGroundState::Free();
}
