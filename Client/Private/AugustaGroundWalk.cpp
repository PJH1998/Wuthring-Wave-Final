#include "ClientPch.h"
#include "AugustaGroundWalk.h"
#include "Augusta.h"
#include "StateMachine.h"


HRESULT CAugustaGroundWalk::Initialize(class CGameObject* pOwner)
{
    if (FAILED(__super::Initialize(pOwner)))
        return E_FAIL;

    m_pAugusta = dynamic_cast<CAugusta*>(pOwner);
    ASSERT_CRASH(m_pAugusta);
        

    // Walk 애니메이션 리스트 셋업

    return S_OK;
}

void CAugustaGroundWalk::OnEnter()
{
    CGroundState::OnEnter();
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

CAugustaGroundWalk* CAugustaGroundWalk::Create(class CGameObject* pOwner)
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
