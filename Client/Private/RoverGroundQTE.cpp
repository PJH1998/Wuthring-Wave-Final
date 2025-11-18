#include "ClientPch.h"
#include "RoverGroundQTE.h"
#include "Rover.h"
#include "StateMachine.h"
#include "RoverState_Enum.h"

HRESULT CRoverGroundQTE::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pRover = dynamic_cast<CRover*>(pOwner);
    ASSERT_CRASH(m_pRover);

    Setup_Animations();
    return S_OK;
}



void CRoverGroundQTE::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pRover->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ERoverQTEType eQTEType = context.m_eQTEType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eQTEType);

    // 4. 상태 초기화
    State_Reset();

	// 5. 중력 켰다.
	m_pRover->Set_Gravity(false);
	m_pRover->Rotate_Target();

	m_pRover->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));

	// 6. 카메라 변경.
	_bool IsSelect = m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::SELECT));
	if (IsSelect) // 선택된 캐릭터일때만?
		m_pRover->Bind_QTECamera();
}

void CRoverGroundQTE::OnUpdate(_float fTimeDelta)
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

void CRoverGroundQTE::OnExit()
{
    CGroundState::OnExit();
	m_pRover->Set_Gravity(false);
	if (!m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::SELECT)))
	{
		m_pRover->Set_QTEEnd(true);
		m_pRover->Bind_ChangeEffect();
	}
	else
		m_pRover->Reset_QTECamera();
		
	
	// 공격 콜라이더 비활성화
	m_pRover->Collider_Active(TEXT("Main|X|X"), false);
	m_pRover->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));

	
}



void CRoverGroundQTE::Handle_Input()
{
	m_States[SELECT] = m_pRover->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::SELECT));

	m_States[MOVE] = m_pRover->Check_AnyInput(m_iMoveKey) && m_States[SELECT];
	m_States[LAND] = m_pRover->Is_LandCollider(&m_vLandNormal);
}

void CRoverGroundQTE::Update_QTEAnimation(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pRover->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; // 거리 계산에 따른 Animation Scale 조절.

	if (m_fAnimationScale > 1.f)
		m_fAnimationScale = 1.f;

    // 1. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pRover, fTimeDelta, m_fAnimationScale);


}

void CRoverGroundQTE::Check_StateTransition(_float fTimeDelta)
{
    _bool IsEscapePossible = CState::Is_EscapePossible();

	if (IsEscapePossible)
	{
		if (m_States[MOVE])
		{
			if (m_States[LAND])
			{
				m_pRover->GetStateContextForWrite().m_eRunType = ERoverRunType::RUN_F;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::RUN));
				return;
			}
			else if (!m_States[LAND])
			{
				m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL));
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
				m_pRover->GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
				return;
			}
			else if (!m_States[LAND])
			{
				m_pRover->GetStateContextForWrite().m_eFallType = ERoverFallType::FALL_LOOP;
				m_pRover->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ERoverAirState::FALL));
				return;
			}
		}
		else
		{
			OnExit();
		}
		
	
		return;
	}
	
}


void CRoverGroundQTE::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ERoverQTEType::SKILL_QTE), "SkillQte", 1.f, 60.f);
}

void CRoverGroundQTE::State_Reset()
{
    for (_uint i = 0; i < QTESTATE::END; ++i)
        m_States[i] = false;
}



CRoverGroundQTE* CRoverGroundQTE::Create(class CGameObject* pOwner)
{
    CRoverGroundQTE* pInstance = new CRoverGroundQTE();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CRoverGroundQTE");
        return nullptr;
    }

    return pInstance;
}

void CRoverGroundQTE::Free()
{
    CGroundState::Free();
}
