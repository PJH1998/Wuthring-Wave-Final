#pragma once
#include "CharacterState.h"
#include "StateCategory_Enum.h"

NS_BEGIN(Client)

// 모든 캐릭터가 공유하는 지상 상태 (카테고리 State)
class CGroundState abstract : public CCharacterState
{
protected:
    explicit CGroundState() = default;
    virtual ~CGroundState() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner) override;
    virtual void OnEnter() override;
    virtual void OnUpdate(_float fTimeDelta) override;
    virtual void OnExit() override;



protected:
    void Apply_Gravity(_float fTimeDelta);
    void Check_GroundCollision();

    

public:
    //void Change_SubState(const _string& strSubStateName, class CStateMachine* pStateMachine);

    virtual void Free() override;
};

NS_END
