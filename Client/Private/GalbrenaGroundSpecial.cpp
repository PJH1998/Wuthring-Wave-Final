#include "ClientPch.h"
#include "GalbrenaGroundSpecial.h"
#include "Galbrena.h"
#include "StateMachine.h"

// OMNI 상태에서만 탈출 가능.
HRESULT CGalbrenaGroundSpecial::Initialize(class CGameObject* pOwner)
{
    if (FAILED(__super::Initialize(pOwner)))
        return E_FAIL;

    m_pGalbrena = dynamic_cast<CGalbrena*>(pOwner);
    ASSERT_CRASH(m_pGalbrena);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

	// 미리 사용할 공간 선언.
	m_ActivePartTypes.reserve(CGalbrena::PARTTYPE::TYPE_END);


    return S_OK;
}

void CGalbrenaGroundSpecial::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pGalbrena->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EGalbrenaSpecialType eSpecialType = context.m_eSpecialType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eSpecialType);

    // 4. 상태 초기화
    State_Reset();
	m_pGalbrena->Set_Gravity(true);
	
	m_ActivePartTypes.clear(); // 파츠 목록 초기화
    // 5. 애니메이션 타입에 맞는 파츠 설정. => 0, 1 SWORD / 2, 3 DARKSCYTHE

	switch(eSpecialType)
	{ 
	case EGalbrenaSpecialType::ATTACK05:
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_DARKWING);
		break;
	case EGalbrenaSpecialType::ATTACK06:
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_DARKWING);
		break;
	case EGalbrenaSpecialType::ATTACK07:
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_FIRSTGUN);
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_SECONDGUN);
		m_ActivePartTypes.emplace_back(CGalbrena::PARTTYPE::PART_DARKWING);
		m_pGalbrena->Set_Gravity(false);
		break;
	case EGalbrenaSpecialType::ATTACK08:
		break;
	case EGalbrenaSpecialType::ATTACK08_H:
		break;
	}

	// 사용하는 PartType이 있다면?
	for (auto& PartType : m_ActivePartTypes)
		m_pGalbrena->PartActivate(PartType, true);
		

	m_pGalbrena->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
	m_strSkillName = m_Animations.at(m_iCurrentAnimIdx).strAnimName; // 진입할때 한번 현재 스킬이름 저장.
}

void CGalbrenaGroundSpecial::OnUpdate(_float fTimeDelta)
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

void CGalbrenaGroundSpecial::OnExit()
{
    CGroundState::OnExit();
	m_iComboCount = 0;


	// Part Activate 비활성화
	for (auto& PartType : m_ActivePartTypes)
		m_pGalbrena->PartActivate(PartType, false);

    m_pGalbrena->Set_Gravity(true); 

	// 공격 콜라이더 비활성화
	m_pGalbrena->Collider_Active(TEXT("Main|X|X"), false);

	m_pGalbrena->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
}

void CGalbrenaGroundSpecial::Handle_Input()
{
    m_States[MOVE] = m_pGalbrena->Check_AnyInput(m_iMoveKey);
    m_States[DASH] = m_States[MOVE] && m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
    m_States[ATTACK] = m_pGalbrena->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
}

void CGalbrenaGroundSpecial::Update_SkillAnimations(_float fTimeDelta)
{
	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pGalbrena->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; // 거리 계산에 따른 Animation Scale 조절.
	
	// 1. 애니메이션 실행
    CCharacterState::Play_Animation(m_pGalbrena, fTimeDelta, m_fAnimationScale);

    // 2. 파츠 실행.
	for (auto& iPartType : m_ActivePartTypes)
	{
		if (iPartType == CGalbrena::PART_FIRSTGUN || iPartType == CGalbrena::PART_SECONDGUN)
		{
			m_pGalbrena->Play_PartAnimation(
				iPartType,
				m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName),
				m_Animations.at(m_iCurrentAnimIdx).fSpeed * fTimeDelta, nullptr
			);
		}
		else
		{
			m_pGalbrena->Play_PartAnimation(
				iPartType,
				m_Animations.at(m_iCurrentAnimIdx).strAnimName,
				m_Animations.at(m_iCurrentAnimIdx).fSpeed * fTimeDelta, nullptr
			);
		}
		
	}
	
	
}

void CGalbrenaGroundSpecial::Check_Physcis(_float fTimeDelta)
{
    m_States[LAND] = m_pGalbrena->Is_LandCollider(&m_vLandNormal);
}

void CGalbrenaGroundSpecial::Check_StateTransition(_float fTimeDelta)
{
    EGalbrenaSpecialType eSpType = static_cast<EGalbrenaSpecialType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. 탈출 가능한 시점에서
	if (IsEscapePossible)
	{
		// 가장 우선순위 높은 상황.
		if (m_States[ATTACK])
		{
			switch (eSpType)
			{
				case EGalbrenaSpecialType::ATTACK05:
				{
					m_pGalbrena->GetStateContextForWrite().m_eSpecialType = EGalbrenaSpecialType::ATTACK06;
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::SPECIAL));
					return;
				}
				case EGalbrenaSpecialType::ATTACK06:
				{
					m_pGalbrena->GetStateContextForWrite().m_eSpecialType = EGalbrenaSpecialType::ATTACK07;
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::SPECIAL));
					return;
				}
				case EGalbrenaSpecialType::ATTACK07:
				{
					m_pGalbrena->GetStateContextForWrite().m_eSpecialType = EGalbrenaSpecialType::ATTACK08;
					m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::SPECIAL));
					return;
				}
				
			}
		}
		
		if (m_States[MOVE])
		{
			m_pGalbrena->GetStateContextForWrite().m_eRunType = EGalbrenaRunType::RUN_F;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::RUN)); // 상위, 하위 상태
			return;
		}

		if (m_States[JUMP])
		{
			m_pGalbrena->GetStateContextForWrite().m_eJumpType = EGalbrenaJumpType::JUMP_WALK_LF;
			m_pGalbrena->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EGalbrenaAirState::JUMP)); // 상위, 하위 상태
			return;
		}
	}

	if (m_IsAnimationEnd)
	{
		if (m_States[LAND])
		{
			m_pGalbrena->GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND2;
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
    
}

void CGalbrenaGroundSpecial::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EGalbrenaSpecialType::ATTACK05), "Attack05", 1.f, 12.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaSpecialType::ATTACK06), "Attack06", 1.f, 19.f);
    CState::Add_Animations(ENUM_CLASS(EGalbrenaSpecialType::ATTACK07), "Attack07", 1.f, 50.f); // 25.f ~ 50.f 에 콤보 이펙트.
    CState::Add_Animations(ENUM_CLASS(EGalbrenaSpecialType::ATTACK08), "Attack08", 1.2f, 50.f); // 바로 누를경우 8_H (날개모션)


	m_PartsAnimations.emplace("Attack05", "Gun01");
	m_PartsAnimations.emplace("Attack06", "Gun01");
	m_PartsAnimations.emplace("Attack07", "Gun01");
	m_PartsAnimations.emplace("Attack08", "Gun01");
}

void CGalbrenaGroundSpecial::State_Reset()
{
    for (_uint i = 0; i < SPEICALSTATE::END; ++i)
        m_States[i] = false;
}


CGalbrenaGroundSpecial* CGalbrenaGroundSpecial::Create(class CGameObject* pOwner)
{
    CGalbrenaGroundSpecial* pInstance = new CGalbrenaGroundSpecial();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CGalbrenaGroundSpecial");
    }

    return pInstance;
}

void CGalbrenaGroundSpecial::Free()
{
    CGroundState::Free();
}
