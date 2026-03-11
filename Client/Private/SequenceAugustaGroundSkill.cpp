#include "ClientPch.h"
#include "SequenceAugustaGroundSkill.h"
#include "SequenceAugusta.h"
#include "StateMachine.h"

HRESULT CSequenceAugustaGroundSkill::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pSequenceAugusta = dynamic_cast<CSequenceAugusta*>(pCharacter);
    ASSERT_CRASH(m_pSequenceAugusta);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

	// 매핑.

    return S_OK;
}


void CSequenceAugustaGroundSkill::OnEnter(void* pArg)
{
	CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pSequenceAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ESequenceAugustaSkillType eAirAttackType = context.m_eSkillType;
	m_strPrevInfo = context.m_strPrevInfo; // 복사본에서 받은 정보.

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eAirAttackType);

    // 4. Attack 상태 초기화
    State_Reset();

    // 5. 무기 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음(Attack은)
    m_iPartType = CSequenceAugusta::PARTTYPE::PART_BAYONET; // 추후 애니메이션에 따른. 분기문 필요.
	m_pSequenceAugusta->PartActivate(CSequenceAugusta::PARTTYPE::PART_BAYONET, true);

    // 6. 무기에 Bone 붙이기. + Offset 추가.
    _string strBoneName = "";
  
	m_pSequenceAugusta->Set_Gravity(true);
	m_pSequenceAugusta->Rotate_TargetPosition(); // 처음 공격한 곳을 타겟으로 공격.
}

void CSequenceAugustaGroundSkill::OnUpdate(_float fTimeDelta)
{
	CGroundState::OnUpdate(fTimeDelta);

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

void CSequenceAugustaGroundSkill::OnExit()
{
	CGroundState::OnExit();

	if (m_iPartType != CSequenceAugusta::PARTTYPE::TYPE_END)
		m_pSequenceAugusta->PartActivate(m_iPartType, false);

    m_pSequenceAugusta->Set_Gravity(true);
    m_fSpeed = 0.f;

	// 공격 콜라이더 비활성화
	m_pSequenceAugusta->Collider_Active(TEXT("Main|X|X"), false);
}

void CSequenceAugustaGroundSkill::Handle_Input()
{
}

void CSequenceAugustaGroundSkill::Update_AttackAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선) // 거리 계산에 따른 Animation Scale 조절.
	m_fRootMotionScale = 0.3f;
	m_fAnimationScale = m_Animations[m_iCurrentAnimIdx].fRootMotionRate * m_fRootMotionScale;

	// 1. 애니메이션 실행.
    CCharacterState::Play_Animation(m_pSequenceAugusta, fTimeDelta, m_fAnimationScale, false);

	m_pSequenceAugusta->Play_PartAnimation(
		m_iPartType,
		m_Animations.at(m_iCurrentAnimIdx).strAnimName,
		fTimeDelta * m_Animations.at(m_iCurrentAnimIdx).fSpeed, nullptr, 1.f, true, false
	);

	ESequenceAugustaSkillType eSkillType = static_cast<ESequenceAugustaSkillType>(m_iCurrentAnimIdx);
    _vector vLook = m_pSequenceAugusta->Get_LookVector();
    
}

void CSequenceAugustaGroundSkill::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pSequenceAugusta->Is_LandCollider(&m_vLandNormal);
}

void CSequenceAugustaGroundSkill::Check_StateTransition(_float fTimeDelta)
{
    ESequenceAugustaSkillType eSkillType = static_cast<ESequenceAugustaSkillType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();
    _float fOffsetY = 0.1f;

	// 애니메이션이 끝나면?
    if (IsEscapePossible)
    {
		if (eSkillType == ESequenceAugustaSkillType::ATTACK_SPEEDDRIVE)
		{
			m_pSequenceAugusta->GetStateContextForWrite().m_eSkillType = ESequenceAugustaSkillType::ATTACK_PULL;
			m_pSequenceAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ESequenceAugustaGroundState::SKILL));
			return;
		}
        
		if (eSkillType == ESequenceAugustaSkillType::ATTACK_PULL)
		{
			m_pSequenceAugusta->GetStateContextForWrite().m_eSkillType = ESequenceAugustaSkillType::ATTACK_SPSKILL;
			m_pSequenceAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ESequenceAugustaGroundState::SKILL));
			return;
		}
    }


	if (m_IsAnimationEnd)
	{
		if (eSkillType == ESequenceAugustaSkillType::ATTACK_SPSKILL)
		{
			OnExit();
			m_pSequenceAugusta->Activate(false);
		}
	}
}

void CSequenceAugustaGroundSkill::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ESequenceAugustaSkillType::ATTACK_SPEEDDRIVE),"Attack_SpeedDrive", 1.f, 30.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(ESequenceAugustaSkillType::ATTACK_PULL) ,"Attack_Pull", 1.f, 30.f, 1.f);
    CState::Add_Animations(ENUM_CLASS(ESequenceAugustaSkillType::ATTACK_SPSKILL),"Attack_SpSkill", 1.f, 60.f, 1.f);
}

void CSequenceAugustaGroundSkill::State_Reset()
{
    for (_uint i = 0; i < AIRATTACKSTATE::END; ++i)
        m_States[i] = false;
}

// 예외적인 애니메이션에 관련해서는 RootMotion Scale을 조절합니다. => 최소 수치를 보장한다?
void CSequenceAugustaGroundSkill::Handle_Animation_SpecialState()
{
	
}



CSequenceAugustaGroundSkill* CSequenceAugustaGroundSkill::Create(CCharacter* pOwner)
{
    CSequenceAugustaGroundSkill* pInstance = new CSequenceAugustaGroundSkill();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CSequenceAugustaGroundSkill");
    }

    return pInstance;
}

void CSequenceAugustaGroundSkill::Free()
{
    CGroundState::Free();
}
