#include "ClientPch.h"
#include "AugustaGroundSpecial.h"
#include "Augusta.h"
#include "StateMachine.h"

// OMNI 상태에서만 탈출 가능.
HRESULT CAugustaGroundSpecial::Initialize(CCharacter* pCharacter)
{
    if (FAILED(__super::Initialize(pCharacter)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pCharacter);
    ASSERT_CRASH(m_pAugusta);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    return S_OK;
}

void CAugustaGroundSpecial::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaSpecialType eSpecialType = context.m_eSpecialType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eSpecialType);

    // 4. 상태 초기화
    State_Reset();

    // 5. 애니메이션 타입에 맞는 파츠 설정.
    //m_iPartType = CAugusta::PARTTYPE::PART_SKILLWEAPON;
    m_iPartType = CAugusta::PARTTYPE::PART_BURSTWEAPON;
	m_iSubPartType = CAugusta::PARTTYPE::PART_HEADPROP;

    m_pAugusta->PartActivate(m_iPartType, true);
    m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations.at(m_iCurrentAnimIdx).strAnimName);
    m_pAugusta->Set_Gravity(true);

	m_pAugusta->PartActivate(m_iSubPartType, true);
	m_pAugusta->Clear_PartAnimation(m_iSubPartType, m_Animations.at(m_iCurrentAnimIdx).strAnimName);
	m_pAugusta->Set_SocketMatrixToParts(m_iSubPartType, "Bone_Hair001_M");

	if (eSpecialType == EAugustaSpecialType::SPATTACKOMNI)
	{
		m_pAugusta->Set_Gravity(true);
		
	}
		
	// 6. 컨디션 추가.
	CGameInstance::GetInstance()->Change_TimeRate(TEXT("Timer_60"), 1.f, 0.1f);
	m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));

	m_strSkillName = m_Animations.at(m_iCurrentAnimIdx).strAnimName; // 진입할때 한번 현재 스킬이름 저장.

	m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::CUTSCENE));


	m_pAugusta->Set_OutLineVisible(false);
}

void CAugustaGroundSpecial::OnUpdate(_float fTimeDelta)
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

void CAugustaGroundSpecial::OnExit()
{
    CGroundState::OnExit();
    m_pAugusta->PartActivate(m_iPartType, false);
    m_pAugusta->PartActivate(m_iSubPartType, false);
    m_iComboCount = 0;
    m_pAugusta->Set_Gravity(true); 

	m_pAugusta->Remove_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
	m_pAugusta->Remove_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::R_SP_ATTACKOMNI));

	// 무적 제거.
	m_pAugusta->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
	m_pAugusta->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::CUTSCENE));

	// 공격 콜라이더 비활성화
	m_pAugusta->Collider_Active(TEXT("Main|X|X"), false);

	m_pAugusta->Set_OutLineVisible(true);

}

void CAugustaGroundSpecial::Handle_Input()
{
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[DASH] = m_States[MOVE] && m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
    m_States[ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
}

void CAugustaGroundSpecial::Update_SkillAnimations(_float fTimeDelta)
{

	// 0. 몬스터와의 거리 계산 (최우선)
	m_fRootMotionScale = m_pAugusta->Calculate_RootMotionScale();
	m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate * m_fRootMotionScale; // 거리 계산에 따른 Animation Scale 조절.

	// 1. 막타는 이동거리 온전하게 다받기.
	if (m_iComboCount == COMBO::COMBO_ATTACKOMNI)
		m_fAnimationScale = m_Animations.at(m_iCurrentAnimIdx).fRootMotionRate;

    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta, m_fAnimationScale);

    // Target이 존재한다면? => Auto Target
    EAugustaSpecialType eSpType = static_cast<EAugustaSpecialType>(m_iCurrentAnimIdx);
    m_eDir = m_pAugusta->Calculate_Direction();
   
    if (eSpType == EAugustaSpecialType::SPWALK_F)
    {
        m_pAugusta->Move_By_Camera_Direction_8Way(m_eDir, fTimeDelta, 0.1f);
    }

    
	m_pAugusta->Play_PartAnimation(
		m_iPartType,
		m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName),
		m_Animations.at(m_iCurrentAnimIdx).fSpeed * fTimeDelta, nullptr
	);

	// HeadProp 먼저 업데이트.
	m_pAugusta->Play_PartAnimation(
		m_iSubPartType,
		"Stand1_idle",
		m_Animations.at(m_iCurrentAnimIdx).fSpeed * fTimeDelta, nullptr
	);



}

