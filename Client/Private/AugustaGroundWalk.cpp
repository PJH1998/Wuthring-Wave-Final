#include "ClientPch.h"
#include "AugustaGroundWalk.h"
#include "Augusta.h"
#include "StateMachine.h"


HRESULT CAugustaGroundWalk::Initialize(CCharacter* pCharacter)
{
    if (FAILED(__super::Initialize(pCharacter)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pCharacter);
    ASSERT_CRASH(m_pAugusta);
        

    // Walk 애니메이션 리스트 셋업

    return S_OK;
}

void CAugustaGroundWalk::OnEnter(void* pArg)
{
    CGroundState::OnEnter(pArg);
}

void CAugustaGroundWalk::OnUpdate(_float fTimeDelta)
{
    CGroundState::OnUpdate(fTimeDelta);

    Update_WalkDirection(fTimeDelta);
}

void CAugustaGroundWalk::OnExit()
{
    CGroundState::OnExit();
}


void CAugustaGroundWalk::Update_WalkDirection(_float fTimeDelta)
{

}

void CAugustaGroundWalk::Check_StateTransition()
{

}

CAugustaGroundWalk* CAugustaGroundWalk::Create(CCharacter* pOwner)
{
    CAugustaGroundWalk* pInstance = new CAugustaGroundWalk();

    if (FAILED(pInstance->Initialize(pOwner)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Failed to Create : CAugustaGroundWalk");
        return nullptr;
    }

    return pInstance;
}

void CAugustaGroundWalk::Free()
{
    CGroundState::Free();
}
