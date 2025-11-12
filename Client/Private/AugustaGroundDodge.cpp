#include "ClientPch.h"
#include "AugustaGroundDodge.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"
#include "GameInstance.h"


HRESULT CAugustaGroundDodge::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();

    return S_OK;
}

void CAugustaGroundDodge::OnEnter(void* pArg)
{
    // 상위 객체 수행 작업.
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
	EAugustaDodgeType EDodgeType = context.m_eDodgeType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eDodgeType);

    State_Reset();

	// 4. 락온 중이였다면? => 한번만 입력방향에 따른 회전.
	if (m_pAugusta->Is_LockOn())
	{
		// 5. 누른 키에 따른 입력 방향 받아오기.
		m_eDir = m_pAugusta->Calculate_Direction();
		_vector vMoveDir = m_pAugusta->Calculate_Move_Direction(m_eDir);
		m_pAugusta->Rotate_Direction(vMoveDir);
	}

	m_pAugusta->Set_Gravity(true);

	// 6. 플레이어 상태 제어 => 무적 추가 및 Hit 상태 제거
	// 회피 가능 창을 닫습니다. => Timer 실행 방지.
	m_pAugusta->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE)); 
	m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::DODGE));
	m_pAugusta->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::HIT));

	// 7. Hit Stop
	CGameInstance* pGameInstance = CGameInstance::GetInstance();
	//pGameInstance->Change_TimeRate(TEXT("Timer_60"), 0.7f, 0.1f);
	pGameInstance->Change_TimeRatio_ToLayer(ENUM_CLASS(pGameInstance->Get_CurrentLevel()), TEXT("Layer_Players"), 0.7f, 0.2f); // Dodge 시간 동안 느리게하기?
	pGameInstance->Change_TimeRatio_ToLayer(ENUM_CLASS(pGameInstance->Get_CurrentLevel()), TEXT("Layer_Enemy"), 0.7f, 0.2f); // Dodge 시간 동안 느리게하기?

	CAMERA_SHAKE Desc{};
	Desc.fDuration = 0.15f;
	Desc.fFrequency = 20.f;
	Desc.fAmplitude = 0.5f;
	Desc.vRotation = { 0.f, 0.1f, 0.f};
	Desc.fFovKick = 0.f; // 
	
	pGameInstance->OnShake(Desc);

}

void CAugustaGroundDodge::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키 입력
    Handle_Input();

    // 1. 애니메이션 실행
    Update_SprintAnimation(fTimeDelta);
    
    // 2. 상태 제어.
    Check_StateTransition(fTimeDelta);

    // 3. 상태 초기화.
    State_Reset();
}

void CAugustaGroundDodge::OnExit()
{
    CGroundState::OnExit();
	m_pAugusta->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::DODGE));
}

void CAugustaGroundDodge::Handle_Input()
{
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);

	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);
	m_States[LOCKON] = m_pAugusta->Is_LockOn();

}



void CAugustaGroundDodge::Update_SprintAnimation(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pAugusta->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; // 거리 계산에 따른 Animation Scale 조절.

    // 1. 누른키에 따른 방향 계산
    m_eDir = m_pAugusta->Calculate_Direction();

	// 2. Animation 실행.
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta, m_fAnimationScale);

	
    
}

void CAugustaGroundDodge::Check_StateTransition(_float fTimeDelta)
{
	EAugustaDodgeType eDodgeType = static_cast<EAugustaDodgeType>(m_iCurrentAnimIdx);

	_bool IsEscapePossible =CState::Is_EscapePossible();
  

	if (IsEscapePossible)
	{
		if (m_States[LAND])
		{
			if (m_States[MOVE])
			{
				m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
				m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN)); // 상위, 하위 상태
				return;
			}
		}

		if (!m_States[LAND])
		{
			m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL)); // 상위, 하위 상태
			return;
		}

	}


	// 애니메이션 끝나면?
	if (m_IsAnimationEnd)
	{
		m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE)); // 상위, 하위 상태
		return;
	}

}

void CAugustaGroundDodge::Setup_Animations()
{
	CState::Add_Animations(ENUM_CLASS(EAugustaDodgeType::MOVE_LIMIT_F), "Move_Limit_F", 1.3f, 52.f, 2.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaDodgeType::MOVE_LIMIT_B), "Move_Limit_B", 1.3f, 52.f, 2.f);
}

void CAugustaGroundDodge::State_Reset()
{
    for (_uint i = 0; i < DASHSTATE::END; ++i)
        m_States[i] = false;
}

CAugustaGroundDodge* CAugustaGroundDodge::Create(class CGameObject* pOwner)
{
    CAugustaGroundDodge* pInstance = new CAugustaGroundDodge();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundDodge");
    }

    return pInstance;
}

void CAugustaGroundDodge::Free()
{
    CGroundState::Free();
}
