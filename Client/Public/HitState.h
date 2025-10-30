#pragma once
#include "CharacterState.h"

NS_BEGIN(Client)

// 모든 캐릭터가 공유하는 피격 상태
class CHitState abstract : public CCharacterState
{
protected:
    explicit CHitState() = default;
    virtual ~CHitState() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

protected:
    // 피격 상태 공통 로직
    void Apply_KnockbackForce(_float fTimeDelta);
    void Check_HitRecovery();

    class CState* m_pCurrentSubState = { nullptr };

public:
    void Change_SubState(const _string& strSubStateName, class CStateMachine* pStateMachine);
    virtual void Free() override;
};

NS_END
