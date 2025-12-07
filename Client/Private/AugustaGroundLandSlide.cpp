#include "ClientPch.h"
#include "AugustaGroundLandSlide.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "AugustaState_Enum.h"

HRESULT CAugustaGroundLandSlide::Initialize(CCharacter* pCharacter)
{
    if (FAILED(CGroundState::Initialize(pCharacter)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pCharacter);
    ASSERT_CRASH(m_pAugusta);

    Setup_Animations();
    return S_OK;
}



void CAugustaGroundLandSlide::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

	// 0. Sliding 정보 가져오기.
	if (nullptr == pArg) return;
	m_SlideData = *static_cast<SLIDE_DATA*>(pArg);

    // 1. 복사본 context 받아오기.
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaLandSlideType eLandSlideType = context.m_eLandSlideType;

    // 3. 값에 따른 상태 변경.
    m_iCurrentAnimIdx = static_cast<_uint>(eLandSlideType);

    // 4. 상태 초기화
    State_Reset();

	// 5. 중력 끕니다. => 정해진 경로로 이동할 것이므로.
	m_pAugusta->Set_Gravity(false);
	m_pAugusta->ColliderActive(false);
	m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::COLLIDER_UNACTIVE));
	// 6. 현재 위치를 받아옵니다.
	XMStoreFloat3(&m_vStart, m_pAugusta->Get_Position());

	// 7. Rotate
	_vector vCurrentPos = m_pAugusta->Get_Position();						// 현재 캐릭터 위치
	_vector vTargetPos = XMLoadFloat3(&m_SlideData.WayPoints[m_iWayPoint]); // 목표 WayPoint

	_vector vDir = XMVectorSetW(vTargetPos - vCurrentPos, 0.f);			// 목표 방향
	m_pAugusta->Rotate_Direction(vDir);


	// 8. Collider 끄기.

	

}

void CAugustaGroundLandSlide::OnUpdate(_float fTimeDelta)
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

void CAugustaGroundLandSlide::OnExit()
{
    CGroundState::OnExit();
	m_pAugusta->ColliderActive(true);
	m_pAugusta->Set_Gravity(true);
	//m_pAugusta->Clear_Animation(m_Animations.at(m_iCurrentAnimIdx).strAnimName, 0.f);
	m_pAugusta->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::LANDSLIDE));
	m_pAugusta->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::COLLIDER_UNACTIVE));
	m_iWayPoint = 0;
	m_SlideData.Reset();
}



void CAugustaGroundLandSlide::Handle_Input()
{
	m_States[EXIT] = !m_pAugusta->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::LANDSLIDE));

	m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);

	m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);
}

void CAugustaGroundLandSlide::Update_LandAnimation(_float fTimeDelta)
{
    // 0. 애니메이션 실행부터
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
	
	// 인덱스가 범위를 벗어났다면 이동 종료.
	if (m_iWayPoint >= m_SlideData.WayPoints.size())
	{
		m_States[EXIT] = true;
		return;
	}

	// 1. 현재 위치와 목표 위치 계산
	_vector vCurrentPos = m_pAugusta->Get_Position();						
	_vector vTargetPos = XMVectorSetW(XMLoadFloat3(&m_SlideData.WayPoints[m_iWayPoint]), 1.f);

	// 2. 방향 벡터 및 거리 계산
	_vector vDir = vTargetPos - vCurrentPos;			// 목표 방향
	_float fDist = XMVectorGetX(XMVector3Length(vDir)); // 남은 거리

	// 3. 이동 처리
	if (fDist <= 0.3f)
	{
		m_pAugusta->Set_Position(vTargetPos);
		m_iWayPoint++; // 다음 WayPoint로 인덱스 증가.

		if (m_iWayPoint >= m_SlideData.WayPoints.size())
			m_States[EXIT] = true;
	}
	else
	{
		// 목표 지점까지 거리가 남은 경우?
		m_pAugusta->Set_Gravity(true);
		//vDir = XMVectorSetW(XMVector3Normalize(XMVectorSetY(vDir, 0.f)), 0.f); // 방향 정규화
		vDir = XMVectorSetW(XMVector3Normalize(vDir), 0.f);
		m_pAugusta->Move_Direction(vDir, fTimeDelta, 1.2f);

		vDir = XMVector3Normalize(XMVectorSetY(vDir, 0.f));
		m_pAugusta->Rotate_DirectionNoPitchLerp(vDir, fTimeDelta, 3.f);
		//m_pAugusta->Rotate_DirectionLerp(vDir, fTimeDelta, 0.1f);

		
	}

}

void CAugustaGroundLandSlide::Check_StateTransition(_float fTimeDelta)
{
    _bool IsEscapePossible = CState::Is_EscapePossible();
	EAugustaLandSlideType eLandSlideType = static_cast<EAugustaLandSlideType>(m_iCurrentAnimIdx);

	if (m_States[EXIT])
	{
		if (m_States[MOVE])
		{
			m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
			return;
		}

		if (m_States[LAND])
		{
			m_pAugusta->GetStateContextForWrite().m_eSprintType = EAugustaSprintType::STOP_SPRINT_L;
			m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SPRINT));
			return;
		}
	}


	if (m_IsAnimationEnd) // 애니메이션 끝나면.
	{
		if (EAugustaLandSlideType::LANDSLIDE_SPRINT_LOOP == eLandSlideType)
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaLandSlideType::LANDSLIDE_SPRINT_LOOP);
			m_pAugusta->Clear_Animation(m_Animations.at(m_iCurrentAnimIdx).strAnimName, 0.f);
			//m_pAugusta->GetStateContextForWrite().m_eLandSlideType = EAugustaLandSlideType::LANDSLIDE_SPRINT_LOOP;
			//m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LANDSLIDE));
			return;
		}

		if (EAugustaLandSlideType::LANDSLIDE_SPRINT_START == eLandSlideType) // Loop로 이동.
		{
			m_iCurrentAnimIdx = ENUM_CLASS(EAugustaLandSlideType::LANDSLIDE_SPRINT_LOOP);
			//m_pAugusta->GetStateContextForWrite().m_eLandSlideType = EAugustaLandSlideType::LANDSLIDE_SPRINT_LOOP;
			//m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::LANDSLIDE));
			return;
		}
	}

	
	
 
}


void CAugustaGroundLandSlide::Setup_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaLandSlideType::LANDSLIDE_SPRINT_START), "Landslide_Sprint_Start", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaLandSlideType::LANDSLIDE_SPRINT_LOOP), "Landslide_Sprint_Loop", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaLandSlideType::LANDSLIDE_SPRINT_POSE_F), "Landslide_Sprint_Pose_F", 1.f, 0.f);
}

void CAugustaGroundLandSlide::State_Reset()
{
    for (_uint i = 0; i < LANDSTATE::END; ++i)
        m_States[i] = false;
}



CAugustaGroundLandSlide* CAugustaGroundLandSlide::Create(CCharacter* pOwner)
{
    CAugustaGroundLandSlide* pInstance = new CAugustaGroundLandSlide();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundLandSlide");
        return nullptr;
    }

    return pInstance;
}

void CAugustaGroundLandSlide::Free()
{
    CGroundState::Free();
}
