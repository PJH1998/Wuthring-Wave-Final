#include "ClientPch.h"
#include "AugustaGroundBurst.h"
#include "Augusta.h"
#include "StateMachine.h"
#include "GameSystem.h"

HRESULT CAugustaGroundBurst::Initialize(class CGameObject* pOwner)
{
    if (FAILED(__super::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
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
    m_iPartType = CAugusta::PARTTYPE::PART_SKILLWEAPON;
    m_pAugusta->PartActivate(m_iPartType, true);
    m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
    m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);

	m_pAugusta->Set_Gravity(false);

	// 궁극기 실행 시?
	//m_pAugusta->Play_Action(TEXT("Action_Augusta_Burst01"));
	// 끝나고 유지 시킬것인지, 돌아올것인지

	///ASSERT_CRASH(m_pTransformCom);
	m_pAugusta->Play_Action(TEXT("Action_Augusta_Burst01"));
	//CGameSystem::GetInstance()->Play_Action(TEXT("Action_Augusta_Burst01"), m_pAugusta->Get_WorldMatrix(), false);
	
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
    m_iPartType = CAugusta::PARTTYPE::TYPE_END;

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

    // Target이 존재한다면? => Auto Target
    // m_pAugusta->Rotate_Target();

    if (m_iPartType != CAugusta::PARTTYPE::TYPE_END)
    {
       /* m_pAugusta->Play_PartAnimation(
            m_iPartType,
            m_Animations[m_iCurrentAnimIdx].strAnimName,
            fTimeDelta, nullptr, 1.f // 파츠 애니메이션 적용하는게 아님. => 적용해서 떴던거.
        );*/
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
			m_pAugusta->Clear_PartAnimation(m_iPartType, m_Animations[m_iCurrentAnimIdx].strAnimName);
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
}

void CAugustaGroundBurst::State_Reset()
{
    for (_uint i = 0; i < BURSTSTATE::END; ++i)
        m_States[i] = false;
}


CAugustaGroundBurst* CAugustaGroundBurst::Create(class CGameObject* pOwner)
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
