#include "ClientPch.h"
#include "GalbrenaGroundIdle.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "Ability.h"
#include "GalbrenaState_Enum.h"


HRESULT CGalbrenaGroundIdle::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pOwner);
    ASSERT_CRASH(m_pGalbrena);

    // Idle 애니메이션 리스트 셋업
    Setup_Animations();

    // 기본 애니메이션 셋업.
    m_iCurrentAnimIdx = 0;

    // 바꿀 파트타입?
    
    return S_OK;
}

void CGalbrenaGroundIdle::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaIdleType eIdleType = context.m_eIdleType;

    m_iCurrentAnimIdx = ENUM_CLASS(eIdleType);

    // 3. Idle 상태 초기화
    State_Reset();

	m_pGalbrena->Set_Gravity(true);
}

void CGalbrenaGroundIdle::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 입력 확인
    Handle_Input();

    // 1. Idle 업데이트
    Update_IdleAnimations(fTimeDelta);

    // 2. 물리 체크
    Check_Physics(fTimeDelta);

    // 3. 상태 전환.
    Check_StateTransition(fTimeDelta);

    // 4. 상태 초기화
    State_Reset();
    
}

void CGalbrenaGroundIdle::OnExit()
{
    CGroundState::OnExit();
	m_pGalbrena->Set_Gravity(true);
	m_fFallTime = 0.f;
}

void CGalbrenaGroundIdle::Handle_Input()
{
	m_States[DASH] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));

	m_States[HIT] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::HIT)); // HIT 상태인가?
	m_States[DODGEABLE] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));

	m_States[DODGE] = m_States[DODGEABLE] && m_States[DASH]; // Dodge 가능하면서 Dash 키 누르면?

	if (m_States[HIT] || m_States[DODGE])
		return;

	m_States[FLY] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::T)); // 최우선 순위

    m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[DASH] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::RB));
    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
    m_States[SPRINT] = m_States[MOVE] && m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));

    // LockOn인 경우에는 W, A, S, D 입력값을 모두 판별.
    m_States[MOVE_U] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
    m_States[MOVE_D] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
    m_States[MOVE_L] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
    m_States[MOVE_R] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));

	// AttackState에서 판별.
	m_States[ATTACK] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
    
    // 기본 Skill E
    m_States[SKILL_E] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::E));
    m_States[SKILL_Q] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::Q));
    m_States[SKILL_R] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::R));

	// Burst 상태 확인하기.
	m_States[BURST] = m_pGalbrena->Check_AnyConidtion_FromAbility(ENUM_CLASS(UI_GALBRENA_CONDITION::BURST_ACTIVE));

	/*if (m_States[BURST])
		m_States[BURST_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pGalbrena->Check_Skill("Ex_Skill02"));
	else
		m_States[DEFAULT_E] = m_States[SKILL_E] && (SKILL_STATE::READY == m_pGalbrena->Check_Skill("Skill02"));*/

	// 궁 상태 확인하기.
	m_States[ULTI] = m_States[SKILL_R] && (m_pGalbrena->Get_Cost(COST_TYPE::COST2) >= m_pGalbrena->Get_MaxCost());
}



void CGalbrenaGroundIdle::Update_IdleAnimations(_float fTimeDelta)
{
	CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);
}

void CGalbrenaGroundIdle::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);

	if (m_States[LAND])
	{
		m_fFallTime = 0.f;
	}
	else if (!m_States[LAND])
	{
		m_fFallTime += fTimeDelta;

		if (m_fFallTime >= 0.2f)
			m_States[FALL] = true;
	}

}

// Idles 조건이 아닌 것들.
void CGalbrenaGroundIdle::Check_StateTransition(_float fTimeDelta)
{
	// 우선순위 순으로 전환조건 진행.
    EGalbrenaIdleType eIdleType = static_cast<EGalbrenaIdleType>(m_iCurrentAnimIdx);

    _uint iKeyInput = {};

	// 1. 우선순위

    // Sprint => 빠르게 달리기.
    if (m_States[SPRINT])
    {
        m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::SPRINT_F;
        m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN)); // 상위, 하위 상태
        return;
    }


    
    // 이동은 Run State에서 조절.
    if (m_States[MOVE])
    {
        // 더 우선순위 높은 것. => Sprint
        // LockOn일때 전환 로직 변경.
        if (m_pGalbrena->Is_LockOn())
        {
            if (m_States[MOVE_U])
            {
                if (m_States[MOVE_L])
                    m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_LF; // 애니메이션 상태 => 블랙보드에 기입.        
                else if (m_States[MOVE_R])
                    m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_RF; // 애니메이션 상태 => 블랙보드에 기입.        
                else
                    m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        
            }
            else if (m_States[MOVE_D])
            {
                if (m_States[MOVE_L])
                    m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_LB; // 애니메이션 상태 => 블랙보드에 기입.        
                else if (m_States[MOVE_R])
                    m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_RB; // 애니메이션 상태 => 블랙보드에 기입.        
                else
                    m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_B; // 애니메이션 상태 => 블랙보드에 기입.        
            }
            else if (m_States[MOVE_L])
                m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_LF; // 애니메이션 상태 => 블랙보드에 기입.        
            else if (m_States[MOVE_R])
                m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_RF; // 애니메이션 상태 => 블랙보드에 기입.        
        }
        else
            m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F; // 애니메이션 상태 => 블랙보드에 기입.        

        m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN)); // 상위, 하위 상태
        return;
    }
}



void CGalbrenaGroundIdle::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaIdleType::STAND1_ACTION01), "Stand1_Action01", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaIdleType::STAND1_ACTION02), "Stand1_Action02", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaIdleType::STAND1_ACTION03), "Stand1_Action03", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaIdleType::STAND1), "Stand1", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaIdleType::STAND2), "Stand2", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaIdleType::STAND_CONTROL), "Stand_Control", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaIdleType::STANDCHANGE), "StandChange", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaIdleType::STANDUP), "StandUp", 1.f, 0.f);
}


void CGalbrenaGroundIdle::State_Reset()
{
    for (_uint i = 0; i < IDLESTATE::END; ++i)
        m_States[i] = false;
}

CGalbrenaGroundIdle* CGalbrenaGroundIdle::Create(class CGameObject* pOwner)
{
    CGalbrenaGroundIdle* pInstance = new CGalbrenaGroundIdle();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaGroundIdle");
        return nullptr;
    }

    return pInstance;
}

void CGalbrenaGroundIdle::Free()
{
    CGroundState::Free();
}
