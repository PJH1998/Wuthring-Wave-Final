#include "ClientPch.h"
#include "AugustaGroundAttack.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaBayonet.h"
#include "AbilityDefine.h"


using namespace AbilityConst::SkillNames;
HRESULT CAugustaGroundAttack::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pCharacter);
    ASSERT_CRASH(m_pAugusta);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    return S_OK;
}


void CAugustaGroundAttack::OnEnter(void* pArg)
{
	CGroundState::OnEnter(pArg);

	const auto context = m_pAugusta->TakeStateContext();

	EAugustaAttackType eAttackType = context.m_eAttackType;

	m_iCurrentAnimIdx = ENUM_CLASS(eAttackType);

	State_Reset();


	m_iPartType = CAugusta::PARTTYPE::PART_BAYONET; 
	m_iSubPartType = CAugusta::PARTTYPE::PART_HEADPROP;

	_string strBoneName = "WeaponProp02";
	m_pAugusta->PartActivate(m_iPartType, true);
	m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations.at(m_iCurrentAnimIdx).strAnimName);
	m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
	m_pAugusta->Set_Gravity(true);


	m_pAugusta->PartActivate(m_iSubPartType, true);
	m_pAugusta->Clear_PartAnimation(m_iSubPartType, m_Animations.at(m_iCurrentAnimIdx).strAnimName);
	m_pAugusta->Set_SocketMatrixToParts(m_iSubPartType, "Bone_Hair001_M");
	m_pAugusta->Part_ShaderPathChange(m_iSubPartType, ENUM_CLASS(SHADER_PROPANIMMESH::DEFAULT_WEAPON));

	m_pAugusta->Rotate_Target();

}

void CAugustaGroundAttack::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    Handle_Input();
    Update_AttackAnimations(fTimeDelta);
    Check_Physics(fTimeDelta);
    Check_StateTransition(fTimeDelta);
    State_Reset();
}

void CAugustaGroundAttack::OnExit()
{
    CGroundState::OnExit();

    m_iComboCount = 0;
    m_fAttackPressTime = 0.f; 
    m_pAugusta->PartActivate(m_iPartType, false); 
    m_pAugusta->PartActivate(m_iSubPartType, false); 
	m_pAugusta->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::HIT));
	m_pAugusta->Collider_Active(TEXT("Main|X|X"), false);
}

_bool CAugustaGroundAttack::Hit_Judge()
{
	if (m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE)))
		return false;

	_bool IsHit = false;
	const CCharacter::HIT_DESC* pDesc = m_pAugusta->GetPendingHitDesc();

	if (nullptr == pDesc)
		return false;

	// Hit 상태이면서 Enemy Skill을 받았을때만 캔슬하고 Hit로
	if (!m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT)))
		return false;

	COLLISIONLAYER eLayer = static_cast<COLLISIONLAYER>(m_pAugusta->GetPendingHitDesc()->iLayer);
	if (eLayer == COLLISIONLAYER::ENEMY_SKILL)
		IsHit = true;

	return IsHit;
}

void CAugustaGroundAttack::Handle_Input()
{
    EAugustaAttackType eAttackType = static_cast<EAugustaAttackType>(m_iCurrentAnimIdx);

	// Dash 도중에 Hit 당하면 Dodge
	m_States[DASH] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));

	m_States[HIT_PENDING] = m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT));
	m_States[HIT] = Hit_Judge(); // Hit_judge()가 True인 경우.

    // HEAVY_ATTACK_PENDING(강공 발생 조건)
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

	m_States[NORMAL_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pAugusta->Check_Skill(AugustaDefaultE)); // 기본 E스킬
	m_States[POINT_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pAugusta->Check_Skill(AugustaGriffonE));// 그리폰

	m_States[SWORD_R] = m_States[SKILL_R] && (SKILL_STATE::READY == m_pAugusta->Check_Skill(AugustaBurstR)); // 궁극기 R스킬(검뽑는거)
	m_States[ULTI] = m_States[SKILL_R] && (SKILL_STATE::READY == m_pAugusta->Check_Skill(AugustaDefaultR)); // Echo 궁극기. (기본 궁극기)
	
}

void CAugustaGroundAttack::Update_AttackAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pAugusta->Calculate_RootMotionScale();
	// 거리 계산에 따른 Animation Scale 조절.
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; 

    // 1. 현재 애니메이션 재생
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta, m_fAnimationScale);


    // 1타 모션일때 누르고 있다면?
    if (m_States[HEAVY_ATTACK_PENDING])
        m_fAttackPressTime += fTimeDelta;
	
    // Attack State에 해당하는 경우 모두 Animation이 존재.
	m_pAugusta->Play_PartAnimation(
		m_iPartType,
		m_Animations.at(m_iCurrentAnimIdx).strAnimName,
		fTimeDelta, nullptr
	);

	// HeadProp은 계속실행.
	m_pAugusta->Play_PartAnimation(
		CAugusta::PARTTYPE::PART_HEADPROP,
		"Stand1_idle",
		m_Animations.at(m_iCurrentAnimIdx).fSpeed * fTimeDelta, nullptr
	);
}

