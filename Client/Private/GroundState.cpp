#include "ClientPch.h"
#include "GroundState.h"
#include "StateMachine.h"


HRESULT CGroundState::Initialize(class CGameObject* pOwner)
{
    if (FAILED(CCharacterState::Initialize(pOwner)))
        return E_FAIL;

    return S_OK;
}

void CGroundState::OnEnter()
{
    CCharacterState::OnEnter();

    // 지상 상태 진입 시 공통 처리
    // ex) 착지 이펙트, 사운드 등
}

void CGroundState::OnUpdate(_float fTimeDelta)
{
    CCharacterState::OnUpdate(fTimeDelta);

    // 지상 상태 공통 로직
    Apply_Gravity(fTimeDelta);
    Check_GroundCollision();

    // 현재 하위 상태 업데이트
   /* if (nullptr != m_pCurrentSubState)
    {
        m_pCurrentSubState->OnUpdate(fTimeDelta);
    }*/
}

void CGroundState::OnExit()
{
    CCharacterState::OnExit();
	m_iNotLandFrames = 0;

    // 지상 상태 탈출 시 공통 처리
}

void CGroundState::Apply_Gravity(_float fTimeDelta)
{
    // TODO: 중력 적용 로직
}

void CGroundState::Check_GroundCollision()
{
    // TODO: 지면 충돌 체크
}

void CGroundState::Free()
{
    CCharacterState::Free();
}
