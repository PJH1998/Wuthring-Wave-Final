#include "ClientPch.h"
#include "AugustaGroundQTE.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaGroundQTE::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaGroundQTE::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaLandType eLandType = context.m_eLandType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(context.m_eLandType);

    // 4. 상태 초기화
    State_Reset();

	// 5. 중력 켰다.
	m_pAugusta->Set_Gravity(true);

}

void CAugustaGroundQTE::OnUpdate(_float fTimeDelta)
{
    
    CGroundState::OnUpdate(fTimeDelta);

    // 0. 키입력 제어
    Handle_Input();

    // 1. 애니메이션 제어.
    Update_LandAnimation(fTimeDelta);

    // 2. 상태 제어.
    Check_StateTransition(fTimeDelta);
   
    // 3. 상태 초기화
    State_Reset();
}

void CAugustaGroundQTE::OnExit()
{
    CGroundState::OnExit();
	m_pAugusta->Set_Gravity(true);
}



void CAugustaGroundQTE::Handle_Input()
{
	
}

void CAugustaGroundQTE::Update_LandAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
}

void CAugustaGroundQTE::Check_StateTransition(_float fTimeDelta)
{
    _bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. 끝나면 콜백을 호출시켜야함 => Player가 인지하게끔?
	if (m_IsAnimationEnd)
	{

	}
	
}


void CAugustaGroundQTE::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaLandType::LAND_LIGHT), "SkillQTE", 1.f, 10.f);
}

void CAugustaGroundQTE::State_Reset()
{
    for (_uint i = 0; i < QTESTATE::END; ++i)
        m_States[i] = false;
}



CAugustaGroundQTE* CAugustaGroundQTE::Create(class CGameObject* pOwner)
{
    CAugustaGroundQTE* pInstance = new CAugustaGroundQTE();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundQTE");
        return nullptr;
    }

    return pInstance;
}

void CAugustaGroundQTE::Free()
{
    CGroundState::Free();
}