void CAugustaGroundAttack::Check_Physics(_float fTimeDelta)
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
    
	if (m_States[HIT])
	{
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EAugustaHitState::HIT));
		return;
	}

	// 1. 우선순위 => 어떤 상황에도 변경 가능.
	if (m_States[DASH])
	{
		// 누른 방향으로 회전 하고 변경.
		m_eDir = m_pAugusta->Calculate_Direction();
		_vector vMoveDir = m_pAugusta->Calculate_Move_Direction(m_eDir);
		m_pAugusta->Rotate_Direction(vMoveDir);

		m_pAugusta->GetStateContextForWrite().m_eDashType = EAugustaDashType::MOVE_F;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::DASH)); // 상위, 하위 상태
		return;
	}


	// 구현. => Burst 게이지 모두 찼을때 궁 누르면 공격기 모션.
	if (m_States[SWORD_R])
	{
		// 위에 서체크하긴 했지만? 다시 체크.
		if (SKILL_STATE::READY != m_pAugusta->Use_Skill(AugustaBurstR))
			return;

		m_pAugusta->Bind_Flag_ToAbillity(ENUM_CLASS(UI_AUGUSTA_VIEWFLAG::LB_SP_ATTACK));
		m_pAugusta->GetStateContextForWrite().m_eBurstType = EAugustaBurstType::BURST01;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::BURST)); // 상위, 하위 상태
		return;
	}
	if (m_States[ULTI])
	{
		if (SKILL_STATE::READY != m_pAugusta->Use_Skill(AugustaDefaultR))
			return;

		m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::ATTACK_SPEEDDRIVE;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL)); // 상위, 하위 상태
		return;
	}

	if (m_States[POINT_E])
	{
		// 위에 서체크하긴 했지만? 다시 체크.
		if (SKILL_STATE::READY != m_pAugusta->Use_Skill(AugustaGriffonE))
			return;

		//m_pAugusta->Bind_Flag_ToAbillity(ENUM_CLASS(UI_AUGUSTA_VIEWFLAG::E_GRIFFON));

		m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::SKILL_STRIKE;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL));
		return;
	}    // SKILL_E 누르면 => 

	if (m_States[NORMAL_E])
	{
		if (SKILL_STATE::READY != m_pAugusta->Use_Skill(AugustaGriffonE))
			return;


		m_pAugusta->GetStateContextForWrite().m_eSkillType = EAugustaSkillType::SKILL_HACK;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SKILL));
		return;
	}

    // 0. 1타모션에서 계속 누르고 임계시간을 넘으면?
    if (m_States[HEAVY_ATTACK_PENDING] && (m_fAttackPressTime >= m_fAttackPressMaxTime) && IsEscapePossible)
    {
		if (SKILL_STATE::READY == m_pAugusta->Use_Skill(AugustaChargeE))
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAttackType::ATTACK_HEAVYHACK);
			m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations.at(m_iCurrentAnimIdx).strAnimName);
			m_fAttackPressTime = 0.f;
			return;
		}
    }

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
			m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations.at(m_iCurrentAnimIdx).strAnimName); // 애니메이션 초기화
            m_iCurrentAnimIdx = ENUM_CLASS(EAugustaAttackType::ATTACK01) + m_iComboCount;
            m_IsNextAttackInput = false;
            m_fAttackPressTime = 0.f; // Attack02나 03으로 전환되므로 PressTime 초기화
			m_pAugusta->Rotate_Target();
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
        m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STANDCHANGE;
        m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
        m_IsNextAttackInput = false;
        return;
    }
 
}

void CAugustaGroundAttack::SetUp_Animations()
{
    
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK01),"Attack01", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK02),"Attack02", 1.f, 21.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK03),"Attack03", 1.f, 31.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK04),"Attack04", 1.f, 10.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK_HEAVYHACK),"Attack_HeavyHack", 1.f, 30.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK_PULL), "Attack_Pull", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK_SPEEDDRIVE),"Attack_SpeedDrive", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaAttackType::ATTACK_SPSKILL),"Attack_SpSkill", 1.f, 0.f);
}

void CAugustaGroundAttack::State_Reset()
{
    for (_uint i = 0; i < ATTACKSTATE::END; ++i)
        m_States[i] = false;
}

CAugustaGroundAttack* CAugustaGroundAttack::Create(CCharacter* pOwner)
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
