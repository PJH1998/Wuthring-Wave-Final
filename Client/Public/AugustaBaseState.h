#pragma once
#include "State.h"
#include "PlayerAugusta.h"


NS_BEGIN(Client)
class CAugustaBaseState abstract : public CState
{
public:
    virtual HRESULT Initialize(const STATE_DATA& StateData) override;
    virtual void OnEnter() override; // 들어갔을 시.
    virtual void OnUpdate(_float fTimeDelta) override; // Update
    virtual void OnExit() override; // Exit 시
    virtual void Change_State(class CStateMachine* pStateMachine, const _string& strStateName) override;
    

protected:
    CPlayerAugusta* m_pPlayer = { nullptr };

    _uint m_iMoveKey = {};
    _uint m_iKeyInput = {};
    

private:
    void Ready_KeyBind();

public:
    virtual void Free() override;

};
NS_END

