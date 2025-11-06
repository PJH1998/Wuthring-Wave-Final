#pragma once
#include "CharacterState.h"

NS_BEGIN(Client)

// 모든 캐릭터가 공유하는 등반 상태
class CClimbState abstract : public CCharacterState
{
protected:
    explicit CClimbState() = default;
    virtual ~CClimbState() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter(void* pArg = nullptr) override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;

protected:
    // 등반 상태 공통 로직
    void Check_ClimbSurface();
    void Check_ClimbExit();

protected:
    _float3 m_vHeadWallNormal = {};

public:
    void Change_SubState(const _string& strSubStateName, class CStateMachine* pStateMachine);
    virtual void Free() override;
};

NS_END