void CAugustaGroundSpecial::Check_Physcis(_float fTimeDelta)
{
    m_States[LAND] = m_pAugusta->Is_LandCollider(&m_vLandNormal);
}

void CAugustaGroundSpecial::Check_StateTransition(_float fTimeDelta)
{
    EAugustaSpecialType eSpType = static_cast<EAugustaSpecialType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

	// 1. 탈출 가능 시점에서
    if (IsEscapePossible)
    {
		// 2. 어택키를 눌렀을때 => Ability에 사용 가능한 스킬인지를 묻습니다.
		if (m_States[ATTACK])
		{
			m_States[ATTACK01] = (SKILL_STATE::READY == m_pAugusta->Check_Skill("SpAttack01")) && (m_iComboCount == 0);
			m_States[ATTACK02] = (SKILL_STATE::READY == m_pAugusta->Check_Skill("SpAttack02")) && (m_iComboCount == 1);
			m_States[ATTACK03] = (SKILL_STATE::READY == m_pAugusta->Check_Skill("SpAttack03")) && (m_iComboCount == 2);
			m_States[ATTACK04] = (SKILL_STATE::READY == m_pAugusta->Check_Skill("SpAttack01")) && (m_iComboCount == 3);
			m_States[ATTACK05] = (SKILL_STATE::READY == m_pAugusta->Check_Skill("SpAttack02")) && (m_iComboCount == 4);
			m_States[ATTACK06] = (SKILL_STATE::READY == m_pAugusta->Check_Skill("SpAttack03")) && (m_iComboCount == 5);
			m_States[ATTACKOMNI] = (SKILL_STATE::READY == m_pAugusta->Check_Skill("SpAttackOmni")) && (m_iComboCount == 6);

			if (m_States[ATTACK01])
			{
				if (SKILL_STATE::READY != m_pAugusta->Use_Skill("SpAttack01"))
					return;

				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaSpecialType::SPATTACK01);
				m_iComboCount = COMBO::COMBO_ATTACK01;
				m_pAugusta->Rotate_Target();
				return;
			}

			if (m_States[ATTACK02])
			{
				if (SKILL_STATE::READY != m_pAugusta->Use_Skill("SpAttack02"))
					return;

				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaSpecialType::SPATTACK02);
				m_iComboCount = COMBO::COMBO_ATTACK02;
				m_pAugusta->Rotate_Target();
				return;
			}

			if (m_States[ATTACK03])
			{
				if (SKILL_STATE::READY != m_pAugusta->Use_Skill("SpAttack03"))
					return;

				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaSpecialType::SPATTACK03);
				m_iComboCount = COMBO::COMBO_ATTACK03;
				m_pAugusta->Rotate_Target();
				return;
			}

			if (m_States[ATTACK04])
			{
				if (SKILL_STATE::READY != m_pAugusta->Use_Skill("SpAttack01"))
					return;

				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaSpecialType::SPATTACK01);
				m_iComboCount = COMBO::COMBO_ATTACK04;
				m_pAugusta->Rotate_Target();
				return;
			}

			if (m_States[ATTACK05])
			{
				if (SKILL_STATE::READY != m_pAugusta->Use_Skill("SpAttack02"))
					return;

				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaSpecialType::SPATTACK02);
				m_iComboCount = COMBO::COMBO_ATTACK05;
				m_pAugusta->Rotate_Target();
				return;
			}

			if (m_States[ATTACK06])
			{
				if (SKILL_STATE::READY != m_pAugusta->Use_Skill("SpAttack03"))
					return;

				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaSpecialType::SPATTACK03);
				m_iComboCount = COMBO::COMBO_ATTACK06;
				m_pAugusta->Rotate_Target();

				// Bind Condition Burst 궁
				m_pAugusta->Remove_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::LB_SP_ATTACK));
				m_pAugusta->Bind_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::R_SP_ATTACKOMNI));

				return;
			}

			if (m_States[ATTACKOMNI])
			{
				if (SKILL_STATE::READY != m_pAugusta->Use_Skill("SpAttackOmni"))
					return;

				

				// Remove Condition 마지막 Burst 궁
				m_pAugusta->Remove_Condition_ToAbillity(ENUM_CLASS(UI_AUGUSTA_CONDITION::R_SP_ATTACKOMNI));
				
				m_iCurrentAnimIdx = ENUM_CLASS(EAugustaSpecialType::SPATTACKOMNI);
				m_iComboCount = COMBO::COMBO_ATTACKOMNI;
				m_pAugusta->Play_Action(TEXT("Action_Augusta_SpAttackOmni"));
				//m_pAugusta->Rotate_Target();
				return;
			}


		}
    
        if (eSpType != EAugustaSpecialType::SPATTACKOMNI)
        {
            if (m_States[DASH])
            {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaSpecialType::SPWALK_DASH_ROOT);
                return;
            }

            if (m_States[MOVE])
            {
                m_iCurrentAnimIdx = ENUM_CLASS(EAugustaSpecialType::SPWALK_F);
                return;
            }
        }


		// 끝났을떄 랜드라면?
		if (eSpType == EAugustaSpecialType::SPATTACKOMNI)
		{
			if (m_States[MOVE])
			{
				if (m_States[LAND])
				{
					m_pAugusta->GetStateContextForWrite().m_eRunType = EAugustaRunType::RUN_F;
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::RUN));
					return;
				}
				else if (!m_States[LAND])
				{
					m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
					m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL));
					return;
				}
			}
		}
    }


    if (m_IsAnimationEnd)
    {
        if (eSpType == EAugustaSpecialType::SPWALK_F)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(EAugustaSpecialType::SPWALK_STOP_L);
            return;
        }

        // 상태 탈출
        if (eSpType == EAugustaSpecialType::SPATTACKOMNI)
        {
            if (m_States[LAND])
            {
                m_pAugusta->GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STANDCHANGE;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
                return;
             
            }
            else if (!m_States[LAND])
            {
                m_pAugusta->GetStateContextForWrite().m_eFallType = EAugustaFallType::FALL_LOOP;
                m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(EAugustaAirState::FALL));
                return;
            }
        }
    }
    
}

