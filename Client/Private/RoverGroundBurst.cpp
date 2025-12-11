#include "ClientPch.h"
#include "RoverGroundBurst.h"
#include "Rover.h"
#include "StateMachine.h"
#include "GameSystem.h"

HRESULT CRoverGroundBurst::Initialize(CCharacter* pCharacter)
{
    if (FAILED(__super::Initialize(pCharacter)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pCharacter);
    ASSERT_CRASH(m_pRover);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    return S_OK;
}

void CRoverGroundBurst::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverBurstType eBurstType = context.m_eBurstType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eBurstType);

    // 4. 상태 초기화
    State_Reset();

	// 5. 이전 정보 가져오기.
	m_strPrevInfo = context.m_strPrevInfo;

	// 6. 만약 궁이라면? => Cut Scene 실행.

	// 7. 흑 날개.
	_string strBoneName = "WingCase";
	m_iPartType = CRover::PARTTYPE::PART_DARKWING;
	m_pRover->PartActivate(m_iPartType, true);
	m_pRover->Set_SocketMatrixToParts(m_iPartType, strBoneName);
	m_pRover->Rotate_Target(); // 진입 시 한번만

	m_pRover->Set_Gravity(true);

	if (m_strPrevInfo == "ULTI")
	{
		m_pRover->Change_TimeRate(TEXT("Timer_60"), 0.5f, 1.f);
		m_pRover->Play_Action(TEXT("Action_Rover_Burst01"));
		m_pRover->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
		m_pRover->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::CUTSCENE));
		m_pRover->Set_OutLineVisible(false); // 궁극기 도중에는 입 모양이 보이게 하기 위함.
		m_pRover->Change_TimeRatio_ToLayer(COLLISIONLAYER::ENEMY, 0.f);
	}
	else
	{
		m_pRover->Play_Action(TEXT("Action_Rover_Ex_Skill_01_01"));
		m_iCurrentAnimIdx = ENUM_CLASS(ERoverBurstType::EX_SKILL01_01);
	}
	
	
}

void CRoverGroundBurst::OnUpdate(_float fTimeDelta)
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

void CRoverGroundBurst::OnExit()
{
    CGroundState::OnExit();
    m_pRover->PartActivate(m_iPartType, false);
    m_iPartType = CRover::PARTTYPE::TYPE_END;


	m_pRover->Clear_PartAnimation(m_iPartType, m_Animations.at(m_iCurrentAnimIdx).strAnimName);

	if (m_strPrevInfo.empty())
	{
		m_pRover->Bind_Condition_ToAbillity(ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE));
		m_pRover->Bind_CostCondition_ToAbility(ENUM_CLASS(COST_TYPE::COST1), ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE));
	}

	// 공격 콜라이더 비활성화
	m_pRover->Collider_Active(TEXT("Main|X|X"), false);

	if (m_strPrevInfo == "ULTI")
	{
		m_pRover->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
		m_pRover->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::CUTSCENE));
		m_pRover->Set_OutLineVisible(false); // 궁극기 도중에는 입 모양이 보이게 하기 위함.
		m_pRover->Change_TimeRatio_ToLayer(COLLISIONLAYER::ENEMY, 1.f);
	}
	

	
}

void CRoverGroundBurst::Handle_Input()
{
	m_States[ATTACK] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
	m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey);
	m_States[JUMP] = m_pRover->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));

	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
}

void CRoverGroundBurst::Update_SkillAnimations(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pRover, fTimeDelta);

	m_pRover->Play_PartAnimation(
		m_iPartType,
		"B_Burst01",
		m_Animations.at(m_iCurrentAnimIdx).fSpeed * fTimeDelta, nullptr
	);
}

void CRoverGroundBurst::Check_Physcis(_float fTimeDelta)
{
}

void CRoverGroundBurst::Check_StateTransition(_float fTimeDelta)
{
    ERoverBurstType eBurstType = static_cast<ERoverBurstType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();


	if (IsEscapePossible && m_strPrevInfo.empty())
	{
		if (m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
				return;
			}
			if (m_States[JUMP])
			{
				m_pRover->GetStateContextForWrite().m_eJumpType = ERoverJumpType::JUMP_WALK_LF;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverAirState::JUMP));
				return;
			}
		}
		if (!m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverAirState::FALL));
				return;
			}
		}
	}
    
    if (m_IsAnimationEnd)
    {
		m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STANDCHANGE;
		m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
		return;
    }
    
}

void CRoverGroundBurst::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERoverBurstType::BURST01), "Burst01", 1.f, 50.f);
    CState::Add_Animations(ENUM_CLASS(ERoverBurstType::EX_SKILL01_01), "Ex_Skill01_01", 1.5f, 30.f);
}

void CRoverGroundBurst::State_Reset()
{
    for (_uint i = 0; i < BURSTSTATE::END; ++i)
        m_States[i] = false;
}


CRoverGroundBurst* CRoverGroundBurst::Create(CCharacter* pOwner)
{
    CRoverGroundBurst* pInstance = new CRoverGroundBurst();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverGroundBurst");
    }

    return pInstance;
}

void CRoverGroundBurst::Free()
{
    CGroundState::Free();
}
