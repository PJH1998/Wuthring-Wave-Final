#include "ClientPch.h"
#include "GalbrenaEvent.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"

HRESULT CGalbrenaEvent::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CInteractionState::Initialize(pCharacter)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pCharacter);
    ASSERT_CRASH(m_pGalbrena);

    Setup_Animations();
    return S_OK;
}



void CGalbrenaEvent::OnEnter(void* pArg)
{
	CInteractionState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaEventType eEventType = context.m_eEventType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eEventType);

    // 4. 상태 초기화
    State_Reset();

	// 5. 몬스터 타겟으로 위치 및 타겟 회전
	CTransform* pBossTransform = static_cast<CTransform*>(pArg);
	_vector vTargetPos = pBossTransform->Get_State(STATE::POSITION);
	_vector vTargetLook = XMVectorSetY(XMVector3Normalize(pBossTransform->Get_State(STATE::LOOK)), 0.f);
	vTargetPos += vTargetLook * 1.f; // 10.f 전방 이동.

	m_pGalbrena->Set_Position(vTargetPos);
	//mm_pGalbrena->Set_ColliderPosition(vTargetPos);

	m_pGalbrena->Rotate_Target(pBossTransform);
}

void CGalbrenaEvent::OnUpdate(_float fTimeDelta)
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

void CGalbrenaEvent::OnExit()
{
	CInteractionState::OnExit();
	m_IsStopOnce = false;
}



void CGalbrenaEvent::Handle_Input()
{
	m_States[EXIT] = !m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::ANIMSTOP)) 
		&& m_IsStopOnce;
}

void CGalbrenaEvent::Update_EventAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta);
	
}

void CGalbrenaEvent::Check_StateTransition(_float fTimeDelta)
{
    _bool IsEscapePossible = CState::Is_EscapePossible();
	// Hit는 무조건 전환

	// Stop 된 적이 없다면? => 애니메이션 탈출 시점에 Stop
	if (!m_IsStopOnce)
	{
		if (IsEscapePossible)
		{
			m_IsStopOnce = true;
			m_pGalbrena->Start_Anim_ToEvent();
			m_pGalbrena->Bind_Condition_ToPlayer("LeviatanExecuteExit"); // Exit 호출.
			return;
		}
	}

	// 1. 탈출 가능 조건이라면?
	if (m_States[EXIT]) 
	{
		if (IsEscapePossible)
		{
			m_pGalbrena->Activate(false); // 활성화 종료.
			return;
		}
	}
}


void CGalbrenaEvent::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaEventType::ATTACK07), "Attack07", 1.2f, 60.f, 1.f);
}

void CGalbrenaEvent::State_Reset()
{
    for (_uint i = 0; i < EventSTATE::END; ++i)
        m_States[i] = false;
}



CGalbrenaEvent* CGalbrenaEvent::Create(CCharacter* pOwner)
{
    CGalbrenaEvent* pInstance = new CGalbrenaEvent();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaEvent");
        return nullptr;
    }

    return pInstance;
}

void CGalbrenaEvent::Free()
{
	CInteractionState::Free();
}
