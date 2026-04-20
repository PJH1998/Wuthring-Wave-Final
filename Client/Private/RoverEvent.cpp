#include "ClientPch.h"
#include "RoverEvent.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"

static constexpr const wchar_t* EFFECT_TAG_LEVIATAN_ANCHOR = L"Common_Bondage";

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
    const auto context = m_pRover->TakeStateContext();
    ERoverEventType eEventType = context.m_eEventType;
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eEventType);
    State_Reset();
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
			_vector vTargetPos = pBossTransform->Get_State(STATE::POSITION);
			_vector vTargetLook = XMVectorSetY(XMVector3Normalize(pBossTransform->Get_State(STATE::LOOK)), 0.f);
			
			vTargetPos += (vTargetLook * -1.f) * 3.f; // 2.f 후방 이동.
			
			m_pRover->Set_Position(vTargetPos);
			m_pRover->Set_ColliderPosition(vTargetPos);

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
    Handle_Input();
    Update_EventAnimation(fTimeDelta);
    Check_StateTransition(fTimeDelta);
    State_Reset();
}

void CRoverEvent::OnExit()
{
	CInteractionState::OnExit();
	m_IsStopOnce = false;

	if (m_iPartType != CRover::PARTTYPE::TYPE_END)
		m_pRover->PartActivate(m_iPartType, false);
}



void CRoverEvent::Handle_Input()
{
	m_States[QTE_EXIT] = !m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::ANIMSTOP)) && m_IsStopOnce;
	m_States[EXECUTE_EXIT] = !m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::ANIMSTOP)) && m_IsStopOnce;
}

void CRoverEvent::Update_EventAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pRover, fTimeDelta);

}

void CRoverEvent::Check_StateTransition(_float fTimeDelta)
{
    _bool IsEscapePossible = CState::Is_EscapePossible();
	ERoverEventType eEventType = static_cast<ERoverEventType>(m_iCurrentAnimIdx);

	if (eEventType == ERoverEventType::BEHIT_FLY_FALL)
	{
		if (!m_IsStopOnce)
		{
			if (IsEscapePossible)
			{
				m_IsStopOnce = true;
				m_pRover->Stop_Anim();
				m_pRover->Set_LeviatanQTE(true);
				m_pRover->Spawn_LeviatanAnchorEffect(EFFECT_TAG_LEVIATAN_ANCHOR);
				return;
			}
		}

		if (m_States[QTE_EXIT])
		{
			if (IsEscapePossible)
			{
				m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1_ACTION01;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
				m_pRover->Start_Anim();
				m_pRover->Set_LeviatanQTE(false);
				return;
			}
		}
	}

	if (eEventType == ERoverEventType::BURST02)
	{
		// 특정 지점에서 Stop Anim
		if (!m_IsStopOnce)
		{
			if (IsEscapePossible)
			{
				m_IsStopOnce = true;
				m_pRover->Stop_Anim();
				m_pRover->Change_TimeRatio_ToLayer(COLLISIONLAYER::ENEMY, 0.f);
				m_pRover->Bind_Condition_ToPlayer(PLAYER_CONDITION::LEVIATANEXECUTE_SUCCESS);
				return;
			}
		}

		if (m_States[EXECUTE_EXIT])
		{
			if (IsEscapePossible)
			{
				m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STANDUP;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
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
