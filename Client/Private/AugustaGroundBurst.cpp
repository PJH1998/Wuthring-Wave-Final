#include "ClientPch.h"
#include "AugustaGroundBurst.h"
#include "Augusta.h"
#include "StateMachine.h"

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

void CAugustaGroundBurst::OnEnter()
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

    m_iPartType = CAugusta::PARTTYPE::PART_BAYONET;
    m_pAugusta->PartAcitvate(m_iPartType, true);
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
    m_pAugusta->PartAcitvate(m_iPartType, false);
}

void CAugustaGroundBurst::Handle_Input()
{
    m_States[JUMP] = m_pAugusta->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));

}

void CAugustaGroundBurst::Update_SkillAnimations(_float fTimeDelta)
{
    CCharacterState::Play_Animation(m_pAugusta, fTimeDelta);

    // Target이 존재한다면? => Auto Target
    m_pAugusta->Rotate_Target();

    m_pAugusta->Play_PartAnimation(
        m_iPartType,
        m_Animations[m_iCurrentAnimIdx].strAnimName,
        fTimeDelta, nullptr
    );
}

void CAugustaGroundBurst::Check_Physcis(_float fTimeDelta)
{
}

void CAugustaGroundBurst::Check_StateTransition(_float fTimeDelta)
{
    EBurstType eBurstType = static_cast<EBurstType>(m_iCurrentAnimIdx);

    _bool IsEscapePossible = CState::Is_EscapePossible();
    
}

void CAugustaGroundBurst::SetUp_Animations()
{

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
