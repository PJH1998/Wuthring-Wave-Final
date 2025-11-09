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
    EAugustaQTEType eQTEType = context.m_eQTEType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = ENUM_CLASS(eQTEType);

	

    // 4. 상태 초기화
    State_Reset();

	// 5. 중력 켰다.
	m_iPartType = CAugusta::PARTTYPE::PART_BAYONET; // 추후 애니메이션에 따른. 분기문 필요.

	_string strBoneName = "WeaponProp02";
	m_pAugusta->PartActivate(m_iPartType, true); // 파츠 변경. // Volume Activate는 Notify로..
	m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
	m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
	m_pAugusta->Set_Gravity(true);

	m_pAugusta->Rotate_Target();
}

void CAugustaGroundQTE::OnUpdate(_float fTimeDelta)
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

void CAugustaGroundQTE::OnExit()
{
    CGroundState::OnExit();
	m_pAugusta->Set_Gravity(false);
	m_pAugusta->Set_QTEEnd(true);
	
}



void CAugustaGroundQTE::Handle_Input()
{
	
}

void CAugustaGroundQTE::Update_QTEAnimation(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pAugusta->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations[m_iCurrentAnimIdx].fRootMotionRate * m_fRootMotionScale; // 거리 계산에 따른 Animation Scale 조절.

	if (m_fAnimationScale > 1.f)
		m_fAnimationScale = 1.f;

    // 1. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta, m_fAnimationScale);


	// 2. 파츠 애니메이션 실행.
	m_pAugusta->Play_PartAnimation(
		m_iPartType,
		m_Animations[m_iCurrentAnimIdx].strAnimName,
		fTimeDelta, nullptr
	);

}

void CAugustaGroundQTE::Check_StateTransition(_float fTimeDelta)
{
    _bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. 끝나면 콜백을 호출시켜야함 => Player가 인지하게끔?
	if (m_IsAnimationEnd)
	{
		OnExit();
		/*m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
		m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));*/
		return;
	}
	
}


void CAugustaGroundQTE::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaQTEType::SkillQTE), "SkillQTE", 1.f, 60.f);
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
