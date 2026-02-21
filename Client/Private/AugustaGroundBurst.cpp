#include "ClientPch.h"
#include "AugustaGroundBurst.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "GameSystem.h"

HRESULT CAugustaGroundBurst::Initialize(CCharacter* pCharacter)
{
    if (FAILED(__super::Initialize(pCharacter)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pCharacter);
    ASSERT_CRASH(m_pAugusta);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();
    

    return S_OK;
}

void CAugustaGroundBurst::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);

    // 1. 복사본 Context 받아오기
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    EAugustaBurstType eBurstType = context.m_eBurstType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eBurstType);

    // 4. 상태 초기화
    State_Reset();

    _string strBoneName = "WeaponProp02";
    //m_iPartType = CAugusta::PARTTYPE::PART_SKILLWEAPON;
    m_iPartType = CAugusta::PARTTYPE::PART_BURSTWEAPON;
	m_iSubPartType = CAugusta::PARTTYPE::PART_HEADPROP;
    m_pAugusta->PartActivate(m_iPartType, true);
    m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations.at(m_iCurrentAnimIdx).strAnimName);
    m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
	
	m_pAugusta->PartActivate(m_iSubPartType, true);
	m_pAugusta->Clear_PartAnimation(CAugusta::PARTTYPE::PART_HEADPROP, m_Animations.at(m_iCurrentAnimIdx).strAnimName);
	m_pAugusta->Set_SocketMatrixToParts(m_iSubPartType, "Bone_Hair001_M");
	m_pAugusta->Part_ShaderPathChange(m_iSubPartType, ENUM_CLASS(SHADER_PROPANIMMESH::AUGUSTA_HEADPROP));
	
	m_pAugusta->Change_TimeRatio_ToLayer(COLLISIONLAYER::ENEMY, 0.0f);

	m_pAugusta->Play_Action(TEXT("Action_Augusta_Burst01"), false, true);
	
	m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
	m_pAugusta->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::CUTSCENE));
	
	m_pAugusta->Set_Gravity(false);
	m_pAugusta->Set_OutLineVisible(false); // 궁극기 도중에는 입 모양이 보이게 하기 위함.
}

void CAugustaGroundBurst::OnUpdate(_float fTimeDelta)
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

void CAugustaGroundBurst::OnExit()
{
    CGroundState::OnExit();
    m_pAugusta->PartActivate(m_iPartType, false);
	m_pAugusta->PartActivate(m_iSubPartType, false);

    m_iPartType = CAugusta::PARTTYPE::TYPE_END;
	m_pAugusta->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE));
	m_pAugusta->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::CUTSCENE));
	m_pAugusta->Collider_Active(TEXT("Main|X|X"), false);
	m_pAugusta->Set_OutLineVisible(true);

	m_pAugusta->Change_TimeRatio_ToLayer(COLLISIONLAYER::ENEMY, 1.0f);

}

void CAugustaGroundBurst::Handle_Input()
{
    m_States[SP_MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[SP_DASH] = m_States[SP_MOVE] && m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
    m_States[SP_ATTACK] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LB));
}

void CAugustaGroundBurst::Update_SkillAnimations(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);
    
	EAugustaBurstType eBurstType = static_cast<EAugustaBurstType>(m_iCurrentAnimIdx);

	// Burst
	m_pAugusta->Play_PartAnimation(
		m_iPartType,
		m_PartsAnimations.at(m_Animations.at(m_iCurrentAnimIdx).strAnimName),
		m_Animations.at(m_iCurrentAnimIdx).fSpeed * fTimeDelta, nullptr
	);

	if (eBurstType == EAugustaBurstType::BURST01)
	{
		m_pAugusta->Play_PartAnimation(
			CAugusta::PARTTYPE::PART_HEADPROP,
			"Burst01",
			m_Animations.at(m_iCurrentAnimIdx).fSpeed * fTimeDelta, nullptr
		);
	}
	else
	{
		m_pAugusta->Play_PartAnimation(
			CAugusta::PARTTYPE::PART_HEADPROP,
			"Stand1_idle",
			m_Animations.at(m_iCurrentAnimIdx).fSpeed * fTimeDelta, nullptr
		);
	}
}

void CAugustaGroundBurst::Check_Physcis(_float fTimeDelta)
{
}

void CAugustaGroundBurst::Check_StateTransition(_float fTimeDelta)
{
    EAugustaBurstType eBurstType = static_cast<EAugustaBurstType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

    // Burst01 => Stand 고정
    if (IsEscapePossible && eBurstType == EAugustaBurstType::BURST_STAND)
    {
        if (m_States[SP_ATTACK])
        {
            m_pAugusta->GetStateContextForWrite().m_eSpecialType = EAugustaSpecialType::SPATTACK01;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SPECIAL));
            return;
        }

        if (m_States[SP_DASH])
        {
            m_pAugusta->GetStateContextForWrite().m_eSpecialType = EAugustaSpecialType::SPWALK_DASH_ROOT;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SPECIAL));
            return;
        }

        if (m_States[SP_MOVE])
        {
            m_pAugusta->GetStateContextForWrite().m_eSpecialType = EAugustaSpecialType::SPWALK_F;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SPECIAL));
            return;
        }

        return;
    }
    
    if (m_IsAnimationEnd)
    {
        if (eBurstType == EAugustaBurstType::BURST01)
        {
            m_iCurrentAnimIdx = ENUM_CLASS(EAugustaBurstType::BURST_STAND);
			m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations.at(m_iCurrentAnimIdx).strAnimName);
			m_pAugusta->Set_AnimationToParts(CAugusta::PARTTYPE::PART_HEADPROP, "Stand1_idle");
            //m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaBurstType::BURST_STAND));
            return;
        }

        if (eBurstType == EAugustaBurstType::BURST_STAND)
        {
            m_pAugusta->GetStateContextForWrite().m_eSpecialType = EAugustaSpecialType::SPWALK_STAND;
            m_pAugusta->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::SPECIAL));
            return;
        }
    }
    
}

void CAugustaGroundBurst::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(EAugustaBurstType::BURST01), "Burst01", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(EAugustaBurstType::BURST_STAND), "Burst_Stand", 1.f, 0.f);

	m_PartsAnimations.emplace("Burst01", "Sword_Open_Loop");
	m_PartsAnimations.emplace("Burst_Stand", "Sword_Open_Loop_2");
}

void CAugustaGroundBurst::State_Reset()
{
    for (_uint i = 0; i < BURSTSTATE::END; ++i)
        m_States[i] = false;
}


CAugustaGroundBurst* CAugustaGroundBurst::Create(CCharacter* pOwner)
{
    CAugustaGroundBurst* pInstance = new CAugustaGroundBurst();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundBurst");
    }

    return pInstance;
}

void CAugustaGroundBurst::Free()
{
    CGroundState::Free();
}