void CAugustaGroundSpecial::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaSpecialType::SPATTACK01), "SpAttack01", 1.f, 10.f, 0.5f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSpecialType::SPATTACK02), "SpAttack02", 1.f, 10.f, 0.5f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSpecialType::SPATTACK03), "SpAttack03", 1.f, 10.f, 0.5f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSpecialType::SPATTACKOMNI), "SpAttackOmni", 1.f, 80.f, 0.5f, true, false );
    CState::Add_Animations(ENUM_CLASS(EAugustaSpecialType::SPWALK_DASH), "SpWalk_Dash", 1.f, 12.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSpecialType::SPWALK_DASH_ROOT), "SpWalk_Dash_Root", 0.5f, 30.f, 1.f); // 너무 빠름.
    CState::Add_Animations(ENUM_CLASS(EAugustaSpecialType::SPWALK_F), "SpWalk_F", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSpecialType::SPWALK_STAND), "SpWalk_Stand", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSpecialType::SPWALK_STOP_L), "SpWalk_Stop_L", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaSpecialType::SPWALK_STOP_R), "SpWalk_Stop_R", 1.f, 0.f);

	m_PartsAnimations.emplace("SpAttack01", "Sword_Open_Loop");
	m_PartsAnimations.emplace("SpAttack02", "Sword_Open_Loop");
	m_PartsAnimations.emplace("SpAttack03", "Sword_Open_Loop");
	m_PartsAnimations.emplace("SpAttackOmni", "Sword_Open_Loop");
	m_PartsAnimations.emplace("SpWalk_Dash", "Sword_Open_Loop");
	m_PartsAnimations.emplace("SpWalk_Dash_Root", "Sword_Open_Loop");
	m_PartsAnimations.emplace("SpWalk_F", "Sword_Open_Loop");
	m_PartsAnimations.emplace("SpWalk_Stand", "Sword_Open_Loop");
	m_PartsAnimations.emplace("SpWalk_Stop_L", "Sword_Open_Loop");
	m_PartsAnimations.emplace("SpWalk_Stop_R", "Sword_Open_Loop");
}

void CAugustaGroundSpecial::State_Reset()
{
    for (_uint i = 0; i < SPEICALSTATE::END; ++i)
        m_States[i] = false;
}


CAugustaGroundSpecial* CAugustaGroundSpecial::Create(CCharacter* pOwner)
{
    CAugustaGroundSpecial* pInstance = new CAugustaGroundSpecial();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundSpecial");
    }

    return pInstance;
}

void CAugustaGroundSpecial::Free()
{
    CGroundState::Free();
}
