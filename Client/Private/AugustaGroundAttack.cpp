#include "ClientPch.h"
#include "AugustaGroundAttack.h"
#include "Augusta.h"
#include "StateMachine.h"

HRESULT CAugustaGroundAttack::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CGroundState::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);

    

    return S_OK;
}


void CAugustaGroundAttack::OnEnter()
{
    CGroundState::OnEnter();

    // 첫 공격 시작 (Attack01)
    if (m_iComboCount == 0)
    {
        // TODO: Play Attack01 animation
    }
}

void CAugustaGroundAttack::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    Update_ComboChain();
}

void CAugustaGroundAttack::OnExit()
{
    CGroundState::OnExit();

    // 콤보 카운트 초기화
    m_iComboCount = 0;
    m_bCanCombo = false;
}

void CAugustaGroundAttack::Update_ComboChain()
{
    // TODO: 콤보 시스템
    // Attack01 → Attack02 → Attack03 → Attack04
    // 애니메이션 특정 구간에서 m_bCanCombo = true
    // 마우스 입력 들어오면 다음 콤보로 전환

    // if (m_bCanCombo && 마우스입력)
    // {
    //     m_iComboCount++;
    //     if (m_iComboCount == 1) Play("Attack02");
    //     else if (m_iComboCount == 2) Play("Attack03");
    //     else if (m_iComboCount == 3) Play("Attack04");
    // }
}

void CAugustaGroundAttack::Check_StateTransition()
{
    // TODO: 공격 애니메이션 끝나고 추가 입력 없으면 Idle로
    // TODO: 스킬 입력 (E, R 등) 들어오면 Skill로
}

CAugustaGroundAttack* CAugustaGroundAttack::Create(class CGameObject* pOwner)
{
    CAugustaGroundAttack* pInstance = new CAugustaGroundAttack();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundAttack");
    }

    return pInstance;
}

void CAugustaGroundAttack::Free()
{
    CGroundState::Free();
}
