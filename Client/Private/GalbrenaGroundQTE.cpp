#include "ClientPch.h"
#include "GalbrenaGroundQTE.h"
#include "Galbrena.h"
#include "StateMachine.h"
#include "GalbrenaState_Enum.h"

HRESULT CGalbrenaGroundQTE::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pCharacter);
    ASSERT_CRASH(m_pGalbrena);

    Setup_Animations();
    return S_OK;
}



void CGalbrenaGroundQTE::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaQTEType eQTEType = context.m_eQTEType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eQTEType);

    // 4. 상태 초기화
    State_Reset();

	// 5. 중력 껐다.
	m_pGalbrena->Set_Gravity(true);
	m_pGalbrena->Rotate_Target();

	m_pGalbrena->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));

	// 6. 카메라 변경
	_bool IsSelect = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::SELECT));
	if (IsSelect) // 선택된 캐릭터일때만?
		m_pGalbrena->Bind_QTECamera();
		
}

void CGalbrenaGroundQTE::OnUpdate(_float fTimeDelta)
{
    
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키입력 제어
    Handle_Input();

    // 1. 애니메이션 제어.
	Update_QTEAnimation(fTimeDelta);

    // 2. 상태 제어.
    Check_StateTransition(fTimeDelta);
   
    // 3. 상태 초기화
    State_Reset();
}

void CGalbrenaGroundQTE::OnExit()
{
    CGroundState::OnExit();
	m_pGalbrena->Set_Gravity(false);
	if (!m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::SELECT)))
	{
		m_pGalbrena->Set_QTEEnd(true);
		m_pGalbrena->Bind_ChangeEffect();
	}
	else
		m_pGalbrena->Reset_QTECamera();
		
	
	// 공격 콜라이더 비활성화
	m_pGalbrena->Collider_Active(TEXT("Main|X|X"), false);

	m_pGalbrena->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));

	//m_pGalbrena->Reset_QTECamera();

	
}



void CGalbrenaGroundQTE::Handle_Input()
{
	m_States[SELECT] = m_pGalbrena->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::SELECT));

	m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey) && m_States[SELECT];
	m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
}

void CGalbrenaGroundQTE::Update_QTEAnimation(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pGalbrena->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; // 거리 계산에 따른 Animation Scale 조절.

	if (m_fAnimationScale > 1.f)
		m_fAnimationScale = 1.f;

    // 1. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta, m_fAnimationScale);
}

void CGalbrenaGroundQTE::Check_StateTransition(_float fTimeDelta)
{
    _bool IsEscapePossible = CState::Is_EscapePossible();

	if (IsEscapePossible)
	{
		if (m_States[MOVE])
		{
			if (m_States[LAND])
			{
				m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN));
				return;
			}
			else if (!m_States[LAND])
			{
				m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL));
				return;
			}
		}
	}

	// 1. 끝나면 콜백을 호출시켜야함 => Player가 인지하게끔?
	if (m_IsAnimationEnd)
	{
		if (m_States[SELECT])
		{
			if (m_States[LAND])
			{
				m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STANDCHANGE02;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
				return;
			}
			else if (!m_States[LAND])
			{
				m_pGalbrena->GetStateContextForWrite().m_eFallType = EGalbrenaFallType::FALL_LOOP;
				m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::FALL));
				return;
			}
		}
		else
		{
			OnExit();
			return;
		}
	}
	
}


void CGalbrenaGroundQTE::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaQTEType::SKILL_QTE), "Attack07", 1.5f, 100.f);
}

void CGalbrenaGroundQTE::State_Reset()
{
    for (_uint i = 0; i < QTESTATE::END; ++i)
        m_States[i] = false;
}



CGalbrenaGroundQTE* CGalbrenaGroundQTE::Create(CCharacter* pOwner)
{
    CGalbrenaGroundQTE* pInstance = new CGalbrenaGroundQTE();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaGroundQTE");
        return nullptr;
    }

    return pInstance;
}

void CGalbrenaGroundQTE::Free()
{
    CGroundState::Free();
}
