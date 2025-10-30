#include "ClientPch.h"
#include "AugustaGroundSpecial.h"
#include "Augusta.h"
#include "StateMachine.h"

HRESULT CAugustaGroundSpecial::Initialize(class CGameObject* pOwner)
{
    if (FAILED(__super::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    // 애니메이션 리스트 셋업.
    SetUp_Animations();

    

    return S_OK;
}

void CAugustaGroundSpecial::OnEnter()
{
    CGroundState::OnEnter();

    // 1. 복사본 Context 받아오기
    const auto context = m_pAugusta->TakeStateContext();

    // 2. 복사본에서 필요한 값 읽기
    ESkillType eSkillType = context.m_eSkillType;

    // 3. 애니메이션 세팅.
    m_iCurrentAnimIdx = ENUM_CLASS(eSkillType);

    // 4. 상태 초기화
    State_Reset();

    // 5. 애니메이션 타입에 맞는 파츠 설정.

     
    switch(eSkillType)
    {
    case ESkillType::SKILL_STRIKE:
    {
        //_string strBoneName = "WeaponProp06";
        _string strBoneName = "Root";
        m_iPartType = CAugusta::PARTTYPE::PART_GRIFFON;
        m_pAugusta->PartAcitvate(m_iPartType, true);
        m_pAugusta->Set_SocketMatrixToParts(m_iPartType, strBoneName);
        break;
    }
    case ESkillType::SKILL_RISE:
    {

        break;
    }
        
    }
    
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
    m_pAugusta->PartAcitvate(m_iPartType, false);
}

void CAugustaGroundSpecial::Handle_Input()
{
    m_States[MOVE] = m_pAugusta->Check_AnyInput(m_iMoveKey);
    m_States[DASH] = m_States[MOVE] && m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));

}

void CAugustaGroundSpecial::Update_SkillAnimations(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // Target이 존재한다면? => Auto Target
    m_pAugusta->Rotate_Target();

    //m_pAugusta->Move_Direction(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta, 0.1f);

    m_pAugusta->Play_PartAnimation(
        m_iPartType,
        m_PartsAnimations[m_Animations[m_iCurrentAnimIdx].strAnimName],
        fTimeDelta * m_Animations[m_iCurrentAnimIdx].fSpeed, nullptr
    );
}

void CAugustaGroundSpecial::Check_Physcis(_float fTimeDelta)
{
    
}

void CAugustaGroundSpecial::Check_StateTransition(_float fTimeDelta)
{
    ESkillType eSkillType = static_cast<ESkillType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();

    
}

void CAugustaGroundSpecial::SetUp_Animations()
{
    CState::Add_Animations(ENUM_CLASS(ESpecialType::SPATTACK01), "SpAttack01", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESpecialType::SPATTACK02), "SpAttack02", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESpecialType::SPATTACK03), "SpAttack03", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESpecialType::SPATTACKOMNI), "SpAttackOmni", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESpecialType::SPWALK_DASH), "SpWalk_Dash", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESpecialType::SPWALK_DASH_ROOT), "SpWalk_Dash_Root", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESpecialType::SPWALK_F), "SpWalk_F", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESpecialType::SPWALK_STAND), "SpWalk_Stand", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESpecialType::SPWALK_STOP_L), "SpWalk_Stop_L", 1.f, 0.f);
    CState::Add_Animations(ENUM_CLASS(ESpecialType::SPWALK_STOP_R), "SpWalk_Stop_R", 1.f, 0.f);

}

void CAugustaGroundSpecial::State_Reset()
{
    for (_uint i = 0; i < SPEICALSTATE::END; ++i)
        m_States[i] = false;
}


CAugustaGroundSpecial* CAugustaGroundSpecial::Create(class CGameObject* pOwner)
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
