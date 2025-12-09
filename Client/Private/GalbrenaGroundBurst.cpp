#include "ClientPch.h"
#include "GalbrenaGroundBurst.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GameSystem.h"

HRESULT CGalbrenaGroundBurst::Initialize(CCharacter* pCharacter)
{
    if (FAILED(__super::Initialize(pCharacter)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pCharacter);
    ASSERT_CRASH(m_pGalbrena);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    return S_OK;
}

void CGalbrenaGroundBurst::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaBurstType eBurstType = context.m_eBurstType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eBurstType);

    // 4. 상태 초기화
    State_Reset();

	// 5. 이전 정보 가져오기.
	m_strPrevInfo = context.m_strPrevInfo;

	// 6. 무적 상태 추가.
	m_pGalbrena->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));

	// 7. 타겟 바라보기
	m_pGalbrena->Rotate_Target();
	
}

void CGalbrenaGroundBurst::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 1. 입력키 받기.
    Handle_Input();

    // 2. Skill 업데이트
    Update_SkillAnimations(fTimeDelta);

    // 3.  물리 체크
    Check_Physcis(fTimeDelta);

    // 4. 전환 조건 체크
    Check_StateTransition(fTimeDelta);

    // 5. 상태 리셋.
    State_Reset();
    
	

}

void CGalbrenaGroundBurst::OnExit()
{
    CGroundState::OnExit();
    m_pGalbrena->PartActivate(m_iPartType, false);
	m_pGalbrena->Clear_PartAnimation(m_iPartType, m_Animations.at(m_iCurrentAnimIdx).strAnimName);
    m_iPartType = CGalbrena::PARTTYPE::TYPE_END;
	
	// 무적 상태 해제?
	m_pGalbrena->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));

	// 강화 공격 상태를 캐릭터에 바인딩.
	m_pGalbrena->Bind_Condition_ToAbillity(ENUM_CLASS(UI_GALBRENA_CONDITION::BURST_ACTIVE));

	// 강화 E스킬 사용 이후에 시간에 따라 Cost1 게이지가 떨어지게 적용.
	m_pGalbrena->Bind_CostCondition_ToAbility(ENUM_CLASS(COST_TYPE::COST1), ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE));

	// 공격 콜라이더 비활성화
	m_pGalbrena->Collider_Active(TEXT("Main|X|X"), false);

}

void CGalbrenaGroundBurst::Handle_Input()
{
	// 현재 애니메이션이 뭐냐에 따라서?
	EGalbrenaBurstType eBurstType = static_cast<EGalbrenaBurstType>(m_iCurrentAnimIdx);

	m_States[ATTACK] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
	m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
	m_States[JUMP] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
	
}

void CGalbrenaGroundBurst::Update_SkillAnimations(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);
}

void CGalbrenaGroundBurst::Check_Physcis(_float fTimeDelta)
{
}

void CGalbrenaGroundBurst::Check_StateTransition(_float fTimeDelta)
{
    EGalbrenaBurstType eBurstType = static_cast<EGalbrenaBurstType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();


	if (IsEscapePossible && m_strPrevInfo.empty())
	{

		if (m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
				return;
			}
			if (m_States[JUMP])
			{
				m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_WALK_LF;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaAirState::JUMP));
				return;
			}
		}
		if (!m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaAirState::FALL));
				return;
			}
		}
	}
    
    if (m_IsAnimationEnd)
    {
		if (m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
				return;
			}
			if (m_States[JUMP])
			{
				m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_WALK_LF;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaAirState::JUMP));
				return;
			}

			m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STANDCHANGE;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
			return;
		}

		if (!m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaAirState::FALL));
				return;
			}
		}
    }
    
}

void CGalbrenaGroundBurst::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaBurstType::SKILL01), "Skill01", 1.5f, 70.f);
}

void CGalbrenaGroundBurst::State_Reset()
{
    for (_uint i = 0; i < BURSTSTATE::END; ++i)
        m_States[i] = false;
}


CGalbrenaGroundBurst* CGalbrenaGroundBurst::Create(CCharacter* pOwner)
{
    CGalbrenaGroundBurst* pInstance = new CGalbrenaGroundBurst();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaGroundBurst");
    }

    return pInstance;
}

void CGalbrenaGroundBurst::Free()
{
    CGroundState::Free();
}
