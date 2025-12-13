#include "ClientPch.h"
#include "RoverEvent.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"

HRESULT CRoverEvent::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CInteractionState::Initialize(pCharacter)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pCharacter);
    ASSERT_CRASH(m_pRover);

    Setup_Animations();
    return S_OK;
}



void CRoverEvent::OnEnter(void* pArg)
{
	CInteractionState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverEventType eEventType = context.m_eEventType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eEventType);

    // 4. 상태 초기화
    State_Reset();

	// 5. 몬스터 타겟으로 회전
	CTransform* pBossTransform = static_cast<CTransform*>(pArg);


	switch (eEventType)
	{
		case ERoverEventType::BEHIT_FLY_FALL:
		{
			m_pRover->Rotate_Target(pBossTransform);
			break;
		}
		case ERoverEventType::BURST02:
		{
			// 1. BossTransform의 반대 방향으로 이동.
			_vector vTargetPos = pBossTransform->Get_State(STATE::POSITION);
			_vector vTargetLook = XMVectorSetY(XMVector3Normalize(pBossTransform->Get_State(STATE::LOOK)), 0.f);
			
			vTargetPos += (vTargetLook * -1.f) * 2.f; // 2.f 후방 이동.
			m_pRover->Set_Position(vTargetPos);

			m_pRover->Rotate_Target(pBossTransform);

			m_iPartType = CRover::PARTTYPE::PART_SWORD;
			m_pRover->PartActivate(m_iPartType, true);
			m_pRover->Set_SocketMatrixToParts(m_iPartType, "WeaponProp01");
			break;
		}

	}


	
}

void CRoverEvent::OnUpdate(_float fTimeDelta)
{
    
	CInteractionState::OnUpdate(fTimeDelta);

    // 0. 키입력 제어
    Handle_Input();

    // 1. 애니메이션 제어.
    Update_EventAnimation(fTimeDelta);

    // 2. 상태 제어.
    Check_StateTransition(fTimeDelta);
   
    // 3. 상태 초기화
    State_Reset();
}

void CRoverEvent::OnExit()
{
	CInteractionState::OnExit();
	m_IsStopOnce = false;

	if (m_iPartType != CRover::PARTTYPE::TYPE_END);
		m_pRover->PartActivate(m_iPartType, false);
}



void CRoverEvent::Handle_Input()
{
	m_States[QTE_EXIT] = !m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::ANIMSTOP)) && m_IsStopOnce;
}

void CRoverEvent::Update_EventAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pRover, fTimeDelta);

}

void CRoverEvent::Check_StateTransition(_float fTimeDelta)
{
    _bool IsEscapePossible = CState::Is_EscapePossible();
	// Hit는 무조건 전환

	ERoverEventType eEventType = static_cast<ERoverEventType>(m_iCurrentAnimIdx);

	// Stop 된 적이 없다면? => 애니메이션 탈출 시점에 Stop


	if (eEventType == ERoverEventType::BEHIT_FLY_FALL)
	{
		if (!m_IsStopOnce)
		{
			if (IsEscapePossible)
			{
				m_IsStopOnce = true;
				m_pRover->Stop_Anim();
				m_pRover->Set_LeviatanQTE(true);
				m_pRover->Spawn_LeviatanAnchorEffect(TEXT("Common_Bondage"));
				m_pRover->Stop_Action();
				//m_pRover->Play_Action(TEXT("Camera_Action"), true, false);
				return;
			}
		}

		// 1. 탈출 가능 조건이라면?
		if (m_States[QTE_EXIT])
		{
			if (IsEscapePossible)
			{
				m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1_ACTION01;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
				m_pRover->Start_Anim();
				m_pRover->Set_LeviatanQTE(false);
				//m_pRover->Stop_Action();
				return;
			}
		}
	}

	if (eEventType == ERoverEventType::BURST02)
	{
		// 특정 지점에서 Stop Anim
		if (!m_IsStopOnce)
		{
			//if (IsEscapePossible)
			if (m_fTrackPosition > 50.f)
			{
				m_IsStopOnce = true;
				m_pRover->Stop_Anim();
				m_pRover->Play_Action(TEXT("Action_Levi_Execute"),true, false);
				return;
			}
		}
	}


	

	

    
}


void CRoverEvent::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERoverEventType::BEHIT_FLY_FALL), "Behit_Fly_Fall", 1.5f, 20.f, 2.f);
	CState::Add_Animations(ENUM_CLASS(ERoverEventType::BURST02), "Burst02", 1.f, 60.f);
}

void CRoverEvent::State_Reset()
{
    for (_uint i = 0; i < EventSTATE::END; ++i)
        m_States[i] = false;
}



CRoverEvent* CRoverEvent::Create(CCharacter* pOwner)
{
    CRoverEvent* pInstance = new CRoverEvent();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverEvent");
        return nullptr;
    }

    return pInstance;
}

void CRoverEvent::Free()
{
	CInteractionState::Free();
}
