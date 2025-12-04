#include "ClientPch.h"
#include "SequenceLupaGroundSkill.h"
#include "SequenceLupa.h"
#include "StateMachine.h"

HRESULT CSequenceLupaGroundSkill::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pSequenceLupa = dynamic_cast<CSequenceLupa*>(pCharacter);
    ASSERT_CRASH(m_pSequenceLupa);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

	// 매핑.

    return S_OK;
}


void CSequenceLupaGroundSkill::OnEnter(void* pArg)
{
	CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pSequenceLupa->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ESequenceLupaSkillType eAirAttackType = context.m_eSkillType;
	m_strPrevInfo = context.m_strPrevInfo; // 복사본에서 받은 정보.

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eAirAttackType);

    // 4. Attack 상태 초기화
    State_Reset();

    // 5. 무기 상태 Activate => 현재 애니메이션 상태에 따라 Parts가 달라질 수 있음(Attack은)
    m_iPartType = CSequenceLupa::PARTTYPE::PART_SPEAR; // 추후 애니메이션에 따른. 분기문 필요.
	m_pSequenceLupa->PartActivate(CSequenceLupa::PARTTYPE::PART_SPEAR, true);

	m_pSequenceLupa->Set_Gravity(true);
	m_pSequenceLupa->Rotate_TargetPosition(); // 처음 공격한 곳을 타겟으로 공격.
}

void CSequenceLupaGroundSkill::OnUpdate(_float fTimeDelta)
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

void CSequenceLupaGroundSkill::OnExit()
{
	CGroundState::OnExit();

	if (m_iPartType != CSequenceLupa::PARTTYPE::TYPE_END)
		m_pSequenceLupa->PartActivate(m_iPartType, false);

    m_pSequenceLupa->Set_Gravity(true);
    m_fSpeed = 0.f;

	// 공격 콜라이더 비활성화
	m_pSequenceLupa->Collider_Active(TEXT("Main|X|X"), false);
}

void CSequenceLupaGroundSkill::Handle_Input()
{
}

void CSequenceLupaGroundSkill::Update_AttackAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선) // 거리 계산에 따른 Animation Scale 조절.
	m_fRootMotionScale = 0.5f;
	m_fAnimationScale = m_Animations[m_iCurrentAnimIdx].fRootMotionRate * m_fRootMotionScale;

	// 1. 애니메이션 실행.
    CCharacterState::Play_Animation_NonFacial(m_pSequenceLupa, fTimeDelta, m_fAnimationScale);

	m_pSequenceLupa->Play_PartAnimation(
		m_iPartType,
		m_Animations.at(m_iCurrentAnimIdx).strAnimName,
		fTimeDelta * m_Animations.at(m_iCurrentAnimIdx).fSpeed, nullptr, 1.f, true, false
	);
    
}

void CSequenceLupaGroundSkill::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pSequenceLupa->Is_LandCollider(&m_vLandNormal);
}

void CSequenceLupaGroundSkill::Check_StateTransition(_float fTimeDelta)
{
    ESequenceLupaSkillType eSkillType = static_cast<ESequenceLupaSkillType>(m_iCurrentAnimIdx);
    _bool IsEscapePossible = CState::Is_EscapePossible();
    _float fOffsetY = 0.1f;

	// 애니메이션이 끝나면?
    if (IsEscapePossible)
    {
		if (eSkillType == ESequenceLupaSkillType::SKILL02_SP)
		{
			OnExit();
			m_pSequenceLupa->Activate(false);
			return;
		}
      
    }
	
}

void CSequenceLupaGroundSkill::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ESequenceLupaSkillType::SKILL02_SP),"Skill02_Sp", 1.f, 150.f, 1.f);
}

void CSequenceLupaGroundSkill::State_Reset()
{
    for (_uint i = 0; i < AIRATTACKSTATE::END; ++i)
        m_States[i] = false;
}

// 예외적인 애니메이션에 관련해서는 RootMotion Scale을 조절합니다. => 최소 수치를 보장한다?
void CSequenceLupaGroundSkill::Handle_Animation_SpecialState()
{
	
}



CSequenceLupaGroundSkill* CSequenceLupaGroundSkill::Create(CCharacter* pOwner)
{
    CSequenceLupaGroundSkill* pInstance = new CSequenceLupaGroundSkill();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CSequenceLupaGroundSkill");
    }

    return pInstance;
}

void CSequenceLupaGroundSkill::Free()
{
    CGroundState::Free();
}
